#include <onnxruntime/onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <print>

#include "list_parser.h"

typedef unsigned int uint;

const int TARGET_FPS = 20;

std::string STREAMS_FILE = "streams.listfile";
const char* DNN_NET_FILE = "model.onnx";
std::string CLASSES_FILE = "coco_labels.listfile";

const float YOLO_SIZE = 640.0;

// 25200 for <=v5, 8400 for >=v8, TODO: Extract from .onnx file, CAN BE DONE!
const uint DNN_OUT_ROWS = 8400;
const uint CLASS_COUNT = 80;  // output dims are 4 + c, where this is c
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

std::vector<Detection> runDetection(Ort::Session& session, cv::Mat frame,
                                    std::vector<std::string> classList) {
  if (frame.cols != YOLO_SIZE || frame.rows != YOLO_SIZE) {
    std::println("Invalid Net input dimensions. Check 'runDetection' input.");
    exit(1);
  }

  cv::Mat blob;
  cv::dnn::blobFromImage(frame, blob, 1. / 255., cv::Size(YOLO_SIZE, YOLO_SIZE),
                         true, false, CV_32F);

  if (blob.empty()) {
    std::println("Failed loading net input blob.");
    exit(1);
  }

  // standard input for all YOLOs
  // TODO: Experiment with variable input side in YOLOv11
  std::vector<int64_t> inputShape = {1, 3, 640, 640};
  int64_t inputSize = 3 * 640 * 640;

  Ort::MemoryInfo memoryInfo =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  Ort::Value tensor = Ort::Value::CreateTensor<float>(
      memoryInfo, (float*)(blob.data), inputSize, inputShape.data(),
      inputShape.size());

  // TODO: I think we can do this without `allocator`
  Ort::AllocatorWithDefaultOptions allocator;
  const std::string inputName =
      session.GetInputNameAllocated(0, allocator).get();
  const std::string outputName =
      session.GetOutputNameAllocated(0, allocator).get();

  const char* inputNames[] = {inputName.c_str()};
  const char* outputNames[] = {outputName.c_str()};

  std::vector<Ort::Value> outputs = session.Run(
      Ort::RunOptions{nullptr}, inputNames, &tensor, 1, outputNames, 1);

  std::vector<int64_t> outputDims =
      outputs[0].GetTensorTypeAndShapeInfo().GetShape();

  auto output = cv::Mat(cv::Size(static_cast<int>(outputDims[2]),
                                 static_cast<int>(outputDims[1])),
                        CV_32F, outputs[0].GetTensorMutableData<float>());

  // TODO: Why do we need to initiate the transposal?
  cv::Mat boxes = output.t();

  if (output.cols == -1 || output.rows == -1) {
    std::println("cv::dnn::Net.forward() failed. Check net input.");
    exit(1);
  }

  // `data` is a blob, `outputs` contains one such blob. no fragmentation risk
  float* data = reinterpret_cast<float*>(output.data);

  std::vector<Detection> detections;

  for (int i = 0; i < DNN_OUT_ROWS; i++) {
    if (i > 0) {
      data += CLASS_COUNT + 4;
    }

    // data is a blob of floats
    float x = data[0], y = data[1], w = data[2], h = data[3];

    if (w == 0 || h == 0) {
      // TODO: Confirm that some low-conf are intentionally zeroed
      continue;
    }

    std::println("{}: \tx: {},\ty: {},\tw: {},\th: {}", i, x, y, w, h);

    // Get class scores and find the class with the highest score
    float* classScores = data + 4;
    cv::Mat scores(1, classList.size(), CV_32FC1, classScores);
    cv::Point classIdx;
    double bestScore;
    cv::minMaxLoc(scores, 0, &bestScore, 0, &classIdx);

    if (bestScore < DNN_MIN_CONFIDENCE) {
      continue;
    }

    // TODO: Make a whitelist of entities
    if (classIdx.x != ClassId::PERSON) {
      // Allow humans only
      // continue;
    }

    Rectangle rect;

    rect.x = (x - 0.5 * w);
    rect.y = (y - 0.5 * h);

    rect.width = w;
    rect.height = h;

    detections.push_back(Detection(classIdx.x, bestScore, rect));
  }

  return detections;
}

cv::Mat imageToYoloFrame(cv::Mat frame) {
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

  const Ort::Env oxxnEnv(ORT_LOGGING_LEVEL_WARNING, "TTGL");
  const Ort::SessionOptions oxxnOptions;
  Ort::Session onnxSession(oxxnEnv, DNN_NET_FILE, oxxnOptions);

  // cv::dnn::Net yolo = cv::dnn::readNetFromONNX(DNN_NET_FILE);
  // yolo.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  // yolo.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

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
      DrawRectangle(rect.x, rect.y, 4, 4, GREEN);
    }

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

