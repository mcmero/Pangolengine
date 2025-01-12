#pragma once

#include "SDL3/SDL_events.h"
#include "Transform.h"

class KeyboardController {
public:
  KeyboardController() = default;

  void update(SDL_Event *event, Transform &transform) {
    if (event->type == SDL_EVENT_KEY_DOWN) {
      switch (event->key.key) {
      case SDLK_W:
        transform.position.y -= 1;
        break;
      case SDLK_A:
        transform.position.x -= 1;
        break;
      case SDLK_D:
        transform.position.x += 1;
        break;
      case SDLK_S:
        transform.position.y += 1;
        break;
      default:
        break;
      }
    }
  }
  int dummy; // prevent void error for now
};
