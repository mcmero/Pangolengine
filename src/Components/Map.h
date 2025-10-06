#pragma once

#include "../Camera.h"
#include "../MapLoader.h"
#include "../TextureManager.h"
#include "../Vector2D.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"

class Map {
public:
  struct Tile {
    Vector2D position;
    Vector2D mapPosition;
    int width, height;
    int index;
    SDL_Texture *texture;
  };

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
        tiles.push_back({Vector2D(float(col) * tileSize, float(row) * tileSize),
                         Vector2D(float(col) * tileSize, float(row) * tileSize),
                         tileSize, tileSize, index, tileMapTex});
      }
    }
  }

  void render() {
    for (auto &tile : tiles) {
      int xidx = tile.index % (tile.texture->w / tile.width);
      int yidx = tile.index / (tile.texture->w / tile.width);

      // Make sure index doesn't exceed the texture height
      assert((yidx + 1) * tile.height <= tile.texture->h);

      srcRect.x = float(xidx * tile.width);
      srcRect.y = float(yidx * tile.height);
      destRect.x = float(tile.position.x);
      destRect.y = float(tile.position.y);

      TextureManager::Draw(tile.texture, srcRect, destRect, SDL_FLIP_NONE);
    }
  }

  void update() {
    for (auto &tile : tiles) {
      tile.position.x = tile.mapPosition.x - Camera::position.x;
      tile.position.y = tile.mapPosition.y - Camera::position.y;
    }
  }

  void clean() {
    SDL_DestroyTexture(tileMapTex);
    tiles.clear();
  }

  ~Map() { clean(); }

private:
  int tileSize;
  MapData *mapData;
  std::vector<Tile> tiles;

  SDL_FRect srcRect, destRect;
  SDL_Texture *tileMapTex;
};
