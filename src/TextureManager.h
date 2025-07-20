#pragma once

#include "Constants.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "UI/UIHelper.h"
#include <filesystem>

namespace fs = std::filesystem;

struct MessageTexture {
  SDL_Texture *tex;
  SDL_Color colour;
};

class TextureManager {

public:
  static fs::path fontPath;

  static SDL_Texture *LoadTexture(const char *filePath);

  static void Draw(SDL_Texture *tex, SDL_FRect srcRect, SDL_FRect destRect,
                   SDL_FlipMode flip);

  static SDL_Texture *LoadMessageTexture(const std::string_view text,
                                         float pointsize,
                                         int wraplength = SCREEN_WIDTH,
                                         SDL_Color colour = {0, 0, 0});

  static void DrawRect(SDL_FRect rect, SDL_Color colour);

  static void Panel(SDL_FRect borderRect, SDL_FRect innerRect,
                    SDL_Color borderColour, SDL_Color innerColour);

  static Size GetMessageTextureDimensions(SDL_Texture *messageTex);
};
