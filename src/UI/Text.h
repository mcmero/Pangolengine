#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

class Text : public IComponent {
public:
  Text(float xpos, float ypos, float width, float height, float pointsize)
      : pointsize(pointsize), rect{xpos, ypos, width, height} {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      // const std::string_view text = "Hey man, fancy seeing you here.";
      const std::string_view text = dialogueLine;
      TextureManager::Text(text, pointsize, rect.x, rect.y);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->isActive) {
      show = true;
      if (!dialogue->active) {
        dialogue->active = true;
        dialogue->beginDialogue();
      }
      dialogueLine = dialogue->getLine();
    }
  }

private:
  bool show = false;
  float pointsize;
  SDL_FRect rect;
  std::string dialogueLine;
};
