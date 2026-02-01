#pragma once

#include "../Constants.h"
#include "Interactable.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "Sprite.h"
#include "Transform.h"
#include "../Camera.h"
#include "Vector2D.h"

class MouseController {
public:
  MouseController() = default;

  void update(SDL_Event *event, bool menuActive, Transform &transform,
              Sprite &sprite, Interactable *intObject = nullptr) {
    if (menuActive || (intObject != nullptr && intObject->active))
      canMove = false;
    else
      canMove = true;

    // TODO: Interact with entity if user clicks on them
  }

  /**
   * Handle input by polling mouse state for smooth movement animation
   */
  void pollInput(const SDL_MouseButtonFlags flags, const float &xpos,
                 const float &ypos, Transform &transform, Sprite &sprite,
                 SDL_Renderer *renderer) {
    bool moving = false;
  
    if (!transform.isMoving && canMove && flags == SDL_BUTTON_LEFT) {
      // Need to adjust moues click pos by the render scale
      float scaleX, scaleY;
      SDL_GetRenderScale(renderer, &scaleX, &scaleY);
      float xposAdj = (xpos / scaleX);
      float yposAdj = (ypos / scaleY);

      // Adjust by camera and sprite offset/dimensions to get center pos of sprite
      xposAdj = xposAdj + Camera::position.x - sprite.posOffset.x - (sprite.width / 2);
      yposAdj = yposAdj + Camera::position.y - sprite.posOffset.y - (sprite.height / 2);

      // Create movement vector relative to player sprite, adjusted to tile grid
      Vector2D movement = {
        round(xposAdj / TILE_SIZE) - round(transform.position.x / TILE_SIZE),
        round(yposAdj / TILE_SIZE) - round(transform.position.y / TILE_SIZE)
      };

      if (movement.x > 0 && movement.x > movement.y) {
        sprite.play("walk_right");

        setPlayerMovement(RIGHT, transform);
        lastDirection = RIGHT;
      } else if (movement.x < 0 && movement.x < movement.y) {
        sprite.play("walk_left");

        setPlayerMovement(LEFT, transform);
        lastDirection = LEFT;
      } else if (movement.y > 0 && movement.y > movement.x) {
        sprite.play("walk_down");

        setPlayerMovement(DOWN, transform);
        lastDirection = DOWN;
      } else if (movement.y < 0 && movement.y < movement.x) {
        sprite.play("walk_up");

        setPlayerMovement(UP, transform);
        lastDirection = UP;
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

  // TODO: abstract this function out
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
