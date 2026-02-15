#pragma once

#include "../Components/MouseController.h"
#include "IUIComponent.h"
#include <memory>
#include <vector>

class Grid {
public:
  void addChild(std::shared_ptr<IUIComponent> child) {
    children.push_back(child);
  };

  void render(SDL_Renderer *renderer, SDL_Window *window) {
    for (auto &child : children) {
      child->render(renderer, window);
    }
  }

  void update(Interactable *i, Dialogue *d) {
    for (auto &child : children) {
      child->update(i, d);
    }
  }

  void handleEvents(const SDL_Event &e, const MouseInfo &m) {
    for (auto &child : children) {
      child->handleEvents(e, m);
    }
  }

  void clean() {
    for (auto &child : children) {
      child->clean();
    }
  }

private:
  std::vector<std::shared_ptr<IUIComponent>> children;
};
