#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <print>

#include "list_parser.h"

typedef unsigned int uint;

const int TARGET_FPS = 20;

std::string STREAMS_FILE = "streams.listfile";
std::string DNN_NET_FILE = "yolo5s_static.onnx";
std::string CLASSES_FILE = "coco_labels.listfile";

const float YOLO_SIZE = 640.0;

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

std::vector<Detection> runDetection(cv::dnn::Net net, cv::Mat frame,
                                    std::vector<std::string> classList) {
  if (frame.cols != YOLO_SIZE || frame.rows != YOLO_SIZE) {
    std::println("Invalid Net input dimensions. Check 'runDetection' input.");
    exit(1);
  }

  cv::Mat blob;
  cv::dnn::blobFromImage(frame, blob, 1. / 255., cv::Size(YOLO_SIZE, YOLO_SIZE),
                         true, false);

  if (blob.empty()) {
    std::println("Failed loading net input blob.");
    exit(1);
  }

  net.setInput(blob);
  std::vector<cv::Mat> outputs;
  net.forward(outputs, net.getUnconnectedOutLayersNames());

  cv::Mat output = outputs[0];

  if (output.cols == -1 || output.rows == -1) {
    std::println("cv::dnn::Net.forward() failed. Check net input.");
    exit(1);
  }

  // `data` is a blob, `outputs` contains one such blob. no fragmentation risk
  float* data = (float*)output.data;

  std::vector<Detection> detections;

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

    double bestScore = rawBestScore;

    if (bestScore < DNN_MIN_CONFIDENCE) {
      continue;
    }

    // TODO: Make a whitelist of entities
    if (classIdx.x != ClassId::PERSON) {
      // Allow humans only
      // continue;
    }

    Rectangle rect;

    // rect.x = (xS - 0.5 * wS) * YOLO_WIDTH;
    // rect.y = (yS - 0.5 * hS) * YOLO_HEIGHT;
    // Using centerpoints for now for debugging
    rect.x = x * YOLO_SIZE;
    rect.y = y * YOLO_SIZE;

    // w, h ignored for now for the sake of debugging this
    rect.width = w * YOLO_SIZE;
    rect.height = h * YOLO_SIZE;

    detections.push_back(Detection(classIdx.x, c, rect));
  }

  return detections;
}

cv::Mat imageToYoloFrame(cv::Mat frame) {
  std::println("foobar");
  uint x = frame.cols, y = frame.rows;
  uint max = std::max(x, y);
  float scale = float(YOLO_SIZE) / float(max);
  uint rX = std::floor(x * scale), rY = std::floor(y * scale);
  cv::Mat adjusted;
  cv::resize(frame, adjusted, cv::Size(rX, rY));
  cv::Mat output = cv::Mat::zeros(YOLO_SIZE, YOLO_SIZE, CV_8UC3);
  adjusted.copyTo(output(cv::Rect(0, 0, adjusted.cols, adjusted.rows)));
  return output;
}

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

  cv::dnn::Net yolo = cv::dnn::readNetFromONNX(DNN_NET_FILE);
  yolo.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  yolo.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
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
    cvFrame = imageToYoloFrame(rawCvFrame);

    // --- DETECTION LOGIC ---

    std::vector<Detection> detections = runDetection(yolo, cvFrame, classes);

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

      rect.x = float(rect.x);
      rect.y = float(rect.y);
      rect.width = float(rect.width);
      rect.height = float(rect.height);

      // DrawText(classname, rect.x + 2, rect.y - 6, 6, WHITE);
      // DrawRectangleLinesEx(rect, 2.f, WHITE);
      DrawRectangle(rect.x, rect.y, 4, 4, GREEN);
    }

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

