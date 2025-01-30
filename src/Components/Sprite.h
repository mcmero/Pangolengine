#pragma once

#include "../TextureManager.h"
#include "Animation.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
#include "Transform.h"
#include <vector>

class Sprite {
public:
  SDL_FlipMode spriteFlip = SDL_FLIP_NONE;

  Sprite(const char *texturePath, int width, int height,
         std::vector<Animation> anims = {}) {
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.h = float(width);
    srcRect.w = float(height);

    texture = TextureManager::LoadTexture(texturePath);
    animations = anims;
  }

  void update(Transform &transform) {
    if (animated && !animations.empty()) {
      // TODO: put a check here to make sure that we have played at least one
      // cycle of the animation; we also need to start the animation at the
      // first frame
      srcRect.x = srcRect.w * static_cast<int>(
                                  (SDL_GetTicks() / animations[animIdx].speed) %
                                  animations[animIdx].frames);
      animStart = false;
    }
    srcRect.y = animations[animIdx].index * srcRect.h;

    destRect.x = float(transform.position.x);
    destRect.y = float(transform.position.y);
    destRect.w = srcRect.w;
    destRect.h = srcRect.h;
  }

  void render() {
    TextureManager::Draw(texture, srcRect, destRect, spriteFlip);
  }

  void play(const char *animName) {
    animStart = true;
    for (int i = 0; i < animations.size(); i++) {
      if (strcmp(animations[i].name, animName) == 0) {
        animIdx = i;
        animated = true;
      }
    }
  }
  void stop() { animated = false; }

  void clean() { SDL_DestroyTexture(texture); }

private:
  SDL_Texture *texture;
  SDL_FRect srcRect, destRect;
  bool animated = false;
  bool animStart = false;
  std::vector<Animation> animations;
  int animIdx = 0;
};
