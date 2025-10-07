#include <raylib.h>

const int TARGET_FPS = 20;

int main() {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "debug display");

  SetTargetFPS(TARGET_FPS);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("Hello world.", 190, 200, 20, LIGHTGRAY);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

