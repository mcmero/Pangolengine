#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "PanelHelper.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <cassert>
#include <ostream>
#include <sstream>

class DialogueResponsePanel : public IComponent {
public:
  DialogueResponsePanel(float xpos, float ypos, float width, float height,
                        float borderThickness, SDL_Color borderColour,
                        SDL_Color innerColour, float pointsize,
                        SDL_Color fontColour, SDL_Color selectColour)
      : borderRect(PanelHelper::getBorderRect(xpos, ypos, width, height,
                                              borderThickness)),
        innerRect(PanelHelper::getInnerRect(xpos, ypos, width, height)),
        borderColour(borderColour), innerColour(innerColour),
        textRect(PanelHelper::getTextRect(xpos, ypos, width, height)),
        fontColour(fontColour), selectColour(selectColour),
        pointsize(pointsize) {}

  ~DialogueResponsePanel() { clean(); }

  void render(SDL_Renderer *renderer) override {
    int yOffset = -scrollOffset;
    if (show) {
      TextureManager::Panel(borderRect, innerRect, borderColour, innerColour);

      for (int idx = 0; idx < responseTextures.size(); idx++) {

        ResponseTexture &curLine = responseTextures[idx];
        curLine.displayed = false;

        SDL_Texture *tex = nullptr;
        if (idx == selectedResponse)
          tex = curLine.activeTex;
        else
          tex = curLine.inactiveTex;

        SDL_FRect dest = {textRect.x, textRect.y + yOffset, curLine.width,
                          curLine.height};

        if (yOffset + textRect.y < textRect.y) {
          // Skip lines above view area
          yOffset += curLine.height;
          continue;
        }

        if (yOffset + textRect.y + curLine.height > textRect.y + textRect.h) {
          // Stop rendering if lines below view area
          break;
        }

        curLine.displayed = true;
        SDL_RenderTexture(renderer, tex, NULL, &dest);

        yOffset += curLine.height;
      }
    }
  }

  void update(const SDL_Event &event, Interactable *interactable,
              Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active &&
        dialogue != nullptr && dialogue->active) {
      if (responses.empty()) {
        // check if there are new responses
        // we should only do this at the start of a new dialogue
        responses = dialogue->getResponses();
        if (responses.empty()) {
          std::cout << "No respones!" << std::endl;
          show = false;
        } else {
          loadResponseTextures();
          show = true;
        }
      } else {
        show = true;
      }
      handleDialogueSelect(event, dialogue, interactable);
    } else {
      show = false;
    }
  }

  void clean() override { responseTextures.clear(); }

private:
  bool show = false;

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

  struct ResponseTexture {
    SDL_Texture *activeTex;
    SDL_Texture *inactiveTex;
    bool displayed;
    float height;
    float width;
    int idx;
  };
  std::vector<ResponseTexture> responseTextures;

  // variables for scroll offsetting
  float scrollOffset;
  enum Direction { UP, DOWN };

  void handleDialogueSelect(const SDL_Event &event, Dialogue *dialogue,
                            Interactable *interactable) {
    if (dialogue == nullptr) {
      std::cout << "No dialogue to select!" << std::endl;
      return;
    } else if (!dialogue->active) {
      std::cout << "Dialogue not active!" << std::endl;
      return;
    } else if (interactable == nullptr) {
      std::cout << "No interactable object!" << std::endl;
      return;
    } else if (!interactable->active) {
      std::cout << "Interactable object not active!" << std::endl;
      return;
    } else if (selectedResponse < 0) {
      std::cout << "No valid selected respoes!" << std::endl;
      return;
    }

    bool continueDialogue = true;
    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_DOWN:
        if ((selectedResponse + 1) >= responses.size())
          selectedResponse = 0;
        else
          selectedResponse++;
        calculateScrollOffset(DOWN);
        break;
      case SDLK_UP:
        if ((selectedResponse - 1) < 0)
          selectedResponse = responses.size() - 1;
        else
          selectedResponse--;
        calculateScrollOffset(UP);
        break;
      case SDLK_RETURN:
        if (responses.size() > 0) {
          nextNodeId = getNextNode();
          continueDialogue = dialogue->progressToNode(nextNodeId);
          if (!continueDialogue)
            interactable->active = false;
          selectedResponse = 0; // Reset selected response
          responses = dialogue->getResponses();
          loadResponseTextures();
        } else {
          // We've run out of responses, end the dialogue
          dialogue->active = false;
          interactable->active = false;
          clean();
        }
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

  void loadResponseTextures() {
    responseTextures.clear();

    for (int idx = 0; idx < responses.size(); idx++) {
      ResponseTexture rtex = {};

      std::string line = responses[idx].line;

      std::stringstream ss;
      ss << idx + 1 << ". " << line << std::endl;
      std::string message = ss.str();

      rtex.activeTex = TextureManager::LoadMessageTexture(
          static_cast<std::string_view>(message), pointsize,
          static_cast<int>(textRect.w), selectColour);

      rtex.inactiveTex = TextureManager::LoadMessageTexture(
          static_cast<std::string_view>(message), pointsize,
          static_cast<int>(textRect.w), fontColour);

      auto messageDims =
          TextureManager::GetMessageTextureDimensions(rtex.activeTex);

      rtex.height = messageDims.height;
      rtex.width = messageDims.width;
      rtex.idx = idx;
      rtex.displayed = false;

      responseTextures.push_back(rtex);
    }
  }

  void calculateScrollOffset(Direction dir) {
    // We can reset the scroll offset if we are at the first response
    if (selectedResponse == 0)
      scrollOffset = 0;

    if (!responseTextures[selectedResponse].displayed) {
      // This means that the message is *not* being displayed but is selected
      for (int idx = 0; idx < selectedResponse; idx++) {
        scrollOffset += responseTextures[selectedResponse].height;
      }
    }
  }
};
