#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "UIHelper.h"
#include <algorithm>
#include <sstream>
#include <string>

class DialoguePanel : public IComponent {
public:
  DialoguePanel(float xpos, float ypos, float width, float height,
                float borderThickness, SDL_Color borderColour,
                SDL_Color innerColour, float pointsize, SDL_Color fontColour)
      : borderRect(UIHelper::getBorderRect(xpos, ypos, width, height,
                                           borderThickness)),
        innerRect(UIHelper::getInnerRect(xpos, ypos, width, height)),
        borderColour(borderColour), innerColour(innerColour),
        textRect(UIHelper::getTextRect(xpos, ypos, width, height)),
        fontColour(fontColour), pointsize(pointsize) {
    fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
    dialogueSound = Mix_LoadWAV(
        (assetsPath / "audio" / "dialogue_blip.ogg").string().c_str());
  }

  void render(SDL_Renderer *renderer, SDL_Window *window) override {
    if (show) {
      TextureManager::DrawPanel(borderRect, innerRect, borderColour,
                                innerColour);

      // Keep seoffset within bounds so that we keep text visible
      if (!finishedWriting) {
        // Set max scroll offset if we haven't finished writing
        scrollOffset = messageDims.height - textRect.h;
      } else {
        scrollOffset = std::max(scrollOffset, 0.0f);
        scrollOffset = std::min(scrollOffset, messageDims.height - textRect.h);
      }

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
    }
  }

  void update(Interactable *interactable, Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active) {
      show = true;

      // Begin dialogue if interact object is active
      if (!dialogue->active) {
        dialogue->active = true;
        dialogue->beginDialogue();
        std::cout << "Begin dialogue" << std::endl;
      }

      // Check if the line has changed
      if (dialogue->getLine() != line) {
        // Pause ability to respond until we finish writing
        dialogue->canRespond = false;

        // Recreate message
        std::stringstream ss;
        ss << dialogue->getSpeaker() << ": " << dialogue->getLine()
           << std::endl;
        message = ss.str();

        // Update line
        line = dialogue->getLine();

        // Reset scrolling
        scrollOffset = 0;

        // Reset var for typewriter effect
        messageIdx = 0;
        finishedWriting = false;
      }

      if (!finishedWriting) {
        // Display only characters we are up to for typewriter effect
        messageIdx++;
        std::string currMessage = message.substr(0, messageIdx);

        // Play dialogue sound every fifth character
        if (currMessage.size() % 5 == 0)
          Mix_PlayChannel(-1, dialogueSound, 0);

        // Recreate texture
        SDL_DestroyTexture(messageTex);
        messageTex = TextureManager::LoadMessageTexture(
            currMessage, pointsize, static_cast<int>(textRect.w), fontColour);
        messageDims = TextureManager::GetMessageTextureDimensions(messageTex);

        // Check if we've finished
        if (messageIdx == message.size()) {
          finishedWriting = true;
          dialogue->canRespond = true;
        }
      }
    } else
      show = false;
  }

  void handleEvents(const SDL_Event &event) override {
    // Handle panel scrolling
    if (show && finishedWriting && event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_S: // scroll down
        scrollOffset += scrollAmount;
        break;
      case SDLK_W: // scroll up
        scrollOffset -= scrollAmount;
        break;
      default:
        break;
      }
      std::cout << "Scroll offset: " << scrollOffset << std::endl;
    }
  }

  void clean() override { SDL_DestroyTexture(messageTex); }

private:
  bool show = false;

  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;

  SDL_FRect textRect;
  SDL_Color fontColour;
  float pointsize;

  SDL_Texture *messageTex;
  Size messageDims;
  std::string line;    // current line
  std::string message; // message to print

  float scrollOffset = 0.0f;
  float const scrollAmount = 5.0f;

  // for typewriter effect
  int messageIdx = 0; // which letter of the message we are up to
  bool finishedWriting = false;

  // for dialogue sounds
  Mix_Chunk *dialogueSound;
};
