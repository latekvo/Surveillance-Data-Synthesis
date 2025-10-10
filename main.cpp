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

const uint DNN_DIMENSIONS = 85;	 // 4: rect, 1: conf, 80: class ids
const uint DNN_OUT_ROWS = 25200;
const float DNN_MIN_CONFIDENCE = 0.5;

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
				    std::vector<std::string> classList,
				    uint sourceWidth, uint sourceHeight) {
  std::vector<Detection> detections;

  cv::Mat blob;
  cv::dnn::blobFromImage(frame, blob, 1. / 255.,
			 cv::Size(YOLO_WIDTH, YOLO_HEIGHT));

  net.setInput(blob);
  std::vector<cv::Mat> outputs;
  net.forward(outputs, net.getUnconnectedOutLayersNames());

  const float scaleX = sourceWidth / YOLO_WIDTH,
	      scaleY = sourceHeight / YOLO_HEIGHT;

  // `data` is a blob, `outputs` contains one such blob. no fragmentation risk
  float* data = (float*)outputs[0].data;

  for (int i = 0; i < DNN_OUT_ROWS - 1; i++) {
    // data is a blob of floats
    float confidence = data[4];

    // Process only detections with confidence above the threshold
    if (confidence > DNN_MIN_CONFIDENCE) {
      // Get class scores and find the class with the highest score
      float* classScores = data + 5;
      cv::Mat scores(1, classList.size(), CV_32FC1, classScores);
      cv::Point classIdx;
      double bestScore;
      cv::minMaxLoc(scores, 0, &bestScore, 0, &classIdx);

      // Allow humans only
      // TODO: Make a whitelist of entities
      if (classIdx.x != ClassId::PERSON) {
	continue;
      }

      // If the class score is above the threshold, store the detection
      if (bestScore > DNN_MIN_CONFIDENCE) {
	const float x = data[0], y = data[1], w = data[2], h = data[3];

	int left = int((x - 0.5 * w) * scaleX);
	int top = int((y - 0.5 * h) * scaleY);
	int width = int(w * scaleX);
	int height = int(h * scaleY);

	detections.push_back(Detection(classIdx.x, confidence,
				       Rectangle(left, top, width, height)));
      }
    }
    // TODO: Apply NMS algorithm to clear overlapping multi-hits

    data += DNN_DIMENSIONS;
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
  yolo.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);  // TODO: Use CUDA
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

    cv::resize(cvFrame, cvFrame, cv::Size(YOLO_WIDTH, YOLO_HEIGHT));

    std::vector<Detection> detections =
	runDetection(yolo, cvFrame, classes, screenWidth, screenHeight);

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
      const Rectangle& rect = detection.rect;
      const auto classname = classes[detection.classIdx].c_str();

      DrawText(classname, rect.x + 2, rect.y - 6, 6, GREEN);
      DrawRectangleLinesEx(detection.rect, 2.f, GREEN);
    }

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

