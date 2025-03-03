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

  void update(const SDL_Event &e, Interactable *i) {
    for (auto &child : children) {
      child->update(e, i);
    }
  }

private:
  std::vector<std::shared_ptr<IComponent>> children;
};
