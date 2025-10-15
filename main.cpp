#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <print>

#include "list_parser.h"

typedef unsigned int uint;

const int TARGET_FPS = 20;

std::string STREAMS_FILE = "streams.listfile";
std::string DNN_NET_FILE = "model.onnx";
std::string CLASSES_FILE = "coco_labels.listfile";

const float YOLO_WIDTH = 640.0;
const float YOLO_HEIGHT = 640.0;

const uint CLASS_COUNT = 80;
const uint DNN_OUT_ROWS = 25200;
const float DNN_MIN_CONFIDENCE = 0.8;

enum ClassId {
  PERSON = 0,
  BICYCLE = 1,
  CAR = 2,
  MOTORCYCLE = 3,
  BUS = 5,
  TRUCK = 7,
};

struct Detection {
  uint classIdx;
  float confidence;
  Rectangle rect;
};

constexpr float sigmoid(float value) {
  return 0.5 * (value / (1 + abs(value)) + 1);
}

std::vector<Detection> runDetection(cv::dnn::Net net, cv::Mat frame,
                                    std::vector<std::string> classList) {
  std::vector<Detection> detections;

  cv::Mat blob;
  cv::dnn::blobFromImage(frame, blob, 1. / 255.,
                         cv::Size(YOLO_WIDTH, YOLO_HEIGHT));

  net.setInput(blob);
  std::vector<cv::Mat> outputs;
  net.forward(outputs, net.getUnconnectedOutLayersNames());

  // `data` is a blob, `outputs` contains one such blob. no fragmentation risk
  // initially removing DNN_DIMENSIONS, since it'll be added on the first iter
  float* data = (float*)outputs[0].data;

  for (int i = 0; i < DNN_OUT_ROWS; i++) {
    if (i > 0) {
      data += CLASS_COUNT + 5;
    }

    // data is a blob of floats
    float x = data[0], y = data[1], w = data[2], h = data[3], c = data[4];

    std::println("{}: c: {},\tx: {},\ty: {},\tw: {},\th: {}", i, c, x, y, w, h);

    // Process only detections with confidence above the threshold
    if (c < DNN_MIN_CONFIDENCE) {
      continue;
    }

    // Get class scores and find the class with the highest score
    float* classScores = data + 5;
    cv::Mat scores(1, classList.size(), CV_32FC1, classScores);
    cv::Point classIdx;
    double rawBestScore;
    cv::minMaxLoc(scores, 0, &rawBestScore, 0, &classIdx);

    double bestScore = sigmoid(rawBestScore);

    if (bestScore < DNN_MIN_CONFIDENCE) {
      continue;
    }

    // TODO: Make a whitelist of entities
    if (classIdx.x != ClassId::PERSON) {
      // Allow humans only
      // continue;
    }

    // FIXME: Apparently w & h don't need a sigmoid, but an exp + anchor
    // multiplication (????)
    float xS = sigmoid(x), yS = sigmoid(y), wS = sigmoid(w), hS = sigmoid(h);

    std::println("B: {}, c: {},\tx: {},\ty: {},\tw: {},\th: {}", i, bestScore,
                 xS, yS, wS, hS);

    Rectangle rect;

    // rect.x = (xS - 0.5 * wS) * YOLO_WIDTH;
    // rect.y = (yS - 0.5 * hS) * YOLO_HEIGHT;
    // Using centerpoints for now for debugging
    rect.x = xS * YOLO_WIDTH;
    rect.y = yS * YOLO_HEIGHT;

    // w, h ignored for now for the sake of debugging this
    rect.width = wS * YOLO_WIDTH;
    rect.height = hS * YOLO_HEIGHT;

    detections.push_back(Detection(classIdx.x, c, rect));
  }

  return detections;
}

int main() {
  const int screenWidth = 800;
  const int screenHeight = 450;

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

  cv::Mat cvFrame;
  Image rayImage;

  video.read(cvFrame);

  rayImage.mipmaps = 1;
  rayImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

  cv::dnn::Net yolo = cv::dnn::readNet(DNN_NET_FILE);
  yolo.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  yolo.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
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

    video.read(cvFrame);

    // --- DETECTION LOGIC ---

    cv::Mat detectionFrame = cvFrame(cv::Rect(0, 0, YOLO_WIDTH, YOLO_HEIGHT));
    std::vector<Detection> detections =
        runDetection(yolo, detectionFrame, classes);

    // TODO: runDetection on every fragment of the cvFrame
    // TODO: Apply NMS algorithm to clear overlapping multi-hits

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

    // Video-to-display scaling
    const float xScaling = float(screenWidth) / cvFrame.cols;
    const float yScaling = float(screenHeight) / cvFrame.rows;

    // --- DRAWING CALLS ---

    DrawTexture(texture, 0, 0, WHITE);

    for (const Detection& detection : detections) {
      Rectangle rect = detection.rect;
      const auto classname = classes[detection.classIdx].c_str();

      rect.x = float(rect.x) * xScaling;
      rect.y = float(rect.y) * yScaling;
      rect.width = float(rect.width) * xScaling;
      rect.height = float(rect.height) * yScaling;

      DrawText(classname, rect.x + 2, rect.y - 6, 6, WHITE);
      // DrawRectangleLinesEx(rect, 2.f, WHITE);
      DrawRectangle(rect.x, rect.y, 4, 4, GREEN);
    }

    // Detection square
    DrawRectangleLinesEx(Rectangle(0, 0, int(YOLO_WIDTH * xScaling),
                                   int(YOLO_HEIGHT * yScaling)),
                         2.f, GREEN);

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

