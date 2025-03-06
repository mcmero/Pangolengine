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
      const std::string_view text = "Hey man, fancy seeing you here.";
      TextureManager::Text(text, pointsize, rect.x, rect.y);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable) override {
    if (interactable != nullptr && interactable->isActive) {
      show = true;
    }
  }

private:
  bool show = false;
  float pointsize;
  SDL_FRect rect;
};
