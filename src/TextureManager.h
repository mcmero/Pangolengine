#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <filesystem>

namespace fs = std::filesystem;

class TextureManager {

public:
  static fs::path fontPath;
  static SDL_Texture *LoadTexture(const char *filePath);
  static void Draw(SDL_Texture *tex, SDL_FRect srcRect, SDL_FRect destRect,
                   SDL_FlipMode flip);
  static void Text(const std::string_view text, float pointsize, float xpos,
                   float ypos, int wraplength, SDL_Color colour = {0, 0, 0});
};
