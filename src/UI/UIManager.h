#pragma once

#include "../Components/Dialogue.h"
#include "DialoguePanel.h"
#include "DialogueResponsePanel.h"
#include "Grid.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include <memory>

class UIManager {
public:
  float pointsize = 14.0f;
  SDL_Color fontColour = {255, 255, 255};
  SDL_Color selectColour = {208, 199, 125};
  SDL_Color dialogueBorderColour = {0, 0, 0};
  SDL_Color dialogueBoxColour = {51, 64, 113};

  UIManager() {
    grid.addChild(std::make_shared<DialoguePanel>(
        80.0f, 130.0f, 220.0f, 40.0f, 2.0f, dialogueBorderColour,
        dialogueBoxColour, pointsize, fontColour));
    grid.addChild(std::make_shared<DialogueResponsePanel>(
        80.0f, 10.0f, 220.0f, 40.0f, 2.0f, dialogueBorderColour,
        dialogueBoxColour, pointsize, fontColour, selectColour));
  }
  ~UIManager() { grid.clean(); }

  void render(SDL_Renderer *renderer) { grid.render(renderer); }
  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) {

    // handle UI events
    if (event.type == SDL_EVENT_KEY_UP) {
      switch (event.key.key) {
      case SDLK_Q:
        std::cout << "Quit dialogue!" << std::endl;
        if (dialogue != nullptr) {
          dialogue->active = false;
        }
        if (interactable != nullptr) {
          interactable->active = false;
        }
        break;
      default:
        break;
      }
    }
    grid.update(event, interactable, dialogue);
  };

private:
  Grid grid;
};
