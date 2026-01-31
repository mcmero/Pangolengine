#pragma once

#include "../Constants.h"
#include "Interactable.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "Sprite.h"
#include "Transform.h"
#include "../Camera.h"

class MouseController {
public:
  MouseController() = default;

  void update(SDL_Event *event, bool menuActive, Transform &transform,
              Sprite &sprite, Interactable *intObject = nullptr) {
    if (menuActive || (intObject != nullptr && intObject->active))
      canMove = false;
    else
      canMove = true;

    // Interact with entity if user clicks on them
  }

  /**
   * Handle input by polling mouse state for smooth movement animation
   */
  void pollInput(const SDL_MouseButtonFlags flags, const float &xpos,
                 const float &ypos, Transform &transform, Sprite &sprite,
                 SDL_Renderer *renderer) {
    bool moving = false;
  

    if (!transform.isMoving && canMove && flags == SDL_BUTTON_LEFT) {
      std::cout << "Clicked mouse at " << xpos << "," << ypos << std::endl;
      std::cout << "Player pos at " << transform.position.x << "," << transform.position.y << std::endl;

      // Need to adjust by the render scale
      float scaleX, scaleY;
      SDL_GetRenderScale(renderer, &scaleX, &scaleY);

      // we need to adjust by 1. the scale, 2. the sprite offset and 3. sprite dimensions
      // this will make any click relative to the sprite center
      float xposAdj = (xpos / scaleX) + Camera::position.x - sprite.posOffset.x - (sprite.width / 2);
      float yposAdj = (ypos / scaleY) + Camera::position.y - sprite.posOffset.y - (sprite.height / 2);
      std::cout << "Adjusted pos at " << xposAdj << "," << yposAdj << std::endl;

      // Get player position on the grid
      float xposGridPlayer = round(transform.position.x / TILE_SIZE);
      float yposGridPlayer = round(transform.position.y / TILE_SIZE);
      std::cout << "Plaer at pos at " << xposGridPlayer << "," << yposGridPlayer << std::endl;

      // Get click grid position
      float xposGridClick = round(xposAdj / TILE_SIZE);
      float yposGridClick = round(yposAdj / TILE_SIZE);
      std::cout << "Clicked grid pos at " << xposGridClick << "," << yposGridClick << std::endl;

      if (xposGridClick > xposGridPlayer) {
        sprite.play("walk_right");

        setPlayerMovement(RIGHT, transform);
        lastDirection = RIGHT;
      } else if (xposGridClick < xposGridPlayer) {
        sprite.play("walk_left");

        setPlayerMovement(LEFT, transform);
        lastDirection = LEFT;
      } else if (yposGridClick > yposGridPlayer) {
        sprite.play("walk_down");

        setPlayerMovement(DOWN, transform);
        lastDirection = DOWN;
      } else if (yposGridClick < yposGridPlayer) {
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
