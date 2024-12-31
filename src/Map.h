#pragma once

#include "Game.h"
#include "SDL3/SDL_render.h"

class Map {
public:
  Map();
  ~Map();

  void LoadMap(int arr[20][20]);
  void DrawMap();

private:
  SDL_FRect srcRect, destRect;
  SDL_Texture *dirt;
  SDL_Texture *grass;
  SDL_Texture *path;

  int map[20][20];
};
