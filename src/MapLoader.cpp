#include "MapLoader.h"
#include <fstream>

MapLoader::MapLoader() {}

MapLoader::~MapLoader() {}

void MapLoader::LoadMap(const char *mapFile) {
  std::ifstream f(mapFile);
  json map_data = json::parse(f)["layers"][0];

  mapData.height = map_data["height"];
  mapData.width = map_data["width"];
  for (int i = 0; i < mapData.height; i++) {
    std::vector<int> row;
    for (int j = 0; j < mapData.width; j++) {
      row.push_back(map_data["data"][i * mapData.width + j]);
    }
    mapData.map.push_back(row);
  }
}
