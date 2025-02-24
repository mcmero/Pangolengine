#pragma once

#include "Grid.h"
#include "Panel.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_surface.h"
#include <memory>

class Manager {
public:
  Manager() { grid.addChild(std::make_shared<Panel>()); }
  ~Manager() {}

  void render(SDL_Surface *surface) { grid.render(surface); }
  void update(const SDL_Event &event) { grid.update(event); };

private:
  Grid grid;
};
