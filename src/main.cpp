#include <onnxruntime_cxx_api.h>
#include <raylib.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>
#include <print>
#include <vector>

#include "coco_labels.h"
#include "components/DetectionOverlay.hpp"
#include "components/MinimapOverlay.hpp"
#include "consts.h"
#include "detection.h"
#include "postprocess.h"
#include "preprocess.h"
#include "remapper.h"
#include "streams.h"
#include "utils.h"

int main() {
  int screenWidthTarget = 1280;
  int screenHeight = 720;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidthTarget, screenHeight, "TTGL");
  SetTargetFPS(TARGET_FPS);

  std::vector coordMaps = loadCoordMaps();
  std::vector streams = loadStreams();

  cv::VideoCapture video(streams[0].url, cv::CAP_FFMPEG);

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

  CoordMap coordMap;
  for (const CoordMap& map : coordMaps) {
    if (map.cameraRef == streams[0].name) {
      coordMap = map;
    }
  }

  while (!WindowShouldClose()) {
    screenWidthTarget = GetScreenWidth();

    // Skip buffered frames, this is a bit hacky, but found no better way
    // TODO: Calculate ideal skip-rate on the go
    uint skippedFrames = 0;
    while (video.grab() && skippedFrames < 5) {
      skippedFrames++;
    }

    video.read(videoFrame);

    float scale = screenWidthTarget / (float)videoFrame.cols;
    screenHeight = (float)videoFrame.rows * scale;

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

    static auto detectionOverlay =
        AS::DetectionOverlay(&detections, &classes, &scale);

    static auto minimapOverlay =
        AS::MinimapOverlay(&detections, &coordMap, &scale);

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

    for (const DetectionArea& area : areas) {
      Rectangle rect =
          scaleRect(Rectangle{(float)area.offset.x, (float)area.offset.y,
                              areaSize, areaSize},
                    scale);
      DrawRectangleLinesEx(rect, 1, GREEN);
    }

    const AS::Point<float>* mapPoints = coordMap.cameraTrig.points;
    AS::Point<float> a = mapPoints[0] * scale, b = mapPoints[1] * scale,
                     c = mapPoints[2] * scale;

    detectionOverlay.draw();
    minimapOverlay.draw();

    const Vector2 vertexes[4] = {a.toRaylib(), b.toRaylib(), c.toRaylib(),
                                 a.toRaylib()};
    DrawLineStrip(vertexes, 4, GREEN);

    DrawText("A", a.x, a.y, 16, PINK);
    DrawText("B", b.x, b.y, 16, PINK);
    DrawText("C", c.x, c.y, 16, PINK);

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

