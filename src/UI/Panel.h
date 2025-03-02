#pragma once

#include "IComponent.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class Panel : public IComponent {
public:
  Panel(float xpos, float ypos, float width, float height)
      : rect{xpos, ypos, width, height} {}

  void render(SDL_Renderer *renderer) override {

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &rect);
    SDL_RenderFillRect(renderer, &rect);
  }

  void update(const SDL_Event &event) override {}

private:
  SDL_FRect rect;
};
