#pragma once

#include "Component.h"
#include <memory>
#include <vector>

class Grid {
public:
  void addChild(std::shared_ptr<Component> child) {
    children.push_back(child);
  };

  void render(SDL_Surface *surface) {
    for (auto &child : children) {
      child->render(surface);
    }
  }

  void update(const SDL_Event &e) {
    for (auto &child : children) {
      child->update(e);
    }
  }

private:
  std::vector<std::shared_ptr<Component>> children;
};
