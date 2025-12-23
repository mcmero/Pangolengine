#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "Transform.h"
#include <string>

class Transition {
public:
  SDL_FRect collider;
  std::string mapPath;
  Mix_Chunk *sound;

  Transition(Transform &transform, std::string mapPath, Mix_Chunk *sound) {
    collider.x = transform.position.x;
    collider.y = transform.position.y;
    collider.w = transform.width;
    collider.h = transform.height;
    this->mapPath = mapPath;
    this->sound = sound;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x;
    collider.y = transform.position.y;
  }

private:
};
