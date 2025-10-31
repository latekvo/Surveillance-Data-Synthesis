#pragma once

#include <string>

typedef unsigned int uint;

const int TARGET_FPS = 20;

const float YOLO_SIZE = 640.0;
const float DISPLAY_WIDTH = 900.0;

// 25200 for <=v5, 8400 for >=v8, some .onnx add top-n & softmax (our case)
const uint DNN_OUT_ROWS = 300;
const float DNN_MIN_CONFIDENCE = 0.20;

const std::string STREAMS_FILE = "streams.listfile";
const std::string CLASSES_FILE = "coco_labels.listfile";
constexpr const char* DNN_NET_FILE = "model.onnx";

enum ClassId {
  PERSON = 0,
  BICYCLE = 1,
  CAR = 2,
  MOTORCYCLE = 3,
  BUS = 5,
  TRUCK = 7,
};
