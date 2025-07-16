#pragma once

#include "../Constants.h"
#include "Interactable.h"
#include "SDL3/SDL_events.h"
#include "Sprite.h"
#include "Transform.h"

class KeyboardController {
public:
  KeyboardController() = default;

  void update(SDL_Event *event, bool menuActive, Transform &transform,
              Sprite &sprite, Interactable *intObject = nullptr) {
    if (!menuActive && intObject != nullptr && intObject->active)
      canMove = false;
    else
      canMove = true;

    if (event->type == SDL_EVENT_KEY_UP) {
      switch (event->key.key) {
      case SDLK_E:
        if (intObject != nullptr) {
          intObject->interact();
          canMove = false;
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

    if (!transform.isMoving && canMove) {

      if (keyState[SDL_SCANCODE_W]) {
        sprite.play("walk_back");

        setPlayerMovement(UP, transform);
        lastDirection = UP;

      } else if (keyState[SDL_SCANCODE_A]) {
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;

        setPlayerMovement(LEFT, transform);
        lastDirection = LEFT;

      } else if (keyState[SDL_SCANCODE_D]) {
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_NONE;

        setPlayerMovement(RIGHT, transform);
        lastDirection = RIGHT;

      } else if (keyState[SDL_SCANCODE_S]) {
        sprite.play("walk_front");

        setPlayerMovement(DOWN, transform);
        lastDirection = DOWN;
      }

    } else {
      switch (lastDirection) {
      case UP:
        sprite.play("walk_back");
        break;
      case DOWN:
        sprite.play("walk_front");
        break;
      case LEFT:
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        break;
      case RIGHT:
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

private:
  enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
  Direction lastDirection = NONE;
  bool canMove = true;

  void setPlayerMovement(Direction dir, Transform &transform) {
    // Set player movement vector if the direction is the same
    // as the current direction, otherwise do not modify vector
    if (lastDirection == dir && !transform.isMoving) {
      switch (dir) {
      case UP:
        transform.initiateMove(Vector2D(0, -1.0f * TILE_SIZE));
        break;
      case DOWN:
        transform.initiateMove(Vector2D(0, TILE_SIZE));
        break;
      case LEFT:
        transform.initiateMove(Vector2D(-1.0f * TILE_SIZE, 0));
        break;
      case RIGHT:
        transform.initiateMove(Vector2D(TILE_SIZE, 0));
        break;
      default:
        break;
      }
    }
  }
};
