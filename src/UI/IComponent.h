#pragma once

#include "../Components/Dialogue.h"
#include "../Components/Interactable.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"

class IComponent {
public:
  virtual ~IComponent() = default;
  virtual void update(Interactable *interactable, Dialogue *dialogue) {}
  virtual void render(SDL_Renderer *renderer, SDL_Window *window) {}
  virtual void handleEvents(const SDL_Event &event) {}
  virtual void clean() {}
};
