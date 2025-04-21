#pragma once

#include "IComponent.h"
#include <memory>
#include <vector>

class Grid {
public:
  void addChild(std::shared_ptr<IComponent> child) {
    children.push_back(child);
  };

  void render(SDL_Renderer *renderer) {
    for (auto &child : children) {
      child->render(renderer);
    }
  }

  void update(Interactable *i, Dialogue *d) {
    for (auto &child : children) {
      child->update(i, d);
    }
  }

  void handleEvents(const SDL_Event &e) {
    for (auto &child : children) {
      child->handleEvents(e);
    }
  }

  void clean() {
    for (auto &child : children) {
      child->clean();
    }
  }

private:
  std::vector<std::shared_ptr<IComponent>> children;
};
