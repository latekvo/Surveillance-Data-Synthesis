#pragma once

#include <string>

typedef unsigned int uint;

constexpr int TARGET_FPS = 20;

constexpr float YOLO_SIZE = 640.0;
constexpr float DETECTION_SIZE = 1080;  // downscale, comparable quality

// 25200 for <=v5, 8400 for >=v8, some .onnx add top-n & softmax (our case)
constexpr const uint DNN_OUT_ROWS = 300;
const float DNN_MIN_CONFIDENCE = 0.20;

const std::string STREAMS_FILE = "streams.csv";
const std::string CLASSES_FILE = "coco_labels.listfile";
const std::string OBSERVED_AREAS_FILE = "observed_areas.csv";
constexpr const char* DNN_NET_FILE = "model.onnx";

enum ClassId {
  PERSON = 0,
  BICYCLE = 1,
  CAR = 2,
  MOTORCYCLE = 3,
  BUS = 5,
  TRUCK = 7,
};
