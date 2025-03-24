#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class DialogueText : public IComponent {
public:
  DialogueText(float xpos, float ypos, float width, float height,
               float pointsize, SDL_Color fontColour)
      : pointsize(pointsize), fontColour(fontColour),
        rect{xpos, ypos, width, height} {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      const std::string_view text = dialogueLine;
      TextureManager::Text(text, pointsize, rect.x, rect.y,
                           static_cast<int>(rect.w), fontColour);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active) {
      show = true;

      if (!dialogue->active) {
        dialogue->active = true;
        dialogue->beginDialogue();
      }
      dialogueLine = dialogue->getLine();
    } else
      show = false;
  }

private:
  bool show = false;
  float pointsize;
  SDL_Color fontColour;
  SDL_FRect rect;
  std::string dialogueLine;
};
