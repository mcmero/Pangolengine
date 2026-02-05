#pragma once

#include "../Constants.h"
#include "Collision.h"
#include "Interactable.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_render.h"
#include "Sprite.h"
#include "Transform.h"
#include "../Camera.h"
#include "Vector2D.h"

struct MouseInfo {
  SDL_MouseButtonFlags flags;
  float xpos;
  float ypos;
};

class MouseController {
public:
  MouseController() = default;

  void update(const MouseInfo mouseInfo, SDL_Renderer *renderer, bool menuActive,
              Transform &transform, Sprite &sprite,
              Interactable *intObject = nullptr) {
    if (menuActive || (intObject != nullptr && intObject->active))
      transform.canMove = false;
    else
      transform.canMove = true;

    // Interact with entity if the user has clicked on them
    Vector2D adjustedMouse = getAdjustedMousePos(
      mouseInfo.xpos, mouseInfo.ypos, renderer
    );
    if (intObject && mouseInfo.flags == SDL_BUTTON_LEFT) {
      SDL_FRect clickRect = {adjustedMouse.x, adjustedMouse.y, 1, 1};
      if (Collision::AABB(intObject->interactArea, clickRect)) {
        intObject->interact();
        transform.canMove = false;
      }
    }
  }

  /**
   * Handle input by polling mouse state for smooth movement animation
   */
  void pollInput(const MouseInfo mouseInfo, Transform &transform, Sprite &sprite,
                 SDL_Renderer *renderer) {
    bool moving = false;
  
    if (!transform.isMoving && transform.canMove && mouseInfo.flags == SDL_BUTTON_LEFT) {
      Vector2D adjustedMouse = getAdjustedMousePos(
        mouseInfo.xpos, mouseInfo.ypos, renderer
      );
      adjustedMouse.x = adjustedMouse.x - sprite.posOffset.x - (sprite.width / 2);
      adjustedMouse.y = adjustedMouse.y - sprite.posOffset.y - (sprite.height / 2);

      // Create movement vector relative to player sprite, adjusted to tile grid
      Vector2D movement = {
        round(adjustedMouse.x / TILE_SIZE) - round(transform.position.x / TILE_SIZE),
        round(adjustedMouse.y / TILE_SIZE) - round(transform.position.y / TILE_SIZE)
      };

      if (movement.x > 0 && movement.x > movement.y) {
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_NONE;
        transform.initiateMove(Direction::Right);
      } else if (movement.x < 0 && movement.x < movement.y) {
        sprite.play("walk_side");
        sprite.spriteFlip = SDL_FLIP_HORIZONTAL;
        transform.initiateMove(Direction::Left);
      } else if (movement.y > 0 && movement.y > movement.x) {
        sprite.play("walk_down");
        transform.initiateMove(Direction::Down);
      } else if (movement.y < 0 && movement.y < movement.x) {
        sprite.play("walk_up");
        transform.initiateMove(Direction::Up);
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

private:
  /*
   * Take the x and y mouse position and adjust it by render scale and camera
   * poisition to get the position corresponding to the game world.
   */
  Vector2D getAdjustedMousePos(const float xpos, const float ypos, SDL_Renderer *renderer) {
    // Need to adjust mouse click pos by the render scale
    float scaleX, scaleY;
    SDL_GetRenderScale(renderer, &scaleX, &scaleY);

    Vector2D adjusted;
    adjusted.x = (xpos / scaleX) + Camera::position.x;
    adjusted.y = (ypos / scaleY) + Camera::position.y;

    return adjusted;
  }
};
