#pragma once

#include "../Camera.h"
#include "../Game.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include "Transform.h"
#include "ECS.h"

class Collider : IComponent {
public:
  SDL_FRect collider;

  SDL_Texture *tex;
  SDL_FRect destRect;
  Offset offset;

  Collider(float xpos, float ypos, float width, float height,
           Transform &transform, Offset offset = {0, 0}) {
    collider.x = xpos + offset.x;
    collider.y = ypos + offset.y;
    collider.w = width;
    collider.h = height;

    destRect.x = collider.x - Camera::position.x;
    destRect.y = collider.y - Camera::position.y;
    destRect.w = width;
    destRect.h = height;

    this->transform = transform;
    this->offset = offset;
  }

  void update() override {
    collider.x = transform.position.x + offset.x;
    collider.y = transform.position.y + offset.y;

    destRect.x = collider.x - Camera::position.x;
    destRect.y = collider.y - Camera::position.y;
  }

  void render() override {
    SDL_SetRenderDrawColor(Game::renderer, 0, 255, 0,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderRect(Game::renderer, &destRect);
    SDL_RenderFillRect(Game::renderer, &destRect);
  }

private:
  Transform transform = {};
};
