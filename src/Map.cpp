#include "Map.h"
#include "Constants.h"
#include "TextureManager.h"
#include <fstream>

std::ifstream f("assets/maps/level1.tmj");
json lvl1 = json::parse(f)["layers"][0];

Map::Map() {
  grass = TextureManager::LoadTexture("assets/tilesets/grass.png");
  dirt = TextureManager::LoadTexture("assets/tilesets/dirt.png");
  path = TextureManager::LoadTexture("assets/tilesets/path.png");

  LoadMap(lvl1);
  srcRect.x = srcRect.y = 0;
  srcRect.w = destRect.w = TILE_SIZE;
  srcRect.h = destRect.h = TILE_SIZE;

  destRect.x = destRect.y = 0;
}

void Map::LoadMap(json map_data) {
  height = map_data["height"];
  width = map_data["width"];
  for (int i = 0; i < height; i++) {
    std::vector<int> row;
    for (int j = 0; j < width; j++) {
      row.push_back(map_data["data"][i * width + j]);
    }
    map.push_back(row);
  }
}

void Map::DrawMap() {
  assert(width > 0 && height > 0);
  int type = 0;
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      type = map[row][col];

      destRect.x = float(col * TILE_SIZE);
      destRect.y = float(row * TILE_SIZE);

      switch (type) {
      case 2:
        TextureManager::Draw(grass, srcRect, destRect);
        break;
      case 3:
        TextureManager::Draw(dirt, srcRect, destRect);
        break;
      case 4:
        TextureManager::Draw(path, srcRect, destRect);
        break;
      default:
        break;
      }
    }
  }
}
