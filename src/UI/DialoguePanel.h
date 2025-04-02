#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class DialoguePanel : public IComponent {
public:
  DialoguePanel(float xpos, float ypos, float width, float height,
                float borderThickness, SDL_Color borderColour,
                SDL_Color innerColour, float pointsize, SDL_Color fontColour)
      : borderRect{xpos - borderThickness, ypos - borderThickness,
                   width + 2 * borderThickness, height + 2 * borderThickness},
        innerRect{xpos, ypos, width, height}, borderColour(borderColour),
        innerColour(innerColour),
        textRect(xpos + 5.0f, ypos + 2.0f, width - 5.0f, height - 5.0f),
        fontColour(fontColour), pointsize(pointsize) {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      TextureManager::Panel(borderRect, innerRect, borderColour, innerColour);
      const std::string_view text = dialogueLine;
      TextureManager::Text(text, pointsize, textRect.x, textRect.y,
                           static_cast<int>(textRect.w), fontColour);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active) {
      show = true;

      if (!dialogue->active) {
        dialogue->active = true;
        dialogue->beginDialogue();
        std::cout << "Begin dialogue" << std::endl;
      }
      dialogueLine = dialogue->getLine();
    } else
      show = false;
  }

private:
  bool show = false;

  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;

  SDL_FRect textRect;
  SDL_Color fontColour;
  float pointsize;
  std::string dialogueLine;
};
