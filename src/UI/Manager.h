#pragma once

#include "Grid.h"
#include "Panel.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include <memory>

class Manager {
public:
  Manager() {
    grid.addChild(std::make_shared<Panel>(32.0f, 120.0f, 256.0f, 40.0f));
  }
  ~Manager() {}

  void render(SDL_Renderer *renderer) { grid.render(renderer); }
  void update(const SDL_Event &event) { grid.update(event); };

private:
  Grid grid;
};
