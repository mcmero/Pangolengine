#pragma once

#include "../Camera.h"
#include "../TextureManager.h"
#include "Transform.h"

class Collider {
public:
  SDL_FRect collider;

  SDL_Texture *tex;
  SDL_FRect destRect;
  Offset offset;

  Collider(float xpos, float ypos, float width, float height,
           Offset offset = {0, 0}) {
    collider.x = xpos + offset.x;
    collider.y = ypos + offset.y;
    collider.w = width;
    collider.h = height;

    destRect.x = collider.x - Camera::position.x;
    destRect.y = collider.y - Camera::position.y;
    destRect.w = width;
    destRect.h = height;

    this->offset = offset;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x + offset.x;
    collider.y = transform.position.y + offset.y;

    destRect.x = collider.x - Camera::position.x;
    destRect.y = collider.y - Camera::position.y;
  }

private:
};
