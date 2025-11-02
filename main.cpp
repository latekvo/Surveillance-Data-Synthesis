#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <cmath>
#include <opencv2/opencv.hpp>
#include <print>
#include <vector>

#include "consts.h"
#include "detection.h"
#include "list_parser.h"
#include "preprocess.h"

std::vector<Detection> mergeDetectionAreas(std::vector<DetectionArea>& areas) {
  std::vector<Detection> merged;

  for (const DetectionArea& area : areas) {
    for (Detection detection : area.detections) {
      detection.rect.x += area.offset.x;
      detection.rect.y += area.offset.y;
      merged.push_back(detection);
    }
  }

  return merged;
}

Rectangle scaleRect(Rectangle rect, float scale) {
  return Rectangle{
      rect.x * scale,
      rect.y * scale,
      rect.width * scale,
      rect.height * scale,
  };
}

cv::Point scalePoint(cv::Point point, float scale) {
  return cv::Point(point.x * scale, point.y * scale);
}

std::vector<DetectionArea> toDetectionAreas(cv::Mat& image,
                                            const float areaSize) {
  std::vector<DetectionArea> areas;

  // TODO: Introduce overlap (10-20px?)

  const uint xCount = uint(std::ceil(float(image.cols) / areaSize));
  const uint yCount = uint(std::ceil(float(image.rows) / areaSize));

  for (uint y = 0; y < yCount; y++) {
    for (uint x = 0; x < xCount; x++) {
      DetectionArea area;
      const uint xOffset = uint(x * areaSize);
      const uint yOffset = uint(y * areaSize);
      uint width = areaSize;
      uint height = areaSize;
      uint widthRemainder = image.cols % uint(areaSize);
      uint heightRemainder = image.rows % uint(areaSize);

      if (x == xCount - 1 && widthRemainder != 0) {
        width = widthRemainder;
      }

      if (y == yCount - 1 && heightRemainder != 0) {
        height = heightRemainder;
      }

      cv::Mat cropped = image(cv::Rect(xOffset, yOffset, width, height));
      cv::Mat letterBox = toLetterBox(cropped, areaSize);

      area.offset = cv::Point(xOffset, yOffset);
      area.frame = letterBox;
      areas.push_back(area);
    }
  }

  return areas;
}

template <typename T>
bool vectorContains(std::vector<T> vec, T element) {
  return std::find(vec.begin(), vec.end(), element) != vec.end();
}

std::vector<Detection> toFilteredDetections(
    std::vector<Detection>& detections, std::vector<uint>& allowedClassIDs) {
  std::vector<Detection> results;

  // This is very inefficient, but both vecs are too small for this to matter
  for (const Detection detection : detections) {
    if (vectorContains(allowedClassIDs, detection.classIdx)) {
      results.push_back(detection);
    }
  }

  return results;
}

int main() {
  // TODO: Make this scalable, variable. For now using video stream dimensions
  int screenWidthTarget = 1280;
  int screenHeight = 720;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidthTarget, screenHeight, "TTGL");
  SetTargetFPS(TARGET_FPS);

  std::vector streams = parseListFile(STREAMS_FILE);

  if (streams.size() == 0) {
    std::println(
        "Error - No streams provided! Add at least one entry to "
        "'streams.listfile'.");
    return -1;
  }

  cv::VideoCapture video(streams[0], cv::CAP_FFMPEG);

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

  std::vector<std::string> classes = parseListFile(CLASSES_FILE);
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

