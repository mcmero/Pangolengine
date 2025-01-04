#pragma once

#include "Game.h"
#include "SDL3/SDL_render.h"
#include <third_party/nlohmann/json.hpp>
#include <vector>

using namespace nlohmann;

class Map {
public:
  Map();
  ~Map();

  void LoadMap(json map_data);
  void DrawMap();

private:
  SDL_FRect srcRect, destRect;
  SDL_Texture *dirt;
  SDL_Texture *grass;
  SDL_Texture *path;

  std::vector<std::vector<int>> map;
  int height;
  int width;
};
