#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class TextureManager {

public:
  static SDL_Texture *LoadTexture(const char *filePath);
  static void Draw(SDL_Texture *tex, SDL_FRect srcRect, SDL_FRect destRect);
};
