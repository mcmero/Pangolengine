#pragma once

#include "../Components/Dialogue.h"
#include "DialoguePanel.h"
#include "DialogueText.h"
#include "Grid.h"
#include "IComponent.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include <memory>

class UIManager {
public:
  SDL_Color fontColour = {255, 255, 255};
  SDL_Color dialogueBorderColour = {0, 0, 0};
  SDL_Color dialogueBoxColour = {51, 64, 113};

  UIManager() {
    grid.addChild(std::make_shared<DialoguePanel>(80.0f, 130.0f, 215.0f, 40.0f,
                                                  2.0f, dialogueBorderColour,
                                                  dialogueBoxColour, DIALOGUE));
    grid.addChild(std::make_shared<DialoguePanel>(80.0f, 10.0f, 215.0f, 40.0f,
                                                  2.0f, dialogueBorderColour,
                                                  dialogueBoxColour, RESPONSE));
    grid.addChild(std::make_shared<DialogueText>(85.0f, 132.0f, 210.0f, 35.0f,
                                                 14.0f, fontColour, DIALOGUE));
    grid.addChild(std::make_shared<DialogueText>(85.0f, 12.0f, 215.0f, 35.0f,
                                                 14.0f, fontColour, RESPONSE));
  }
  ~UIManager() {}

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
