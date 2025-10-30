#include "detection.h"

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>
#include <print>

#include "preprocess.h"

const float YOLO_SIZE = 640.0;

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

std::vector<Detection> runDetection(Ort::Session& session, cv::Mat frame,
                                    std::vector<std::string> classList) {
  if (frame.cols != YOLO_SIZE || frame.rows != YOLO_SIZE) {
    std::println("Invalid input dimensions. Check 'runDetection' input.");
    exit(1);
  }

  Ort::Value tensor = toYoloInputTensor(frame);

  // Note: Inlining inputName into inputNames crashes
  const std::string inputName = session.GetInputNames()[0];
  const std::string outputName = session.GetOutputNames()[0];
  const char* inputNames[] = {inputName.c_str()};
  const char* outputNames[] = {outputName.c_str()};

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
    int32_t l = data[0], t = data[1], r = data[2], b = data[3],
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
