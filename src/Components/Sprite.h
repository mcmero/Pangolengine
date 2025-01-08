#pragma once

#include "../TextureManager.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <iostream>

class Sprite {
public:
  Sprite(const char *texturePath, int x, int y) {
    texture = TextureManager::LoadTexture(texturePath);

    xpos = x;
    ypos = y;
  }

  void update() {
    if (xpos > 320)
      xpos = 0;
    if (ypos > 180)
      ypos = 0;

    xpos++;
    ypos++;

    std::cout << "x: " << xpos << " y: " << ypos << std::endl;

    srcRect.h = 32;
    srcRect.w = 32;
    srcRect.x = 0;
    srcRect.y = 0;

    destRect.x = float(xpos);
    destRect.y = float(ypos);
    destRect.w = srcRect.w;
    destRect.h = srcRect.h;
  }

  void render() {
    SDL_RenderTexture(Game::renderer, texture, &srcRect, &destRect);
  }

  void clean() { SDL_DestroyTexture(texture); }

private:
  SDL_Texture *texture;
  SDL_FRect srcRect, destRect;
  int xpos, ypos;
};
