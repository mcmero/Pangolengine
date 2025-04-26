#pragma once

#include "SDL3/SDL_rect.h"
#include "Transform.h"
#include <string>

class Transition {
public:
  SDL_FRect collider;
  std::string mapPath;

  Transition(Transform &transform, std::string mapPath) {
    collider.x = transform.position.x;
    collider.y = transform.position.y;
    collider.w = transform.width;
    collider.h = transform.height;
    this->mapPath = mapPath;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x;
    collider.y = transform.position.y;
  }

private:
};
