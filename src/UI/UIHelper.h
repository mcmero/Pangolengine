#pragma once

#include "SDL3/SDL_rect.h"

class UIHelper {
public:
  static SDL_FRect getBorderRect(float xpos, float ypos, float width,
                                 float height, float borderThickness) {
    return SDL_FRect{xpos - borderThickness, ypos - borderThickness,
                     width + 2 * borderThickness, height + 2 * borderThickness};
  }

  static SDL_FRect getInnerRect(float xpos, float ypos, float width,
                                float height) {
    return SDL_FRect{xpos, ypos, width, height};
  }

  static SDL_FRect getTextRect(float xpos, float ypos, float width,
                               float height) {
    return SDL_FRect{xpos + 5.0f, ypos + 2.0f, width - 5.0f, height - 5.0f};
  }

  static void centerRectRelativeToContainer(SDL_FRect &rect,
                                            const SDL_FRect &containerRect) {
    if (rect.w >= containerRect.w)
      return;

    // Center the x coordinate
    rect.x = containerRect.x + ((containerRect.w / 2) - (rect.w / 2));
    rect.y = containerRect.y;
  }
};
