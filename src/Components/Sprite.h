#pragma once

#include "../Game.h"
#include "../TextureManager.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "Transform.h"

class Sprite {
public:
  Sprite(const char *texturePath) {
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.h = 32;
    srcRect.w = 32;

    texture = TextureManager::LoadTexture(texturePath);
  }

  void update(Transform &transform) {
    destRect.x = float(transform.x());
    destRect.y = float(transform.y());
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
};
