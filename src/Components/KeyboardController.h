#pragma once

#include "../Constants.h"
#include "SDL3/SDL_events.h"
#include "Sprite.h"
#include "Transform.h"

class KeyboardController {
public:
  KeyboardController() = default;

  void update(SDL_Event *event, Transform &transform, Sprite &sprite) {
    if (event->type == SDL_EVENT_KEY_DOWN) {
      switch (event->key.key) {
      case SDLK_W:
        sprite.play("walk_back");
        transform.position.y -= PLAYER_SPEED;
        break;
      case SDLK_A:
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        transform.position.x -= PLAYER_SPEED;
        break;
      case SDLK_D:
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_NONE;
        transform.position.x += PLAYER_SPEED;
        break;
      case SDLK_S:
        sprite.play("walk_front");
        transform.position.y += PLAYER_SPEED;
        break;
      default:
        break;
      }
    }
    if (event->type == SDL_EVENT_KEY_UP) {
      switch (event->key.key) {
      case SDLK_W:
        sprite.stop();
        break;
      case SDLK_A:
        sprite.stop();
        break;
      case SDLK_D:
        sprite.stop();
        break;
      case SDLK_S:
        sprite.stop();
        break;
      default:
        break;
      }
    }
  }
  int dummy; // prevent void error for now
};
