#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
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
      show = true;
    } else {
      show = false;
    }
  }

private:
  bool show = false;
  float pointsize;
  SDL_Color fontColour;
  SDL_Color selectColour;
  SDL_FRect rect;
  std::vector<Response> responses;
  int selectedResponse = 0;
  const float lineSpacing = 15.0f;
};
