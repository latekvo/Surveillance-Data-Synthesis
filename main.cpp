#include <raylib.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <print>

#include "detection.h"
#include "list_parser.h"
#include "preprocess.h"

typedef unsigned int uint;

const int TARGET_FPS = 20;

std::string STREAMS_FILE = "streams.listfile";
const char* DNN_NET_FILE = "model.onnx";
std::string CLASSES_FILE = "coco_labels.listfile";

const float YOLO_SIZE = 640.0;
const float DISPLAY_WIDTH = 900.0;

// 25200 for <=v5, 8400 for >=v8, some .onnx add top-n & softmax (our case)
const uint DNN_OUT_ROWS = 300;
const float DNN_MIN_CONFIDENCE = 0.20;

enum ClassId {
  PERSON = 0,
  BICYCLE = 1,
  CAR = 2,
  MOTORCYCLE = 3,
  BUS = 5,
  TRUCK = 7,
};

int main() {
  const int screenWidth = YOLO_SIZE;
  const int screenHeight = YOLO_SIZE;

  InitWindow(screenWidth, screenHeight, "debug display");
  SetTargetFPS(TARGET_FPS);

  std::vector streams = parseListFile(STREAMS_FILE);

  if (streams.size() == 0) {
    std::println(
        "Error - No streams provided! Add at least one entry to "
        "'streams.listfile'.");
    return -1;
  }

  cv::VideoCapture video(streams[0], cv::CAP_FFMPEG);

  // Removes buffer, but this doesn't work with most backends.
  // If this line does work, it reduces latency by a lot.
  video.set(cv::CAP_PROP_BUFFERSIZE, 1);

  if (!video.isOpened()) {
    std::println("Error - Could not open video!");
    return -1;
  }

  cv::Mat rawCvFrame;
  cv::Mat cvFrame;
  Image rayImage;

  rayImage.mipmaps = 1;
  rayImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

  const Ort::Env oxxnEnv(ORT_LOGGING_LEVEL_WARNING, "TTGL");
  const Ort::SessionOptions oxxnOptions;
  Ort::Session onnxSession(oxxnEnv, DNN_NET_FILE, oxxnOptions);

  std::vector<std::string> classes = parseListFile(CLASSES_FILE);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Skip buffered frames, this is a bit hacky, but found no better way
    // TODO: Calculate ideal skip-rate on the go
    uint skippedFrames = 0;
    while (video.grab() && skippedFrames < 5) {
      skippedFrames++;
    }

    video.read(rawCvFrame);
    cvFrame = toLetterBox(rawCvFrame, YOLO_SIZE);

    // --- DETECTION LOGIC ---

    std::vector<Detection> detections =
        runDetection(onnxSession, cvFrame, classes);

    // --- RENDERING LOGIC ---

    // OpenCV and raylib use different pixel formats
    cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGR2RGB);

    // remap OpenCV to raylib image, no copy, using shared memory
    rayImage.data = cvFrame.data;
    rayImage.width = cvFrame.cols;
    rayImage.height = cvFrame.rows;

    Texture2D texture = LoadTextureFromImage(rayImage);
    texture.width = screenWidth;
    texture.height = screenHeight;

    // --- DRAWING CALLS ---

    DrawTexture(texture, 0, 0, WHITE);

    for (const Detection& detection : detections) {
      Rectangle rect = detection.rect;
      const auto classname = classes[detection.classIdx].c_str();
      DrawText(classname, rect.x + 2, rect.y - 6, 6, WHITE);
      DrawRectangleLinesEx(rect, 2.f, WHITE);
    }

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

