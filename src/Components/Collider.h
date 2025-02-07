#pragma once

#include "../Game.h"
#include "../TextureManager.h"
#include "Transform.h"

struct Offset {
  float x = 0;
  float y = 0;
  float w = 0;
  float h = 0;
};

class Collider {
public:
  SDL_FRect collider;

  SDL_Texture *tex;
  SDL_FRect srcRect, destRect;
  Offset offset;

  Collider(float xpos, float ypos, float width, float height,
           Offset offset = {0, 0, 0, 0}) {
    this->offset = offset;
    collider.x = xpos + offset.x;
    collider.y = ypos + offset.y;
    collider.w = width + offset.w;
    collider.h = height + offset.h;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x + offset.x;
    collider.y = transform.position.y + offset.y;
    collider.w = transform.width + offset.w;
    collider.h = transform.height + offset.h;

    destRect.x = collider.x - Game::camera.x;
    destRect.y = collider.y - Game::camera.y;
  }

  void draw() { TextureManager::Draw(tex, srcRect, destRect, SDL_FLIP_NONE); }

private:
};
