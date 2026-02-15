#pragma once

#include "../Constants.h"
#include "../Vector2D.h"
#include <SDL3/SDL_stdinc.h>
#include <iostream>

// Offset is used for the positioning of the component,
// relative to its transform component
struct Offset {
  float x = 0;
  float y = 0;
};
  
enum class Direction { Up, Down, Left, Right, None };

class Transform {
public:
  float width;
  float height;

  Vector2D position;
  Vector2D targetPosition;
  Vector2D startPosition;

  float moveProgress; // between 0 and 1
  bool isMoving = false;
  bool isPlayer = false;
  bool canMove = true;
  Direction lastDirection = Direction::None;

  Transform() { position = Vector2D(); }

  Transform(float x, float y, float width, float height,
            bool isPlayer = false) {
    position = Vector2D(x, y);
    startPosition = Vector2D(x, y);
    this->width = width;
    this->height = height;
    this->isPlayer = isPlayer;
  }

  void update() {
    if (isMoving) {
      // Calculate the distance to move this frame
      const float distance = PLAYER_SPEED / 60.0f;
      float totalDistance = static_cast<float>(SDL_sqrt(
          (targetPosition.x - position.x) * (targetPosition.x - position.x) +
          (targetPosition.y - position.y) * (targetPosition.y - position.y)));
      moveProgress += distance / totalDistance;

      if (moveProgress >= 1.0f) {
        // Movement complete
        position.x = targetPosition.x;
        position.y = targetPosition.y;
        isMoving = false;
      } else {
        // Interpolate position
        position.x =
            position.x + (targetPosition.x - position.x) * moveProgress;
        position.y =
            position.y + (targetPosition.y - position.y) * moveProgress;
      }
    }
  }

  void initiateMove(Direction dir) {
    if (lastDirection == dir && !isMoving) {
      startPosition.x = position.x;
      startPosition.y = position.y;
      Vector2D moveVect = {0, 0};

      // Determine position change vector
      switch (dir) {
      case Direction::Up:
        moveVect.y = -1.0f * TILE_SIZE;
        break;
      case Direction::Down:
        moveVect.y = TILE_SIZE;
        break;
      case Direction::Left:
        moveVect.x = -1.0f * TILE_SIZE;
        break;
      case Direction::Right:
        moveVect.x = TILE_SIZE;
        break;
      default:
        break;
      }
    
      // Add vector to target position
      targetPosition.x = position.x + moveVect.x;
      targetPosition.y = position.y + moveVect.y;
      std::cout << "Moving to (" << targetPosition.x << "," << targetPosition.y
                << ")" << std::endl;

      // Update properties
      isMoving = true;
      moveProgress = 0.0f;
    }
    lastDirection = dir;
  }

  void abortMove() {
    position.x = startPosition.x;
    position.y = startPosition.y;
    isMoving = false;
    moveProgress = 1.0f;
  }
};
