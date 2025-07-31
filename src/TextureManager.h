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

struct Margin {
  float top = 0;
  float bottom = 0;
  float left = 0;
  float right = 0;
};

struct TextProperties {
  std::string text = "";
  float pointsize = 14.0f;
  int wraplength = SCREEN_WIDTH;
  SDL_Color colour = {0, 0, 0};
  Align horizontalAlign = Align::Center;
  Align verticalAlign = Align::Top;
  Margin margin = {};
};

struct ButtonProperties {
  Size size = {60.0f, 12.0f};
  SDL_Color colour = {255, 255, 255};
  Align horizontalAlign = Align::Center;
  Align verticalAlign = Align::Top;
  TextProperties textProps = {};
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

  static void DrawPanel(SDL_FRect borderRect, SDL_FRect innerRect,
                        SDL_Color borderColour, SDL_Color innerColour);

  static void DrawText(TextProperties textProps,
                       SDL_FRect const &containerRect);

  static void DrawButton(ButtonProperties buttonProps,
                         SDL_FRect const &containerRect, float buttonSpacing);

  static Size GetMessageTextureDimensions(SDL_Texture *messageTex);
};
