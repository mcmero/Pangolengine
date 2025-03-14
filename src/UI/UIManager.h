#pragma once

#include "../Components/Dialogue.h"
#include "Grid.h"
#include "Panel.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "Text.h"
#include <memory>

class UIManager {
public:
  SDL_Color fontColour = {0, 0, 0};

  UIManager() {

    grid.addChild(std::make_shared<Panel>(32.0f, 120.0f, 256.0f, 40.0f));
    grid.addChild(std::make_shared<Text>(37.0f, 125.0f, 256.0f, 40.0f, 14.0f,
                                         fontColour));
  }
  ~UIManager() {}

  void render(SDL_Renderer *renderer) { grid.render(renderer); }
  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) {
    grid.update(event, interactable, dialogue);
  };

private:
  Grid grid;
};
