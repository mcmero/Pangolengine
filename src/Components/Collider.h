#pragma once

#include "../Camera.h"
#include "../Engine.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include "Transform.h"

class Collider {
public:
  SDL_FRect collider;
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

    this->offset = offset;
  }

  void update(Transform &transform) {
    collider.x = transform.position.x + offset.x;
    collider.y = transform.position.y + offset.y;

    destRect.x = collider.x - Camera::position.x;
    destRect.y = collider.y - Camera::position.y;
  }

  void render() {
    SDL_SetRenderDrawColor(Engine::renderer, 0, 255, 0,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderRect(Engine::renderer, &destRect);
    SDL_RenderFillRect(Engine::renderer, &destRect);
  }

private:
  SDL_FRect destRect;
};
