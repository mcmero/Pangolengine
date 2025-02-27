#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"

class Component {
public:
  virtual void update(const SDL_Event &event) {}
  virtual void render(SDL_Renderer *renderer) {}
};
