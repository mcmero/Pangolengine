#pragma once

#include "SDL3/SDL_rect.h"

class Camera {
public:
  Camera() = delete;

  static SDL_Rect position;

  static void update(int xpos, int ypos, int mapPixelWidth, int mapPixelHeight);
};
