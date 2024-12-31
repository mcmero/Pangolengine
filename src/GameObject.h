#pragma once

#include "Game.h"
#include "SDL3/SDL_render.h"

class GameObject {
public:
  GameObject(const char *textureSheet, int x, int y);
  ~GameObject();

  void Update();
  void Render();

private:
  int xpos;
  int ypos;

  SDL_Texture *objTexture;
  SDL_FRect srcRect, destRect;
};
