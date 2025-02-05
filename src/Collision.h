#pragma once

#include "SDL3/SDL_rect.h"

class Collider;

class Collision {
public:
  static bool AABB(const SDL_FRect &recA, const SDL_FRect &recB);
  static bool AABB(const Collider &colA, const Collider &colB);
};
