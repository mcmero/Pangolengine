#pragma once

#include "IComponent.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class Panel : public IComponent {
public:
  Panel(float xpos, float ypos, float width, float height,
        float borderThickness, SDL_Color borderColour, SDL_Color innerColour)
      : borderRect(xpos - borderThickness, ypos - borderThickness,
                   width + 2 * borderThickness, height + 2 * borderThickness),
        innerRect{xpos, ypos, width, height}, borderColour(borderColour),
        innerColour(innerColour) {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      // Draw border rect
      SDL_SetRenderDrawColor(renderer, borderColour.r, borderColour.g,
                             borderColour.b, SDL_ALPHA_OPAQUE);
      SDL_RenderRect(renderer, &borderRect);
      SDL_RenderFillRect(renderer, &borderRect);

      // Draw inner rect
      SDL_SetRenderDrawColor(renderer, innerColour.r, innerColour.g,
                             innerColour.b, SDL_ALPHA_OPAQUE);
      SDL_RenderRect(renderer, &innerRect);
      SDL_RenderFillRect(renderer, &innerRect);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->isActive) {
      show = true;
    }
  }

private:
  bool show = false;
  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;
};
