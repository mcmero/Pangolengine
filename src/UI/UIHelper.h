#pragma once

#include "../TextureManager.h"
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

  // TODO: rename message dims to something more general
  static SDL_FRect getCenteredRectTop(SDL_FRect sourceRect,
                                      MessageDims messageDims) {
    if (messageDims.width >= sourceRect.w)
      return sourceRect;

    // Get center x coordinate
    float x = sourceRect.x + ((sourceRect.w / 2) - (messageDims.width / 2));

    return SDL_FRect(x, sourceRect.y, messageDims.width, messageDims.height);
  }
};
