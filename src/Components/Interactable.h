#pragma once

#include "../Camera.h"
#include "Collider.h"
#include "Transform.h"
#include <iostream>

class Interactable {
public:
  SDL_FRect interactArea;

  SDL_FRect srcRect, destRect;
  Offset offset;
  bool canInteract = false;

  Interactable(float xpos, float ypos, float width, float height,
               Offset offset = {}) {
    interactArea.x = xpos + offset.x;
    interactArea.y = ypos + offset.y;
    interactArea.w = width;
    interactArea.h = height;
  }

  void update(Transform &transform) {
    interactArea.x = transform.position.x + offset.x;
    interactArea.y = transform.position.y + offset.y;

    destRect.x = interactArea.x - Camera::position.x;
    destRect.y = interactArea.y - Camera::position.y;
  }

  void interact() {
    if (canInteract) {
      std::cout << "Interacted with entity!" << std::endl;
    }
  }

private:
};
