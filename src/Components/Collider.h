#pragma once

#include "../Camera.h"
#include "../TextureManager.h"
#include "Transform.h"

// Offset is used for the positioning of the component,
// relative to its transform component
struct Offset {
  float x = 0;
  float y = 0;
};

class Collider {
public:
  SDL_FRect collider;

  SDL_Texture *tex;
  SDL_FRect srcRect, destRect;
  Offset offset;

  Collider(float xpos, float ypos, float width, float height,
           Offset offset = {0, 0}) {
    this->offset = offset;
    collider.x = xpos + offset.x;
    collider.y = ypos + offset.y;
    collider.w = width;
    collider.h = height;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x + offset.x;
    collider.y = transform.position.y + offset.y;

    destRect.x = collider.x - Camera::position.x;
    destRect.y = collider.y - Camera::position.y;
  }

  void draw() { TextureManager::Draw(tex, srcRect, destRect, SDL_FLIP_NONE); }

private:
};
