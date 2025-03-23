#pragma once

#include "../Constants.h"
#include "Interactable.h"
#include "SDL3/SDL_events.h"
#include "Sprite.h"
#include "Transform.h"

class KeyboardController {
public:
  KeyboardController() = default;

  void update(SDL_Event *event, Transform &transform, Sprite &sprite,
              Interactable *intObject = nullptr) {
    if (intObject != nullptr && intObject->active)
      canMove = false;
    else
      canMove = true;

    if (event->type == SDL_EVENT_KEY_DOWN && canMove) {
      switch (event->key.key) {
      case SDLK_W:
        sprite.play("walk_back");
        setPlayerMovement(UP, transform);
        lastDirection = UP;
        break;
      case SDLK_A:
        sprite.play("walk_side");
        setPlayerMovement(LEFT, transform);
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        lastDirection = LEFT;
        break;
      case SDLK_D:
        sprite.play("walk_side");
        setPlayerMovement(RIGHT, transform);
        sprite.spriteFlip = SDL_FLIP_NONE;
        lastDirection = RIGHT;
        break;
      case SDLK_S:
        sprite.play("walk_front");
        setPlayerMovement(DOWN, transform);
        lastDirection = DOWN;
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
