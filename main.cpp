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

std::vector<DetectionArea> toDetectionAreas(cv::Mat& image) {
  std::vector<DetectionArea> areas;

  // TODO: Introduce overlap (10-20px?)

  const uint xCount = uint(std::ceil(float(image.cols) / YOLO_SIZE));
  const uint yCount = uint(std::ceil(float(image.rows) / YOLO_SIZE));

  for (uint y = 0; y < yCount; y++) {
    for (uint x = 0; x < xCount; x++) {
      DetectionArea area;
      const uint xOffset = uint(x * YOLO_SIZE);
      const uint yOffset = uint(y * YOLO_SIZE);
      uint width = YOLO_SIZE;
      uint height = YOLO_SIZE;
      uint widthRemainder = image.cols % uint(YOLO_SIZE);
      uint heightRemainder = image.rows % uint(YOLO_SIZE);

      if (x == xCount - 1 && widthRemainder != 0) {
        width = widthRemainder;
      }

      if (y == yCount - 1 && heightRemainder != 0) {
        height = heightRemainder;
      }

      cv::Mat cropped = image(cv::Rect(xOffset, yOffset, width, height));
      cv::Mat letterBox = toLetterBox(cropped, YOLO_SIZE);

      area.offset = cv::Point(xOffset, yOffset);
      area.frame = letterBox;
      areas.push_back(area);
    }
  }

  return areas;
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

  while (!WindowShouldClose()) {
    screenWidthTarget = GetScreenWidth();
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Skip buffered frames, this is a bit hacky, but found no better way
    // TODO: Calculate ideal skip-rate on the go
    uint skippedFrames = 0;
    while (video.grab() && skippedFrames < 5) {
      skippedFrames++;
    }

    video.read(videoFrame);

    // --- DETECTION LOGIC ---

    std::vector<DetectionArea> areas = toDetectionAreas(videoFrame);

    for (DetectionArea& area : areas) {
      std::vector<Detection> results =
          runDetection(onnxSession, area.frame, classes);
      area.detections.insert(area.detections.end(), results.begin(),
                             results.end());
    }

    std::vector<Detection> detections = mergeDetectionAreas(areas);

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
                              YOLO_SIZE, YOLO_SIZE},
                    scale);
      DrawRectangleLinesEx(rect, 1, GREEN);
    }

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

