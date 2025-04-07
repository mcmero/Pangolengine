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
        SDL_Texture *messageTex = TextureManager::GetMessageTexture(
            msgTextures, textRect, line, pointsize, textColour);

        auto messageDims =
            TextureManager::GetMessageTextureDimensions(messageTex);
        SDL_FRect dest = {textRect.x, textRect.y + (lineSpacing * idx),
                          std::get<0>(messageDims), std::get<1>(messageDims)};

        SDL_RenderTexture(renderer, messageTex, NULL, &dest);
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

  void clean() override {
    for (auto &pair : msgTextures) {
      SDL_DestroyTexture(pair.second.tex);
    }
    msgTextures.clear();
  }

  ~DialogueResponsePanel() { clean(); }

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

  std::unordered_map<std::string, MessageTexture> msgTextures;

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
        break;
      case SDLK_UP:
        if ((selectedResponse - 1) < 0)
          selectedResponse = responses.size() - 1;
        else
          selectedResponse--;
        break;
      case SDLK_RETURN:
        if (responses.size() > 0) {
          nextNodeId = getNextNode();
          continueDialogue = dialogue->progressToNode(nextNodeId);
          if (!continueDialogue)
            interactable->active = false;
          selectedResponse = 0; // Reset selected response
        } else {
          // We've run out of responses, end the dialogue
          dialogue->active = false;
          interactable->active = false;
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
  /*
    SDL_Texture *getTextTexture(const std::string &text, SDL_Color colour) {
      auto it = msgTextures.find(text);
      if (it != msgTextures.end()) {
        // Return the texture if it has the same colour values
        if (it->second.colour.a == colour.a && it->second.colour.b == colour.b
    && it->second.colour.g == colour.g && it->second.colour.r == colour.r) {
          return it->second.tex;
        } else {
          // We have to destroy the texture as it has changed colour
          // and let it be recreated
          SDL_DestroyTexture(it->second.tex);
        }
      }
      SDL_Texture *texture = TextureManager::LoadMessageTexture(
          static_cast<std::string_view>(text), pointsize, textRect.x,
    textRect.y, static_cast<int>(textRect.w), colour); msgTextures[text].tex =
    texture; msgTextures[text].colour = colour; return texture;
    }*/
};
