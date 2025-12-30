#include <onnxruntime_cxx_api.h>
#include <raylib.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>
#include <print>
#include <vector>

#include "coco_labels.h"
#include "components/CameraView.hpp"
#include "components/MinimapOverlay.hpp"
#include "components/PixelPicker.hpp"
#include "consts.h"
#include "detection.h"
#include "remapper.h"
#include "streams.h"

int main() {
  int screenWidthTarget = 1280;
  int screenHeight = 720;
  // size_t trackedVideoIds[4] = {0, 1, 2, 3};  // TODO: Do this by ref

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidthTarget, screenHeight, "TTGL");
  SetTargetFPS(TARGET_FPS);

  std::vector coordMaps = loadCoordMaps();
  std::vector streams = loadStreams();

  // std::vector<cv::VideoCapture> videos;

  // for (const size_t& id : trackedVideoIds) {
  //   cv::VideoCapture video(streams[id].url, cv::CAP_FFMPEG);

  //   if (!video.isOpened()) {
  //     std::println("Error - Could not open video {}!", id);
  //     return -1;
  //   }

  //   videos.push_back(video);
  // }

  cv::VideoCapture video(streams[0].url, cv::CAP_FFMPEG);

  if (!video.isOpened()) {
    std::println("Error - Could not open video!");
    return -1;
  }

  cv::Mat videoFrame;
  cv::Mat detectionFrame;

  const Ort::Env oxxnEnv(ORT_LOGGING_LEVEL_WARNING, "TTGL");
  const Ort::SessionOptions oxxnOptions;
  Ort::Session onnxSession(oxxnEnv, DNN_NET_FILE, oxxnOptions);

  std::vector<std::string> classes = getCocoLabels();
  std::vector<uint> allowedClassIDs = {PERSON,     BICYCLE, CAR,
                                       MOTORCYCLE, BUS,     TRUCK};

  CoordMap coordMap;
  for (const CoordMap& map : coordMaps) {
    if (map.cameraRef == streams[0].name) {
      coordMap = map;
    }
  }

  while (!WindowShouldClose()) {
    screenWidthTarget = GetScreenWidth();

    // for (auto& video : videos) {
    //   video.read(videoFrame);
    // }

    video.read(videoFrame);
    cv::cvtColor(videoFrame, videoFrame, cv::COLOR_BGR2RGB);

    float scale = screenWidthTarget / (float)videoFrame.cols;
    screenHeight = (float)videoFrame.rows * scale;

    std::vector<Detection> detections =
        getDetectionsFromFrame(onnxSession, videoFrame, classes);

    cv::resize(videoFrame, videoFrame,
               cv::Size(screenWidthTarget, screenHeight));

    // --- RENDERING LOGIC ---

    static auto minimapOverlay =
        AS::MinimapOverlay(&detections, &coordMap, &scale);

    static auto pixelPicker = AS::PixelPicker(&scale);

    static auto cameraView =
        AS::CameraView({}, &scale, &detections, &coordMap, &videoFrame);

    if (IsWindowResized()) {
      SetWindowSize(screenWidthTarget, screenHeight);
    }

    // --- DRAWING CALLS ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // NOTE: Detections have to be performed centrally - outside of components
    // TODO: Add a central detections registry - aggregated by camera id, async
    // TODO: Isolate detections to separate threat to fix large delays

    cameraView.draw();
    minimapOverlay.draw();
    pixelPicker.draw();

    EndDrawing();

    cameraView.postRender();
  }

  CloseWindow();

  return 0;
}

