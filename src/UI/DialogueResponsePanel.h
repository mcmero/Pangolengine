#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "UIHelper.h"
#include <cassert>
#include <ostream>
#include <sstream>

class DialogueResponsePanel : public IComponent {
public:
  DialogueResponsePanel(float xpos, float ypos, float width, float height,
                        float borderThickness, SDL_Color borderColour,
                        SDL_Color innerColour, float pointsize,
                        SDL_Color fontColour, SDL_Color selectColour)
      : borderRect(UIHelper::getBorderRect(xpos, ypos, width, height,
                                           borderThickness)),
        innerRect(UIHelper::getInnerRect(xpos, ypos, width, height)),
        borderColour(borderColour), innerColour(innerColour),
        textRect(UIHelper::getTextRect(xpos, ypos, width, height)),
        fontColour(fontColour), selectColour(selectColour),
        pointsize(pointsize) {}

  ~DialogueResponsePanel() { clean(); }

  void render(SDL_Renderer *renderer, SDL_Window *window) override {
    float yOffset = -scrollOffset;
    if (show) {
      TextureManager::DrawPanel(borderRect, innerRect, borderColour,
                                innerColour);

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

  void update(Interactable *interactable, Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active &&
        dialogue != nullptr && dialogue->active && dialogue->canRespond) {

      if (state == INACTIVE) {
        // Reset vars to start new dialogue
        state = ACTIVE;
        selectedResponse = 0;
        nextNodeId = 0;
        scrollOffset = 0;

        responses = dialogue->getResponses();
        clean(); // Clean before loading new textures
        loadResponseTextures();
      } else if (state == PROGRESS) {
        // Fetch the next node and determine if dialogue progresses
        nextNodeId = getNextNode();
        bool continueDialogue = dialogue->progressToNode(nextNodeId);
        if (!continueDialogue)
          interactable->active = false;

        // Get responses
        responses = dialogue->getResponses();
        clean(); // Clean before loading new textures
        loadResponseTextures();

        // Reset selected response
        selectedResponse = 0;

        // State progreses to active
        state = ACTIVE;
      }
      if (state == ACTIVE) {
        nextNodeId = getNextNode();

        if (responses.empty())
          show = false;
        else
          show = true;

      } else if (state == END) {
        show = false;
        interactable->active = false;
        dialogue->active = false;
        state = INACTIVE;
        clean();
      }
    } else {
      show = false;
    }
  }

  void handleEvents(const SDL_Event &event) override {
    // Dialogue selection events
    if (state != INACTIVE && event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_DOWN:
        if ((selectedResponse + 1) >= static_cast<int>(responses.size()))
          selectedResponse = 0;
        else
          selectedResponse++;
        setScrollOffset(DOWN);
        break;
      case SDLK_UP:
        if ((selectedResponse - 1) < 0)
          selectedResponse = static_cast<int>(responses.size()) - 1;
        else
          selectedResponse--;
        setScrollOffset(UP);
        break;
      case SDLK_RETURN:
        if (responses.size() > 0 && getNextNode() > 0) {
          state = PROGRESS;
        } else {
          // We've run out of responses, end the dialogue
          state = END;
        }
        break;
      default:
        break;
      }
    }
  }

  void clean() override {
    for (auto &responseTexture : responseTextures) {
      if (responseTexture.activeTex) {
        SDL_DestroyTexture(responseTexture.activeTex);
        responseTexture.activeTex = nullptr;
      }
      if (responseTexture.inactiveTex) {
        SDL_DestroyTexture(responseTexture.inactiveTex);
        responseTexture.inactiveTex = nullptr;
      }
    }
    responseTextures.clear();
  }

private:
  bool show = false;

  enum DialogueState { INACTIVE, ACTIVE, PROGRESS, END };
  DialogueState state = INACTIVE;

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

  void setScrollOffset(Direction dir) {
    // We can reset the scroll offset if we are at the first or last response
    if (selectedResponse == 0 || selectedResponse == responses.size() - 1) {
      scrollOffset = 0;
      dir = DOWN; // change the direction to down as we now need to add the
                  // offset (not subtract)
    }

    if (!responseTextures[selectedResponse].displayed) {
      // We need to scroll as the message is selected but not displayed
      if (dir == DOWN) {
        for (int idx = 0; idx < selectedResponse; idx++) {
          scrollOffset += responseTextures[selectedResponse].height;
        }
      } else {
        for (int idx = static_cast<int>(responseTextures.size()) - 1;
             idx > selectedResponse; idx--) {
          scrollOffset -= responseTextures[selectedResponse].height;
        }
      }
    }
  }
};
