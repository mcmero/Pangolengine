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
        setPlayerMovement(UP, transform.position);
        lastDirection = UP;
        break;
      case SDLK_A:
        sprite.play("walk_side");
        setPlayerMovement(LEFT, transform.position);
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        lastDirection = LEFT;
        break;
      case SDLK_D:
        sprite.play("walk_side");
        setPlayerMovement(RIGHT, transform.position);
        sprite.spriteFlip = SDL_FLIP_NONE;
        lastDirection = RIGHT;
        break;
      case SDLK_S:
        sprite.play("walk_front");
        setPlayerMovement(DOWN, transform.position);
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
      default:
        break;
      }
    }
  }

private:
  enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
  Direction lastDirection = NONE;

  void setPlayerMovement(Direction dir, Vector2D &tfVec) {
    // Set player movement vector if the direction is the same
    // as the current direction, otherwise do not modify vector
    if (lastDirection == dir) {
      switch (dir) {
      case UP:
        tfVec.Subtract(Vector2D(0, PLAYER_SPEED));
        break;
      case DOWN:
        tfVec.Add(Vector2D(0, PLAYER_SPEED));
        break;
      case LEFT:
        tfVec.Subtract(Vector2D(PLAYER_SPEED, 0));
        break;
      case RIGHT:
        tfVec.Add(Vector2D(PLAYER_SPEED, 0));
        break;
      default:
        break;
      }
    }
  }
};
