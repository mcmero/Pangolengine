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

class DialogueResponseText : public IComponent {
public:
  DialogueResponseText(float xpos, float ypos, float width, float height,
                       float pointsize, SDL_Color fontColour,
                       SDL_Color selectColour)
      : pointsize(pointsize), fontColour(fontColour),
        selectColour(selectColour), rect{xpos, ypos, width, height} {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
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
                             rect.x, rect.y + (lineSpacing * idx),
                             static_cast<int>(rect.w), textColour);
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
  float pointsize;
  const float lineSpacing = 15.0f;

  SDL_Color fontColour;
  SDL_Color selectColour;
  SDL_FRect rect;

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
