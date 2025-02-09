#include "MapLoader.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

// Return the path to the tileset definition file given its ID value
std::string MapLoader::getTilesetSource(int tilesetID,
                                        const json &tilesetInfo) {
  if (!tilesetInfo.contains("tilesets")) {
    return "";
  }

  for (const auto &tileset : tilesetInfo["tilesets"]) {
    if (tileset.value("firstgid", -1) == tilesetID) {
      return tileset.value("source", "");
    }
  }

  return "";
}

MapData MapLoader::LoadMap(const char *mapFile) {
  std::ifstream f(mapFile);
  json mapDataJson = json::parse(f);
  json &tileData = mapDataJson["layers"][0];

  int tilesetID = static_cast<int>(tileData["id"]);
  fs::path tilesetFile =
      fs::path(MapLoader::getTilesetSource(tilesetID, mapDataJson));
  tilesetFile = fs::path(mapFile).parent_path() / tilesetFile;
  std::cout << "Tileset file: " << tilesetFile << std::endl;

  std::ifstream file(tilesetFile);
  if (!file.is_open()) {
    std::cerr << "Failed to open file." << std::endl;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }

  MapData mapData;
  mapData.height = tileData["height"];
  mapData.width = tileData["width"];
  for (int i = 0; i < mapData.height; i++) {
    std::vector<int> row;
    for (int j = 0; j < mapData.width; j++) {
      row.push_back(tileData["data"][i * mapData.width + j]);
    }
    mapData.map.push_back(row);
  }

  return mapData;
}
