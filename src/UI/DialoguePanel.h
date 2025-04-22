#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "PanelHelper.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <algorithm>
#include <sstream>

class DialoguePanel : public IComponent {
public:
  DialoguePanel(float xpos, float ypos, float width, float height,
                float borderThickness, SDL_Color borderColour,
                SDL_Color innerColour, float pointsize, SDL_Color fontColour)
      : borderRect(PanelHelper::getBorderRect(xpos, ypos, width, height,
                                              borderThickness)),
        innerRect(PanelHelper::getInnerRect(xpos, ypos, width, height)),
        borderColour(borderColour), innerColour(innerColour),
        textRect(PanelHelper::getTextRect(xpos, ypos, width, height)),
        fontColour(fontColour), pointsize(pointsize) {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      TextureManager::Panel(borderRect, innerRect, borderColour, innerColour);
      SDL_Texture *messageTex = TextureManager::LoadMessageTexture(
          dialogueLine, pointsize, static_cast<int>(textRect.w), fontColour);

      auto messageDims =
          TextureManager::GetMessageTextureDimensions(messageTex);

      // Keep scroll offset within bounds so that we keep text visible
      scrollOffset = std::max(scrollOffset, 0.0f);
      scrollOffset = std::min(scrollOffset, messageDims.height - textRect.h);

      // source rect used to clip the text based on offset
      SDL_FRect dest;
      SDL_FRect src;

      if (messageDims.height > textRect.h) {
        // scrolling is needed in this case
        src = {0, scrollOffset, textRect.w, textRect.h};
        dest = {textRect.x, textRect.y, textRect.w, textRect.h};
      } else {
        // message fits in window -- no scrolling
        src = {0, 0, textRect.w, textRect.h};
        dest = {textRect.x, textRect.y, messageDims.width, messageDims.height};
      }

      SDL_RenderTexture(renderer, messageTex, &src, &dest);
      SDL_DestroyTexture(messageTex); // Free texture
    }
  }

  void update(Interactable *interactable, Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active) {
      show = true;

      if (!dialogue->active) {
        dialogue->active = true;
        dialogue->beginDialogue();
        std::cout << "Begin dialogue" << std::endl;
      }

      std::stringstream ss;
      ss << dialogue->getSpeaker() << ": " << dialogue->getLine() << std::endl;
      dialogueLine = ss.str();

    } else
      show = false;
  }

  void handleEvents(const SDL_Event &event) override {

    // Handle panel scrolling
    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_S: // scroll down
        scrollOffset += 5;
        break;
      case SDLK_W: // scroll up
        scrollOffset -= 5;
        break;
      default:
        break;
      }
      std::cout << "Scroll offset: " << scrollOffset << std::endl;
    }
  }

private:
  bool show = false;

  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;

  SDL_FRect textRect;
  SDL_Color fontColour;
  float pointsize;
  std::string dialogueLine;

  float scrollOffset = 0;
};
