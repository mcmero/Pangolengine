#pragma once

#include "SDL3/SDL_rect.h"

struct Size {
  float width = 0;
  float height = 0;
};

enum class Align { Center, Left, Right, Top, Bottom, None };

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

  static void alignRelativeToContainer(SDL_FRect &rect,
                                       const SDL_FRect &containerRect,
                                       Align horizontal, Align vertical) {
    // Handle horizontal alignment
    if (rect.w <= containerRect.w) {
      switch (horizontal) {
      case Align::Center:
        rect.x = containerRect.x + ((containerRect.w / 2) - (rect.w / 2));
        break;
      case Align::Left:
        rect.x = containerRect.x;
        break;
      case Align::Right:
        rect.x = containerRect.x + containerRect.w - rect.w;
        break;
      default:
        // No alignment for Top, Bottom or None
        break;
      }
    }

    // Handle vertical alignment
    if (rect.w <= containerRect.w) {
      switch (vertical) {
      case Align::Center:
        rect.y = containerRect.y + ((containerRect.h / 2) - (rect.h / 2));
        break;
      case Align::Top:
        rect.y = containerRect.y;
        break;
      case Align::Bottom:
        rect.y = containerRect.y + containerRect.h - rect.h;
        break;
      default:
        // No alignment for Left, Right or None
        break;
      }
    }
  }
};
