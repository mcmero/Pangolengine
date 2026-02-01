#pragma once

#include "Interactable.h"
#include "SDL3/SDL_events.h"
#include "Sprite.h"
#include "Transform.h"

class KeyboardController {
public:
  KeyboardController() = default;

  void update(SDL_Event *event, bool menuActive, Transform &transform,
              Sprite &sprite, Interactable *intObject = nullptr) {
    if (menuActive || (intObject != nullptr && intObject->active))
      transform.canMove = false;
    else
      transform.canMove = true;

    if (event->type == SDL_EVENT_KEY_UP) {
      switch (event->key.key) {
      case SDLK_E:
        if (intObject != nullptr) {
          intObject->interact();
          transform.canMove = false;
        }
        break;
      default:
        break;
      }
    }
  }

  /**
   * Handle input by polling key state for smooth movement animation
   */
  void pollInput(const bool *keyState, Transform &transform, Sprite &sprite) {
    bool moving = false;

    if (!transform.isMoving && transform.canMove) {

      if (keyState[SDL_SCANCODE_W]) {
        sprite.play("walk_back");
        transform.initiateMove(Direction::Up);
      } else if (keyState[SDL_SCANCODE_A]) {
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        transform.initiateMove(Direction::Left);
      } else if (keyState[SDL_SCANCODE_D]) {
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_NONE;
        transform.initiateMove(Direction::Right);
      } else if (keyState[SDL_SCANCODE_S]) {
        sprite.play("walk_front");
        transform.initiateMove(Direction::Down);
      }

    } else {
      switch (transform.lastDirection) {
      case Direction::Up:
        sprite.play("walk_back");
        break;
      case Direction::Down:
        sprite.play("walk_front");
        break;
      case Direction::Left:
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        break;
      case Direction::Right:
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_NONE;
        break;
      default:
        break;
      }
      moving = true;
    }

    if (!moving) {
      sprite.stop();
    }
  }
};
