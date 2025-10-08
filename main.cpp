#include <raylib.h>

#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <print>

const int TARGET_FPS = 20;

int main() {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "debug display");
  SetTargetFPS(TARGET_FPS);

  cv::VideoCapture video("...");

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
    SetWindowSize(cvFrame.cols, cvFrame.rows);

    Texture2D texture = LoadTextureFromImage(rayImage);

    DrawTexture(texture, 0, 0, WHITE);
    DrawText("Hello world.", 20, 20, 20, GREEN);
    EndDrawing();

    UnloadTexture(texture);
  }

  CloseWindow();

  return 0;
}

