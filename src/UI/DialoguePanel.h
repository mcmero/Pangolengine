#pragma once

#include "IComponent.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class DialoguePanel : public IComponent {
public:
  DialoguePanel(float xpos, float ypos, float width, float height,
                float borderThickness, SDL_Color borderColour,
                SDL_Color innerColour, ComponentType compType)
      : borderRect(xpos - borderThickness, ypos - borderThickness,
                   width + 2 * borderThickness, height + 2 * borderThickness),
        innerRect{xpos, ypos, width, height}, borderColour(borderColour),
        innerColour(innerColour), compType(compType) {}

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
    // TODO: we need to mark the dialogue as active before we
    // draw the text, as this is causing a delay in the panel
    // appearing under the dialogue test
    if (interactable != nullptr && interactable->active &&
        dialogue != nullptr && dialogue->active) {
      if (compType == RESPONSE && dialogue->getResponses().empty()) {
        // No responses to show
        show = false;
      } else {
        show = true;
      }
    } else {
      show = false;
    }
  }

private:
  bool show = false;
  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;
  ComponentType compType;
};
