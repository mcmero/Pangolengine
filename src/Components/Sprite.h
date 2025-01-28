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
      srcRect.x =
          srcRect.w *
          static_cast<int>((SDL_GetTicks() / animations[anim_idx].speed) %
                           animations[anim_idx].frames);
    }
    srcRect.y = animations[anim_idx].index * srcRect.h;

    destRect.x = float(transform.position.x);
    destRect.y = float(transform.position.y);
    destRect.w = srcRect.w;
    destRect.h = srcRect.h;
  }

  void render() {
    TextureManager::Draw(texture, srcRect, destRect, spriteFlip);
  }

  void play(const char *animName) {
    for (int i = 0; i < animations.size(); i++) {
      if (strcmp(animations[i].name, animName) == 0) {
        anim_idx = i;
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
  std::vector<Animation> animations;
  int anim_idx = 0;
};
