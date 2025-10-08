#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <print>

#include "list_parser.h"

typedef unsigned int uint;

const int TARGET_FPS = 20;

std::string STREAMS_FILE = "streams.listfile";
std::string DNN_NET_FILE = "model.onnx";
std::string CLASSES_FILE = "coco_labels.listfile";

const float YOLO_WIDTH = 640.0;
const float YOLO_HEIGHT = 640.0;

const uint DNN_DIMENSIONS = 85;	 // 4: rect, 1: conf, 80: class ids

struct Detection {
  uint classIdx;
  float confidence;
  Rectangle rect;  // raylib rect
};

cv::Mat toSquare(const cv::Mat& source, uint size, uint hOffsetIdx,
		 uint vOffsetIdx) {
  int col = source.cols;
  int row = source.rows;
  int _max = MAX(col, row);
  cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
  source.copyTo(result(cv::Rect(0, 0, col, row)));
  return result;
}

std::vector<Detection> runDetection(cv::dnn::Net net, cv::Mat frame,
				    uint sourceWidth, uint sourceHeight) {
  std::vector<Detection> detections;

  // TODO: Do we need scale factor etc?
  cv::Mat blob;
  cv::dnn::blobFromImage(frame, blob, 1. / 255.,
			 cv::Size(YOLO_WIDTH, YOLO_HEIGHT));

  net.setInput(blob);
  std::vector<cv::Mat> outputs;
  net.forward(outputs, net.getUnconnectedOutLayersNames());

  const float scaleX = sourceWidth / YOLO_WIDTH,
	      scaleY = sourceHeight / YOLO_HEIGHT;

  for (const cv::Mat& output : outputs) {
    // data is a blob of floats
    const float* data = (float*)output.data;

    const float confidence = data[4];

    if (confidence < 0.5) {
      std::println("Skipping, did not pass primary threshold");
      // continue;
    }

    const float x = data[0], y = data[1], w = data[2], h = data[3];

    float maxScore = 0;
    uint maxScoreClassId = -1;

    // TODO: Check if minMaxLoc is more efficient
    for (uint i = 5; i < 85; i++) {
      const float score = data[i];
      if (data[i] > maxScore) {
	maxScore = score;
	maxScoreClassId = i;
      }
    }

    const float correctedConfidence = confidence * maxScore;

    if (confidence < 0.5) {
      std::println("Skipping, did not pass secondary threshold");
      // continue;
    }

    // TODO: Apply NMS algorithm to clear overlapping multi-hits

    std::println("Found {} at {} {}", maxScoreClassId, x, y);

    detections.push_back(
	{maxScoreClassId - 4, correctedConfidence,
	 Rectangle(x * scaleX, y * scaleY, w * scaleX, h * scaleY)});
  }

  std::println("Found {} objects.", detections.size());
  return detections;
}

int main() {
  const int screenWidth = YOLO_WIDTH;	 // 800;
  const int screenHeight = YOLO_HEIGHT;	 // 450;

  InitWindow(screenWidth, screenHeight, "debug display");
  SetTargetFPS(TARGET_FPS);

  std::vector streams = parseListFile(STREAMS_FILE);

  if (streams.size() == 0) {
    std::println(
	"Error - No streams provided! Add at least one entry to "
	"'streams.listfile'.");
    return -1;
  }

  cv::VideoCapture video(streams[0]);

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
  yolo.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);  // TODO: Use CUDA
  std::vector<std::string> classes = parseListFile(CLASSES_FILE);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    video.read(cvFrame);

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

    // --- DETECTION LOGIC ---

    cv::Mat yoloInputFrame;
    cv::resize(cvFrame, yoloInputFrame, cv::Size(YOLO_WIDTH, YOLO_HEIGHT));

    std::vector<Detection> detections =
	runDetection(yolo, yoloInputFrame, screenWidth, screenHeight);

    // --- DRAWING CALLS ---

    DrawTexture(texture, 0, 0, WHITE);

    for (const Detection& detection : detections) {
      const Rectangle& rect = detection.rect;
      const auto classname = classes[detection.classIdx].c_str();
      DrawText(classname, rect.x + 2, rect.y + 2, 6, GREEN);
      DrawRectangleLinesEx(detection.rect, 2.f, GREEN);
    }

    EndDrawing();

    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

