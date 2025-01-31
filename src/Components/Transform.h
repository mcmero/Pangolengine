#pragma once

#include "../Constants.h"
#include "../Game.h"
#include "../Vector2D.h"
#include <SDL3/SDL_stdinc.h>

class Transform {
public:
  Vector2D position;
  Vector2D targetPosition;
  Vector2D mapPosition;
  float moveProgress; // between 0 and 1
  bool isMoving = false;
  bool isPlayer = false;

  Transform() { position = Vector2D(); }

  Transform(float x, float y, bool isPlayer = false) {
    position = Vector2D(x, y);
    mapPosition = Vector2D(x, y);
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
    if (!isPlayer) {
      position.x = mapPosition.x - static_cast<int>(Game::camera.x);
      position.y = mapPosition.y - static_cast<int>(Game::camera.y);
    }
  }

  void initiateMove(Vector2D pos) {
    targetPosition.x = position.x + pos.x;
    targetPosition.y = position.y + pos.y;
    isMoving = true;
    moveProgress = 0.0f;
  }
};
