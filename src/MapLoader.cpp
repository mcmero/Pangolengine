#include "MapLoader.h"
#include "third_party/tinyxml2/tinyxml2.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

fs::path MapLoader::getTilesetImageFile(const fs::path &tilsetFile) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError eResult = doc.LoadFile(tilsetFile.string().c_str());

  if (eResult != tinyxml2::XML_SUCCESS) {
    std::cerr << "Error loading XML file: " << eResult << std::endl;
  }

  tinyxml2::XMLNode *root = doc.FirstChildElement("tileset");
  if (root == nullptr) {
    std::cerr << "No root element found!" << std::endl;
  }

  tinyxml2::XMLElement *imageElement = root->FirstChildElement("image");
  if (imageElement == nullptr) {
    std::cerr << "No 'image' element found." << std::endl;
  }

  const char *imageSource = imageElement->Attribute("source");
  if (imageSource == nullptr) {
    std::cerr << "No 'source' attribute found in 'image' element." << std::endl;
  }

  return tilsetFile.parent_path() / fs::path(imageSource);
}

// Return the path to the tileset definition file given its ID value
std::string MapLoader::getTilesetSource(int tilesetID,
                                        const json &tilesetInfo) {
  if (!tilesetInfo.contains("tilesets")) {
    std::cerr << "No 'tilesets' key found." << std::endl;
    return "";
  }

  for (const auto &tileset : tilesetInfo["tilesets"]) {
    if (tileset.value("firstgid", -1) == tilesetID) {
      return tileset.value("source", "");
    }
  }

  std::cerr << "ID value not found in tileset definition." << std::endl;
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
  fs::path tilesetImgPath = MapLoader::getTilesetImageFile(tilesetFile);

  MapData mapData;
  mapData.tilesetImg = fs::canonical(tilesetImgPath).string();
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
