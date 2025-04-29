#include "MapLoader.h"
#include "SDL3/SDL_filesystem.h"
#include "nlohmann/json.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

MapData MapLoader::LoadMap(const char *mapFile, std::string tileLayerName,
                           std::string spriteLayerName,
                           std::string collisionLayerName,
                           std::string transitionLayerName) {
  std::ifstream f(mapFile);
  if (!f.is_open()) {
    std::stringstream ss;
    ss << "Failed to open map file: " << mapFile << std::endl;
    throw std::runtime_error(ss.str());
  }

  MapData mapData;
  fs::path mapDir = fs::path(mapFile).parent_path();
  json mapDataJson = json::parse(f);
  json tileDataJson, spriteObjectJson, collisionDataJson, transitionDataJson;

  json &layers = mapDataJson["layers"];

  // Find tile layer
  for (int i = 0; i < layers.size(); i++) {
    if (layers[i]["name"] == tileLayerName) {
      tileDataJson = layers[i];
    }
  }

  // Load tile map data
  if (tileDataJson.size() == 0) {
    throw std::runtime_error("Map file must have a tile layer");
  } else {
    // Get image path for tile set
    int tilesetID = static_cast<int>(tileDataJson["id"]);
    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(tilesetID, mapDataJson));
    tilesetFile = mapDir / tilesetFile;
    fs::path tilesetImgPath = MapLoader::getTilesetImageFile(tilesetFile);
    mapData.tilesetImg = fs::canonical(tilesetImgPath).string();

    // Set up tile set dimensions
    mapData.height = tileDataJson["height"];
    mapData.width = tileDataJson["width"];
    int h = static_cast<int>(mapData.height);
    int w = static_cast<int>(mapData.width);
    for (int i = 0; i < h; i++) {
      std::vector<int> row;
      for (int j = 0; j < w; j++) {
        row.push_back(tileDataJson["data"][i * w + j]);
      }
      mapData.map.push_back(row);
    }

    // Get the player starting position (set to 0,0 if not found)
    try {
      mapData.startPos.x =
          MapLoader::getProperty<float>(tileDataJson, "startPos_x");
      mapData.startPos.y =
          MapLoader::getProperty<float>(tileDataJson, "startPos_y");
    } catch (std::runtime_error e) {
      std::cout << "Start pos could not be set from map, setting to 0,0."
                << std::endl;
      mapData.startPos.x = 0;
      mapData.startPos.y = 0;
    }
  }

  mapData.spriteVector =
      MapLoader::loadMapObjects(mapDataJson, spriteLayerName, SPRITE, mapDir);
  mapData.colliderVector = MapLoader::loadMapObjects(
      mapDataJson, collisionLayerName, COLLISION, mapDir);
  mapData.transitionVector = MapLoader::loadMapObjects(
      mapDataJson, transitionLayerName, TRANSITION, mapDir);

  return mapData;
}

void MapLoader::processSpriteObject(MapObject &mapObject, const json &object,
                                    const json &mapDataJson,
                                    const fs::path &mapDir) {
  // Correct y coord to top-left corner as Tiled uses bottom-left
  mapObject.ypos -= mapObject.height;

  // Get path to texture for sprite object
  int spritesetID = static_cast<int>(object["gid"]);
  auto spritesetFile =
      mapDir / MapLoader::getTilesetSource(spritesetID, mapDataJson);
  auto spritesetTex = MapLoader::getTilesetImageFile(spritesetFile);
  mapObject.filePath = fs::canonical(spritesetTex).string();
}

void MapLoader::processTransitionObject(MapObject &mapObject,
                                        const json &object) {
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  mapObject.filePath =
      (assetsPath / "maps" / MapLoader::getProperty<std::string>(object, "map"))
          .string();
}

MapObject MapLoader::loadObject(const json &object, const json &mapDataJson,
                                const fs::path &mapDir,
                                PropertyType propertyType) {
  MapObject mapObject;
  mapObject.height = object.value("height", 0);
  mapObject.width = object.value("width", 0);
  mapObject.xpos = object.value("x", 0);
  mapObject.ypos = object.value("y", 0);

  switch (propertyType) {
  case SPRITE:
    processSpriteObject(mapObject, object, mapDataJson, mapDir);
    break;
  case TRANSITION:
    processTransitionObject(mapObject, object);
    break;
  default:
    break;
  }
  return mapObject;
}

std::vector<MapObject> MapLoader::loadMapObjects(json &mapDataJson,
                                                 std::string layerName,
                                                 PropertyType propertyType,
                                                 fs::path mapDir) {
  json objectDataJson;
  json &layers = mapDataJson["layers"];
  for (int i = 0; i < layers.size(); i++) {
    if (layers[i]["name"] == layerName)
      objectDataJson = layers[i];
  }

  std::vector<MapObject> mapObjects;
  if (objectDataJson.size() == 0) {
    std::cerr << "Layer " << layerName << " not found." << std::endl;
    return mapObjects;
  }

  for (const auto &object : objectDataJson["objects"]) {
    MapObject mapObject = loadObject(object, mapDataJson, mapDir, propertyType);
    mapObjects.push_back(mapObject);
  }

  return mapObjects;
}

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

template <typename T>
T MapLoader::getProperty(const json &object, const std::string &property) {
  if (object.find("properties") == object.end() ||
      object["properties"].size() == 0) {
    throw std::runtime_error("Properties field not found.");
  }
  for (const auto &prop : object["properties"]) {
    if (prop["name"] == property) {
      // Check if type matches
      const std::string type = prop["type"];
      if constexpr (std::is_same_v<T, float>) {
        if (type == "float")
          return prop["value"].get<float>();
      } else if constexpr (std::is_same_v<T, int>) {
        if (type == "int")
          return prop["value"].get<float>();
      } else if constexpr (std::is_same_v<T, std::string>) {
        if (type == "string")
          return prop["value"].get<std::string>();
      }
      throw std::runtime_error("Property type mismatch or unsupported type.");
    }
  }
  throw std::runtime_error("Property " + property + " not found.");
}
