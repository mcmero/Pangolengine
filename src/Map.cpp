#include "Map.h"
#include "TextureManager.h"

int lvl1[20][20] = {
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}};

Map::Map() {
  grass = TextureManager::LoadTexture("assets/tilesets/grass.png");
  dirt = TextureManager::LoadTexture("assets/tilesets/dirt.png");
  path = TextureManager::LoadTexture("assets/tilesets/path.png");

  LoadMap(lvl1);
  srcRect.x = srcRect.y = 0;
  srcRect.w = destRect.w = 16;
  srcRect.h = destRect.h = 16;

  destRect.x = destRect.y = 0;
}

void Map::LoadMap(int arr[20][20]) {
  for (int row = 0; row < 20; row++) {
    for (int col = 0; col < 20; col++) {
      map[row][col] = arr[row][col];
    }
  }
}

void Map::DrawMap() {
  int type = 0;
  for (int row = 0; row < 20; row++) {
    for (int col = 0; col < 20; col++) {
      type = map[row][col];

      destRect.x = float(col * 16);
      destRect.y = float(row * 16);

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
