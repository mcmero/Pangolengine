#pragma once

#include "../Game.h"
#include "../TextureManager.h"
#include "Animation.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
#include "Transform.h"
#include <iostream>
#include <vector>

class Sprite {
public:
  SDL_FlipMode spriteFlip = SDL_FLIP_NONE;

  Sprite(const char *texturePath, float width, float height,
         std::vector<Animation> anims = {}) {
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.h = width;
    srcRect.w = height;

    texture = TextureManager::LoadTexture(texturePath);
    animations = anims;
    std::cout << texturePath << std::endl;
  }

  void update(Transform &transform) {
    if ((animated || animUnfinished) && !animations.empty()) {
      // If we just started this animation, calculate an
      // offset to ensure we always start on frame 0.
      // Only need this offset when moving from stationary
      if (animStart && !transform.isMoving) {
        animFrameOffset = ((SDL_GetTicks() / animations[animIdx].speed) %
                           animations[animIdx].frames);
        animStart = false;
      }

      int drawFrame = ((SDL_GetTicks() / animations[animIdx].speed) %
                       animations[animIdx].frames) -
                      animFrameOffset;

      if (drawFrame < 0) {
        drawFrame += animations[animIdx].frames;
      }

      srcRect.x = srcRect.w * drawFrame;

      // if we've reached the first frame again, mark
      // animation as finished
      if (drawFrame == 0 && !animated) {
        animUnfinished = false;
      }
    }

    if (!animations.empty()) {
      srcRect.y = animations[animIdx].index * srcRect.h;
    } else {
      srcRect.y = 0;
    }

    // update sprite position relative to camera ensuring sprite
    // remains on fixed position on the tile grid
    destRect.x = float(transform.position.x - Game::camera.x);
    destRect.y = float(transform.position.y - Game::camera.y);
    destRect.w = srcRect.w;
    destRect.h = srcRect.h;
  }

  void render() {
    TextureManager::Draw(texture, srcRect, destRect, spriteFlip);
  }

  void play(const char *animName) {
    for (int i = 0; i < animations.size(); i++) {
      if (strcmp(animations[i].name, animName) == 0) {
        animIdx = i;
        animated = true;
        animStart = true;
        animUnfinished = true;
      }
    }
  }
  void stop() {
    animated = false;
    animStart = 0;
  }

  void clean() { SDL_DestroyTexture(texture); }

private:
  SDL_Texture *texture;
  SDL_FRect srcRect, destRect;
  bool animated = false;
  bool animStart = false;
  bool animUnfinished = false;
  int animFrameOffset = 0;
  std::vector<Animation> animations;
  int animIdx = 0;
};
