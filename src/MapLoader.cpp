#include "MapLoader.h"
#include "third_party/tinyxml2/tinyxml2.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
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

MapData MapLoader::LoadMap(const char *mapFile, std::string tileLayerName,
                           std::string spriteLayerName,
                           std::string collisionLayerName) {
  std::ifstream f(mapFile);
  MapData mapData;
  json mapDataJson = json::parse(f);
  json tileData, spriteData, collisionData;
  json &layers = mapDataJson["layers"];

  // Find tile, sprite and collision layers by name
  for (int i = 0; i < layers.size(); i++) {
    if (layers[i]["name"] == tileLayerName) {
      tileData = layers[i];
    } else if (layers[i]["name"] == spriteLayerName) {
      spriteData = layers[i];
    } else if (layers[i]["name"] == collisionLayerName) {
      collisionData = layers[i];
    }
  }

  // Load tile map data
  if (tileData.size() == 0) {
    std::cerr << "Tile layer not found." << std::endl;
  } else {
    // Get image path for tile set
    int tilesetID = static_cast<int>(tileData["id"]);
    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(tilesetID, mapDataJson));
    tilesetFile = fs::path(mapFile).parent_path() / tilesetFile;
    fs::path tilesetImgPath = MapLoader::getTilesetImageFile(tilesetFile);
    mapData.tilesetImg = fs::canonical(tilesetImgPath).string();

    // Set up tile set dimensions
    mapData.height = tileData["height"];
    mapData.width = tileData["width"];
    for (int i = 0; i < mapData.height; i++) {
      std::vector<int> row;
      for (int j = 0; j < mapData.width; j++) {
        row.push_back(tileData["data"][i * mapData.width + j]);
      }
      mapData.map.push_back(row);
    }
  }

  // Load sprite data
  if (spriteData.size() == 0) {
    std::cerr << "Sprite layer not found." << std::endl;
  } else {
    for (auto object : spriteData["objects"]) {
      int spritesetID = static_cast<int>(object["gid"]);
      SpriteData spriteData;
      fs::path spritesetFile =
          fs::path(MapLoader::getTilesetSource(spritesetID, mapDataJson));
      spritesetFile = fs::path(mapFile).parent_path() / spritesetFile;
      fs::path spritesetTex = MapLoader::getTilesetImageFile(spritesetFile);
      spriteData.height = object["height"];
      spriteData.width = object["width"];
      spriteData.xpos = object["x"];
      spriteData.ypos = object["y"];
      spriteData.texPath = spritesetTex.string();
      mapData.spriteVector.push_back(spriteData);
    }
  }

  return mapData;
}
