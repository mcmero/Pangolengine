#pragma once

#include "../Game.h"
#include "../TextureManager.h"
#include "Transform.h"

class Collider {
public:
  SDL_FRect collider;

  SDL_Texture *tex;
  SDL_FRect srcRect, destRect;

  Collider(float xpos, float ypos, float width, float height) {
    collider.x = xpos;
    collider.y = ypos;
    collider.w = width;
    collider.h = height;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x;
    collider.y = transform.position.y;
    collider.w = transform.width;
    collider.h = transform.height;

    destRect.x = collider.x - Game::camera.x;
    destRect.y = collider.y - Game::camera.y;
  }

  void draw() { TextureManager::Draw(tex, srcRect, destRect, SDL_FLIP_NONE); }

private:
};
