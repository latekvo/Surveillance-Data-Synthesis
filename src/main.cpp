#include <onnxruntime_cxx_api.h>
#include <raylib.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>
#include <print>
#include <vector>

#include "coco_labels.h"
#include "components/DetectionOverlay.hpp"
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
  Image rayImage;

  rayImage.mipmaps = 1;
  rayImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

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

    float scale = screenWidthTarget / (float)videoFrame.cols;
    screenHeight = (float)videoFrame.rows * scale;

    std::vector<Detection> detections =
        getDetectionsFromFrame(onnxSession, videoFrame, classes);

    // --- RENDERING LOGIC ---

    static auto detectionOverlay =
        AS::DetectionOverlay(&detections, &classes, &scale);

    static auto minimapOverlay =
        AS::MinimapOverlay(&detections, &coordMap, &scale);

    static auto pixelPicker = AS::PixelPicker(&scale);

    if (IsWindowResized()) {
      SetWindowSize(screenWidthTarget, screenHeight);
    }

    cv::resize(videoFrame, videoFrame,
               cv::Size(screenWidthTarget, screenHeight));

    // TODO: Apply this to YOLO input too?
    // OpenCV and raylib use different pixel formats
    cv::cvtColor(videoFrame, videoFrame, cv::COLOR_BGR2RGB);

    // remap OpenCV to raylib image, no copy, using shared memory
    rayImage.data = videoFrame.data;
    rayImage.width = videoFrame.cols;
    rayImage.height = videoFrame.rows;

    Texture2D texture = LoadTextureFromImage(rayImage);

    // --- DRAWING CALLS ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawTexture(texture, 0, 0, WHITE);

    const AS::Point<float>* mapPoints = coordMap.cameraTrig.points;
    AS::Point<float> a = mapPoints[0] * scale, b = mapPoints[1] * scale,
                     c = mapPoints[2] * scale;

    detectionOverlay.draw();
    minimapOverlay.draw();
    pixelPicker.draw();

    const Vector2 vertexes[4] = {a.toRaylib(), b.toRaylib(), c.toRaylib(),
                                 a.toRaylib()};
    DrawLineStrip(vertexes, 4, GREEN);

    DrawText("A", a.x, a.y, 16, PINK);
    DrawText("B", b.x, b.y, 16, PINK);
    DrawText("C", c.x, c.y, 16, PINK);

    EndDrawing();
    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

