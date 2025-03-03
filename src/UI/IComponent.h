#pragma once

#include "../Components/Interactable.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"

class IComponent {
public:
  virtual ~IComponent() = default;
  virtual void update(const SDL_Event &event, Interactable *interactable) {}
  virtual void render(SDL_Renderer *renderer) {}
};
