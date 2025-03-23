#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <ostream>
#include <sstream>

class DialogueText : public IComponent {
public:
  DialogueText(float xpos, float ypos, float width, float height,
               float pointsize, SDL_Color fontColour, ComponentType compType)
      : pointsize(pointsize), fontColour(fontColour),
        rect{xpos, ypos, width, height}, compType(compType) {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      // TODO: we will need to have access to the array of responses here,
      // because selected lines will be a different colour, so these will
      // have to be different objects drawn by the TextureManager
      // perhaps one response = one call to Text
      const std::string_view text = dialogueLine;
      TextureManager::Text(text, pointsize, rect.x, rect.y,
                           static_cast<int>(rect.w), fontColour);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active) {
      show = true;

      // TODO: we can handle dialogue selection and events here
      if (compType == DIALOGUE) {
        if (!dialogue->active) {
          dialogue->active = true;
          dialogue->beginDialogue();
        }
        dialogueLine = dialogue->getLine();
      } else if (compType == RESPONSE) {
        if (dialogue->active) {
          std::vector<Response> responses = dialogue->getResponses();
          std::stringstream ss;
          for (int idx = 0; idx < responses.size(); idx++) {
            ss << idx + 1 << ". " << responses[idx].response << std::endl;
          }
          dialogueLine = ss.str();
        }
      }
    } else {
      show = false;
    }
  }

private:
  bool show = false;
  float pointsize;
  SDL_Color fontColour;
  SDL_FRect rect;
  std::string dialogueLine;
  ComponentType compType;
};
