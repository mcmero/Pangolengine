#pragma once

#include "../Game.h"
#include "../MapLoader.h"
#include "../TextureManager.h"
#include "../Vector2D.h"
#include "SDL3/SDL_render.h"

class Map {
public:
  struct Tile {
    Vector2D position;
    Vector2D mapPosition;
    int width, height;
    int index;
    SDL_Texture *texture;
  };

  int tileSize;
  MapData *mapData;
  std::vector<Tile> tiles;

  SDL_FRect srcRect, destRect;
  SDL_Texture *tileMapTex;

  Map(MapData *mapData, const char *tileMapImage, int tileSize) {
    assert(mapData->width > 0 && mapData->height > 0);

    this->tileSize = tileSize;
    this->mapData = mapData;

    srcRect.x = srcRect.y = 0;
    destRect.x = destRect.y = 0;

    srcRect.w = destRect.w = float(tileSize);
    srcRect.h = destRect.h = float(tileSize);

    tileMapTex = TextureManager::LoadTexture(tileMapImage);

    int index = 0;
    for (int row = 0; row < mapData->height; row++) {
      for (int col = 0; col < mapData->width; col++) {
        index = mapData->map[row][col] - 1;
        tiles.push_back({Vector2D(float(col), float(row)),
                         Vector2D(float(col), float(row)), tileSize, tileSize,
                         index, tileMapTex});
      }
    }
  }

  void render() {
    for (auto &tile : tiles) {
      srcRect.x = float(tile.index * tile.width);
      srcRect.y = float(0);
      destRect.x = float(tile.position.x * tile.width);
      destRect.y = float(tile.position.y * tile.height);
      TextureManager::Draw(tile.texture, srcRect, destRect);
    }
  }

  void update() {
    for (auto &tile : tiles) {
      tile.position.x = tile.mapPosition.x - static_cast<int>(Game::camera.x);
      tile.position.y = tile.mapPosition.y - static_cast<int>(Game::camera.y);
    }
  }

  void clean() { SDL_DestroyTexture(tileMapTex); }
};
