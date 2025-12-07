#include "detection.h"

#include <onnxruntime_cxx_api.h>
#include <raylib.h>

#include <opencv2/opencv.hpp>
#include <print>
#include <vector>

#include "consts.h"
#include "postprocess.h"
#include "preprocess.h"

Rectangle scaleRect2(Rectangle rect, float scale) {
  return Rectangle{
      rect.x * scale,
      rect.y * scale,
      rect.width * scale,
      rect.height * scale,
  };
}

// TODO: Should accept arbitrairy input size, then fragment if needed
// For simplicity, works with arbitriary square inputs for now
std::vector<Detection> runDetection(Ort::Session& session, cv::Mat& input,
                                    std::vector<std::string>& classList) {
  const float scale = input.cols / YOLO_SIZE;
  cv::Mat frame = toLetterBox(input, YOLO_SIZE);

  Ort::Value tensor = toYoloInputTensor(frame);

  // Note: Inlining inputName into inputNames crashes (why????)
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

    if (score < DNN_MIN_CONFIDENCE) {
      // Our YOLOv10 variant has outputs sorted by TopN
      break;
    }

    std::println("{}: \ts: (x{}:y{})/{}={}, \tu: {},\tv: {}", i, input.cols,
                 input.rows, YOLO_SIZE, scale, score, classId);

    Rectangle rect;

    rect.width = r - l;
    rect.height = b - t;
    rect.x = l;
    rect.y = t;

    rect = scaleRect2(rect, scale);

    detections.push_back(Detection(classId, score, rect));
  }

  return detections;
}

std::vector<Detection> getDetectionsFromFrame(
    Ort::Session& session, cv::Mat& input, std::vector<std::string>& classes) {
  // TODO: Introduce hybrid detection:
  // - use grayskull for programmatic tracking
  // - use HOG for extracting humanoid shapes
  // - use YOLO for verifying hits (in a spearate function,
  //   cram multiple detections into a single YOLO frame mosaic).

  // TODO: move to config
  std::vector<uint> allowedClassIDs = {PERSON,     BICYCLE, CAR,
                                       MOTORCYCLE, BUS,     TRUCK};

  std::vector<DetectionArea> areas = toDetectionAreas(input, DETECTION_SIZE);

  for (DetectionArea& area : areas) {
    std::vector<Detection> results = runDetection(session, area.frame, classes);
    area.detections.insert(area.detections.end(), results.begin(),
                           results.end());
  }

  std::vector<Detection> unfilteredDetections = mergeDetectionAreas(areas);

  return toFilteredDetections(unfilteredDetections, allowedClassIDs);
}
