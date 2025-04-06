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
#include <string_view>

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
        // TextureManager::Text(static_cast<std::string_view>(line), pointsize,
        //                      textRect.x, textRect.y + (lineSpacing * idx),
        //                      static_cast<int>(textRect.w), textColour);
        SDL_Texture *messageTex = getTextTexture(line, textColour);

        // Get on-screen dimensions of the text, necessary for rendering
        auto texprops = SDL_GetTextureProperties(messageTex);
        SDL_FRect dest = {textRect.x, textRect.y + (lineSpacing * idx),
                          float(SDL_GetNumberProperty(
                              texprops, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)),
                          float(SDL_GetNumberProperty(
                              texprops, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0))};

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

  ~DialogueResponsePanel() { clearTextures(); }

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

  struct textTexture {
    SDL_Texture *tex;
    SDL_Color colour;
  };

  std::unordered_map<std::string, textTexture> textTextures;

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

  SDL_Texture *getTextTexture(const std::string &text, SDL_Color colour) {
    auto it = textTextures.find(text);
    if (it != textTextures.end()) {
      // Return the texture if it has the same colour values
      if (it->second.colour.a == colour.a && it->second.colour.b == colour.b &&
          it->second.colour.g == colour.g && it->second.colour.r == colour.r) {
        return it->second.tex;
      } else {
        // We have to destroy the texture as it has changed colour
        // and let it be recreated
        SDL_DestroyTexture(it->second.tex);
      }
    }
    SDL_Texture *texture = TextureManager::LoadMessageTexture(
        static_cast<std::string_view>(text), pointsize, textRect.x, textRect.y,
        static_cast<int>(textRect.w), colour);
    textTextures[text].tex = texture;
    textTextures[text].colour = colour;
    return texture;
  }

  void clearTextures() {
    for (auto &pair : textTextures) {
      SDL_DestroyTexture(pair.second.tex);
    }
    textTextures.clear();
  }
};
