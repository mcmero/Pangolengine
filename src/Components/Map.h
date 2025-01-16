#pragma once

#include "../MapLoader.h"
#include "../TextureManager.h"
#include "SDL3/SDL_render.h"

class Map {
public:
  struct Tile {
    int x, y;
    int width, height;
    SDL_Texture *texture;
  };

  int tileSize;
  MapData *mapData;
  std::vector<Tile> tiles;

  SDL_FRect srcRect, destRect;
  SDL_Texture *dirt;
  SDL_Texture *grass;
  SDL_Texture *path;

  Map(MapData *mapData, int tileSize) {
    assert(mapData->width > 0 && mapData->height > 0);

    this->tileSize = tileSize;
    this->mapData = mapData;

    srcRect.x = srcRect.y = 0;
    srcRect.w = destRect.w = float(tileSize);
    srcRect.h = destRect.h = float(tileSize);

    destRect.x = destRect.y = 0;

    grass = TextureManager::LoadTexture("assets/tilesets/grass.png");
    dirt = TextureManager::LoadTexture("assets/tilesets/dirt.png");
    path = TextureManager::LoadTexture("assets/tilesets/path.png");

    int type = 0;
    for (int row = 0; row < mapData->height; row++) {
      for (int col = 0; col < mapData->width; col++) {
        type = mapData->map[row][col];

        switch (type) {
        case 2:
          tiles.push_back({col, row, tileSize, tileSize, grass});
          break;
        case 3:
          tiles.push_back({col, row, tileSize, tileSize, dirt});
          break;
        case 4:
          tiles.push_back({col, row, tileSize, tileSize, path});
          break;
        default:
          break;
        }
      }
    }
  }

  void render() {
    for (auto &tile : tiles) {
      destRect.x = float(tile.x * tile.width);
      destRect.y = float(tile.y * tile.height);
      TextureManager::Draw(tile.texture, srcRect, destRect);
    }
  }
};
