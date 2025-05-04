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
#include <unordered_map>

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
  json tileDataJson;
  json &layers = mapDataJson["layers"];

  // Map where index = global ID and string = texture path
  std::unordered_map<int, std::string> gidTextures = {};

  // We need at least one tileset, otherwise the map has no textures...
  if (!mapDataJson.contains("tilesets")) {
    throw std::runtime_error("No tilesets found!");
  }

  // Build global ID to texture lookup
  for (const auto &tileset : mapDataJson["tilesets"]) {
    int gid = tileset.value("firstgid", -1);
    if (gid < 0)
      continue;

    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(gid, mapDataJson));
    tilesetFile = mapDir / tilesetFile;
    MapLoader::addGidTexturesFromTileset(gidTextures, tilesetFile, gid);
  }

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
    mapData.tilesetImg = gidTextures[tilesetID];

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

  mapData.spriteVector = MapLoader::loadMapObjects(mapDataJson, spriteLayerName,
                                                   SPRITE, mapDir, gidTextures);
  mapData.colliderVector = MapLoader::loadMapObjects(
      mapDataJson, collisionLayerName, COLLISION, mapDir, gidTextures);
  mapData.transitionVector = MapLoader::loadMapObjects(
      mapDataJson, transitionLayerName, TRANSITION, mapDir, gidTextures);

  return mapData;
}

void MapLoader::processSpriteObject(
    MapObject &mapObject, const json &object, const json &mapDataJson,
    const fs::path &mapDir,
    const std::unordered_map<int, std::string> &gidTextures) {
  // Correct y coord to top-left corner as
  // Tiled uses bottom-left
  mapObject.ypos -= mapObject.height;

  // Get path to texture for sprite object
  int spritesetID = static_cast<int>(object["gid"]);
  auto it = gidTextures.find(spritesetID);
  if (it != gidTextures.end()) {
    mapObject.filePath = it->second;
  } else {
    throw std::runtime_error("spritesetID not found in gidTextures: " +
                             std::to_string(spritesetID));
  }
}

void MapLoader::processTransitionObject(MapObject &mapObject,
                                        const json &object) {
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  mapObject.filePath =
      (assetsPath / "maps" / MapLoader::getProperty<std::string>(object, "map"))
          .string();
}

MapObject
MapLoader::loadObject(const json &object, const json &mapDataJson,
                      const fs::path &mapDir, PropertyType propertyType,
                      const std::unordered_map<int, std::string> &gidTextures) {
  MapObject mapObject;
  mapObject.height = object.value("height", 0);
  mapObject.width = object.value("width", 0);
  mapObject.xpos = object.value("x", 0);
  mapObject.ypos = object.value("y", 0);

  switch (propertyType) {
  case SPRITE:
    processSpriteObject(mapObject, object, mapDataJson, mapDir, gidTextures);
    break;
  case TRANSITION:
    processTransitionObject(mapObject, object);
    break;
  default:
    break;
  }
  return mapObject;
}

// TODO: the passing of gidTextures is getting a bit wild, consider refactoring
// to non-static class
std::vector<MapObject> MapLoader::loadMapObjects(
    json &mapDataJson, std::string layerName, PropertyType propertyType,
    fs::path mapDir, const std::unordered_map<int, std::string> &gidTextures) {
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
    MapObject mapObject =
        loadObject(object, mapDataJson, mapDir, propertyType, gidTextures);
    mapObjects.push_back(mapObject);
  }

  return mapObjects;
}

void MapLoader::addGidTexturesFromTileset(
    std::unordered_map<int, std::string> &gidTextures,
    const fs::path &tilesetFile, int firstGid) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError eResult = doc.LoadFile(tilesetFile.string().c_str());

  if (eResult != tinyxml2::XML_SUCCESS) {
    throw std::runtime_error("Error loading XML file: " + tilesetFile.string() +
                             " (Error Code: " + std::to_string(eResult) + ")");
  }

  tinyxml2::XMLNode *root = doc.FirstChildElement("tileset");
  if (root == nullptr) {
    throw std::runtime_error("No root element found in tileset "
                             "file: " +
                             tilesetFile.string());
  }

  tinyxml2::XMLElement *imageElement = root->FirstChildElement("image");
  if (imageElement) {
    // Regular tileset
    const char *imageSource = imageElement->Attribute("source");
    if (imageSource == nullptr) {
      throw std::runtime_error("No 'source' attribute found in "
                               "'image' element in tileset "
                               "file: " +
                               tilesetFile.string());
    }
    int tileCount = 0;
    root->ToElement()->QueryIntAttribute("tilecount", &tileCount);

    // Add the same texture path for all
    // tiles in this tileset
    for (int i = 0; i < tileCount; ++i) {
      try {
        fs::path filePath = tilesetFile.parent_path() / fs::path(imageSource);
        gidTextures[firstGid + i] = fs::canonical(filePath).string();
      } catch (const std::exception &e) {
        throw std::runtime_error("Error resolving file path for "
                                 "tile: " +
                                 std::string(e.what()));
      }
    }
  } else {
    // Object tileset, in this case each
    // tile is one image
    tinyxml2::XMLElement *tileNode = root->FirstChildElement("tile");
    while (tileNode) {
      int id = 0;
      eResult = tileNode->QueryIntAttribute("id", &id);
      if (eResult != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("No 'id' attribute found in "
                                 "tile node. Error "
                                 "Code: " +
                                 std::to_string(eResult));
      }

      // Each tile has its own image tag
      tinyxml2::XMLElement *tileImageNode =
          tileNode->FirstChildElement("image");
      if (tileImageNode) {
        const char *imageSource = tileImageNode->Attribute("source");
        if (imageSource) {
          fs::path filePath = tilesetFile.parent_path() / fs::path(imageSource);
          gidTextures[firstGid + id] = fs::canonical(filePath).string();
        } else {
          throw std::runtime_error("Missing 'source' attribute "
                                   "for tile ID " +
                                   std::to_string(id) +
                                   " in tileset file: " + tilesetFile.string());
        }
      } else {
        throw std::runtime_error("No 'image' element found for "
                                 "tile ID " +
                                 std::to_string(id) +
                                 " in tileset file: " + tilesetFile.string());
      }

      tileNode = tileNode->NextSiblingElement("tile");
    }
  }
}
// Return the path to the tileset definition
// file given its ID value
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

  std::cerr << "ID value not found in "
               "tileset definition."
            << std::endl;
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
      throw std::runtime_error("Property type mismatch or "
                               "unsupported type.");
    }
  }
  throw std::runtime_error("Property " + property + " not found.");
}
