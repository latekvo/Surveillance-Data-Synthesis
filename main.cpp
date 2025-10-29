#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <print>

#include "list_parser.h"
#include "preprocess.h"

typedef unsigned int uint;

const int TARGET_FPS = 20;

std::string STREAMS_FILE = "streams.listfile";
const char* DNN_NET_FILE = "model.onnx";
std::string CLASSES_FILE = "coco_labels.listfile";

const float YOLO_SIZE = 640.0;

// 25200 for <=v5, 8400 for >=v8, some .onnx add top-n & softmax (our case)
const uint DNN_OUT_ROWS = 300;
const float DNN_MIN_CONFIDENCE = 0.15;

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
    std::println("Invalid input dimensions. Check 'runDetection' input.");
    exit(1);
  }

  Ort::Value tensor = toYoloInputTensor(frame);

  const char* inputNames[] = {session.GetInputNames()[0].c_str()};
  const char* outputNames[] = {session.GetOutputNames()[0].c_str()};

  std::vector<Ort::Value> outputs = session.Run(
      Ort::RunOptions{nullptr}, inputNames, &tensor, 1, outputNames, 1);

  std::vector<int64_t> outputDims =
      outputs[0].GetTensorTypeAndShapeInfo().GetShape();

  float* data = (float*)outputs[0].GetTensorRawData();

  std::vector<Detection> detections;

  for (int i = 0; i < DNN_OUT_ROWS; i++) {
    if (i > 0) {
      data += 6;
    }

    // `data` is a blob of output values
    int64_t l = data[0], t = data[1], r = data[2], b = data[3],
            classId = data[5];
    float score = data[4];

    std::println("{}: \tl: {},\tr: {},\tt: {},\tb: {},\tu: {},\tv: {}", i, l, r,
                 t, b, score, classId);

    if (score < DNN_MIN_CONFIDENCE) {
      // Our YOLOv10 variant has outputs sorted by TopN
      break;
    }

    // TODO: Make a whitelist of entities
    if (classId != ClassId::PERSON) {
      // Allow humans only
      // continue;
    }

    Rectangle rect;

    rect.width = r - l;
    rect.height = b - t;
    rect.x = l;
    rect.y = t;

    detections.push_back(Detection(classId, score, rect));
  }

  return detections;
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

