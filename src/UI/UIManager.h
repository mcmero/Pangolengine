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
  SDL_Color fontColour = {255, 255, 255};
  SDL_Color dialogueBorderColour = {0, 0, 0};
  SDL_Color dialogueBoxColour = {51, 64, 113};

  UIManager() {

    grid.addChild(std::make_shared<Panel>(80.0f, 130.0f, 215.0f, 40.0f, 2.0f,
                                          dialogueBorderColour,
                                          dialogueBoxColour));
    grid.addChild(std::make_shared<Text>(85.0f, 132.0f, 210.0f, 35.0f, 14.0f,
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
