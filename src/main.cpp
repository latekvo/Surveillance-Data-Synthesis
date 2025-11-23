#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>
#include <print>
#include <vector>

#include "coco_labels.h"
#include "consts.h"
#include "csv.h"
#include "detection.h"
#include "postprocess.h"
#include "preprocess.h"
#include "remapper.h"
#include "utils.h"

int main() {
  // TODO: Make this scalable, variable. For now using video stream dimensions
  int screenWidthTarget = 1280;
  int screenHeight = 720;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidthTarget, screenHeight, "TTGL");
  SetTargetFPS(TARGET_FPS);

  std::vector coordMaps = loadCoordMaps();
  std::vector streams = loadCsv(STREAMS_FILE);

  // TODO: Isolate stream loading to separate file
  if (streams.size() == 0) {
    std::println(
        "Error - No streams provided! Add at least one entry to "
        "'streams.csv'.");
    return -1;
  }

  if (streams[0].size() != 2) {
    std::println("Error - The streams CSV file is malformed!");
    return -1;
  }

  cv::VideoCapture video(streams[0][1], cv::CAP_FFMPEG);

  if (!video.isOpened()) {
    std::println("Error - Could not open video!");
    return -1;
  }

  cv::Mat videoFrame;
  cv::Mat detectionFrame;
  Image rayImage;

  rayImage.mipmaps = 1;
  rayImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

  const Ort::Env oxxnEnv(ORT_LOGGING_LEVEL_WARNING, "TTGL");
  const Ort::SessionOptions oxxnOptions;
  Ort::Session onnxSession(oxxnEnv, DNN_NET_FILE, oxxnOptions);

  std::vector<std::string> classes = getCocoLabels();
  std::vector<uint> allowedClassIDs = {PERSON,     BICYCLE, CAR,
                                       MOTORCYCLE, BUS,     TRUCK};

  while (!WindowShouldClose()) {
    screenWidthTarget = GetScreenWidth();

    // Skip buffered frames, this is a bit hacky, but found no better way
    // TODO: Calculate ideal skip-rate on the go
    uint skippedFrames = 0;
    while (video.grab() && skippedFrames < 5) {
      skippedFrames++;
    }

    video.read(videoFrame);

    // --- DETECTION LOGIC ---

    constexpr float areaSize = DETECTION_SIZE;
    std::vector<DetectionArea> areas = toDetectionAreas(videoFrame, areaSize);

    for (DetectionArea& area : areas) {
      std::vector<Detection> results =
          runDetection(onnxSession, area.frame, classes);
      area.detections.insert(area.detections.end(), results.begin(),
                             results.end());
    }

    std::vector<Detection> unfilteredDetections = mergeDetectionAreas(areas);
    std::vector<Detection> detections =
        toFilteredDetections(unfilteredDetections, allowedClassIDs);

    // --- RENDERING LOGIC ---

    const float scale = screenWidthTarget / (float)videoFrame.cols;
    screenHeight = (float)videoFrame.rows * scale;

    if (IsWindowResized()) {
      SetWindowSize(screenWidthTarget, screenHeight);
    }

    cv::resize(videoFrame, videoFrame,
               cv::Size(screenWidthTarget, screenHeight));

    // TODO: Apply this to YOLO input too?
    // OpenCV and raylib use different pixel formats
    cv::cvtColor(videoFrame, videoFrame, cv::COLOR_BGR2RGB);

    // remap OpenCV to raylib image, no copy, using shared memory
    rayImage.data = videoFrame.data;
    rayImage.width = videoFrame.cols;
    rayImage.height = videoFrame.rows;

    Texture2D texture = LoadTextureFromImage(rayImage);

    // --- DRAWING CALLS ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawTexture(texture, 0, 0, WHITE);

    for (const Detection& detection : detections) {
      Rectangle rect = scaleRect(detection.rect, scale);
      const auto classname = classes[detection.classIdx].c_str();
      DrawText(classname, rect.x, rect.y - 10, 6, WHITE);
      DrawRectangleLinesEx(rect, 2.f, WHITE);
    }

    for (const DetectionArea& area : areas) {
      Rectangle rect =
          scaleRect(Rectangle{(float)area.offset.x, (float)area.offset.y,
                              areaSize, areaSize},
                    scale);
      DrawRectangleLinesEx(rect, 1, GREEN);
    }

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

