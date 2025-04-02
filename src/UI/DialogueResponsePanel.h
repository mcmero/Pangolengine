#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <cassert>
#include <ostream>
#include <sstream>
#include <string_view>

class DialogueResponsePanel : public IComponent {
public:
  DialogueResponsePanel(float xpos, float ypos, float width, float height,
                        float borderThickness, SDL_Color borderColour,
                        SDL_Color innerColour, float pointsize,
                        SDL_Color fontColour, SDL_Color selectColour)
      : borderRect{xpos - borderThickness, ypos - borderThickness,
                   width + 2 * borderThickness, height + 2 * borderThickness},
        innerRect{xpos, ypos, width, height}, borderColour(borderColour),
        innerColour(innerColour),
        textRect(xpos + 5.0f, ypos + 2.0f, width - 5.0f, height - 5.0f),
        fontColour(fontColour), selectColour(selectColour),
        pointsize(pointsize) {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      TextureManager::Panel(borderRect, innerRect, borderColour, innerColour);
      SDL_Color textColour = fontColour;
      for (int idx = 0; idx < responses.size(); idx++) {
        if (idx == selectedResponse)
          textColour = selectColour;
        else
          textColour = fontColour;
        std::stringstream ss;
        ss << idx + 1 << ". " << responses[idx].response << std::endl;
        std::string line = ss.str();
        TextureManager::Text(static_cast<std::string_view>(line), pointsize,
                             textRect.x, textRect.y + (lineSpacing * idx),
                             static_cast<int>(textRect.w), textColour);
      }
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active &&
        dialogue != nullptr && dialogue->active) {
      responses = dialogue->getResponses();
      if (responses.empty()) {
        std::cout << "No respones!" << std::endl;
        show = false;
      } else {
        show = true;
      }
      handleDialogueSelect(event, dialogue, interactable);
    } else {
      show = false;
    }
  }

private:
  bool show = false;
  const float lineSpacing = 15.0f;

  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;

  SDL_FRect textRect;
  SDL_Color fontColour;
  SDL_Color selectColour;
  float pointsize;

  std::vector<Response> responses;
  int selectedResponse = 0;
  int nextNodeId = 0;

  void handleDialogueSelect(const SDL_Event &event, Dialogue *dialogue,
                            Interactable *interactable) {
    assert(dialogue != nullptr && "Cannot handle dialogue with no dialogue "
                                  "object!");
    // TODO: need to make sure you can't select dialogue line if there's
    // no responses displayed
    bool continueDialogue = true;
    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_DOWN:
        if ((selectedResponse + 1) >= responses.size())
          selectedResponse = 0;
        else
          selectedResponse++;
        break;
      case SDLK_UP:
        if ((selectedResponse - 1) < 0)
          selectedResponse = responses.size() - 1;
        else
          selectedResponse--;
        break;
      case SDLK_RETURN:
        nextNodeId = getNextNode();
        continueDialogue = dialogue->progressToNode(nextNodeId);
        if (!continueDialogue)
          interactable->active = false;
        break;
      default:
        break;
      }
    }
  }

  int getNextNode() {
    for (int idx = 0; idx < responses.size(); idx++) {
      if (idx == selectedResponse)
        return responses[idx].next;
    }
    return 0;
  }
};
