#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <print>

#include "list_parser.h"

const int TARGET_FPS = 20;
std::string STREAMS_FILE = "streams.listfile";

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

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    video.read(cvFrame);

    // use color channel translation if all else fails:
    cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGR2RGB);

    // remap OpenCV to raylib image, no copy, using shared memory
    // FIXME: size data not available until ~10 frames
    rayImage.data = cvFrame.data;
    rayImage.width = cvFrame.cols;
    rayImage.height = cvFrame.rows;

    Texture2D texture = LoadTextureFromImage(rayImage);
    texture.width = 800;
    texture.height = 450;

    DrawTexture(texture, 0, 0, WHITE);
    DrawText("Hello world.", 20, 20, 20, GREEN);
    EndDrawing();

    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

