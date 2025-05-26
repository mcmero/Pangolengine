#include "MapLoader.h"
#include "Components/Transform.h"
#include "SDL3/SDL_filesystem.h"
#include "nlohmann/json.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

MapLoader::MapLoader(std::string mapFile, int tileSize,
                     std::string tileLayerName, std::string spriteLayerName,
                     std::string collisionLayerName,
                     std::string transitionLayerName,
                     std::string interactionLayerName)
    : mapFile(mapFile), tileSize(tileSize), tileLayerName(tileLayerName),
      spriteLayerName(spriteLayerName), collisionLayerName(collisionLayerName),
      transitionLayerName(transitionLayerName),
      interactionLayerName(interactionLayerName) {}

MapData MapLoader::LoadMap() {
  std::ifstream f(mapFile);
  if (!f.is_open()) {
    std::stringstream ss;
    ss << "Failed to open map file: " << mapFile << std::endl;
    throw std::runtime_error(ss.str());
  }

  mapDir = fs::path(mapFile).parent_path();
  mapDataJson = json::parse(f);
  json &layers = mapDataJson["layers"];

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
    MapLoader::addGidTexturesFromTileset(tilesetFile, gid);
  }

  // Find tile layer
  json tileDataJson;
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

    // Calculate pixel height and width
    mapData.pixelHeight = static_cast<int>(mapData.height) * tileSize;
    mapData.pixelWidth = static_cast<int>(mapData.width) * tileSize;

    // Get the player starting position (set to 0,0 if not found)
    // TODO: maybe this can be defined by placing a player sprite on the map
    // instead of a property on the tile map?
    mapData.startPos.x =
        MapLoader::getProperty<float>(tileDataJson, "startPos_x").value_or(0);
    mapData.startPos.y =
        MapLoader::getProperty<float>(tileDataJson, "startPos_y").value_or(0);
  }

  mapData.spriteVector = MapLoader::loadMapObjects(spriteLayerName, SPRITE);
  mapData.colliderVector =
      MapLoader::loadMapObjects(collisionLayerName, COLLISION);
  mapData.transitionVector =
      MapLoader::loadMapObjects(transitionLayerName, TRANSITION);
  mapData.interactionVector =
      MapLoader::loadMapObjects(interactionLayerName, INTERACTION);

  // Find and load player object
  // Iterate through tilesets
  bool playerObjectFound = false;
  for (const auto &tileset : mapDataJson["tilesets"]) {
    int gid = tileset.value("firstgid", -1);
    if (gid < 0)
      continue;

    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(gid, mapDataJson));
    tilesetFile = mapDir / tilesetFile;

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.LoadFile(tilesetFile.string().c_str());

    if (eResult != tinyxml2::XML_SUCCESS) {
      throw std::runtime_error(
          "Error loading XML file: " + tilesetFile.string() +
          " (Error Code: " + std::to_string(eResult) + ")");
    }

    tinyxml2::XMLNode *root = doc.FirstChildElement("tileset");
    if (root == nullptr) {
      throw std::runtime_error("No root element found in tileset file: " +
                               tilesetFile.string());
    }

    tinyxml2::XMLElement *imageElement = root->FirstChildElement("image");
    if (imageElement) {
      // Not an object tileset
      continue;
    }

    // Attempt to find player in object tileset
    tinyxml2::XMLElement *tileNode = root->FirstChildElement("tile");
    while (tileNode) {
      int id = 0;
      eResult = tileNode->QueryIntAttribute("id", &id);
      if (eResult != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "No 'id' attribute found in tile node. Error Code: " +
            std::to_string(eResult));
      }

      // Get properties tag if it exists
      tinyxml2::XMLElement *tileProperties =
          tileNode->FirstChildElement("properties");
      if (!tileProperties) {
        // No properties on tile, so keep looking
        tileNode = tileNode->NextSiblingElement("tile");
        continue;
      }

      const char *type;
      eResult = tileNode->QueryStringAttribute("type", &type);

      // Class (type in the XML file) indicates the player object
      if (eResult == tinyxml2::XML_SUCCESS && strcmp(type, "player") == 0) {
        playerObjectFound = true;

        // Set IDs
        mapData.playerObject.objectId = id;
        mapData.playerObject.globalId = gid + id;

        // Get collider -- first object in object group
        tinyxml2::XMLElement *object =
            tileNode->FirstChildElement("objectgroup")
                ->FirstChildElement("object");
        if (object) {
          object->QueryIntAttribute("id",
                                    &mapData.playerObject.collider.objectId);
          object->QueryFloatAttribute("x", &mapData.playerObject.collider.xpos);
          object->QueryFloatAttribute("y", &mapData.playerObject.collider.ypos);
          object->QueryFloatAttribute("width",
                                      &mapData.playerObject.collider.width);
          object->QueryFloatAttribute("height",
                                      &mapData.playerObject.collider.height);
        }
      } else {
        tileNode = tileNode->NextSiblingElement("tile");
        continue;
      }

      // Process player properties
      tinyxml2::XMLElement *tileProperty =
          tileProperties->FirstChildElement("property");
      while (tileProperty) {
        const char *name;
        eResult = tileProperty->QueryStringAttribute("name", &name);

        if (eResult != tinyxml2::XML_SUCCESS) {
          tileProperty = tileProperty->NextSiblingElement("property");
          continue;
        }

        MapLoader::processPlayerProperty(name, tileProperty,
                                         tilesetFile.parent_path());
        tileProperty = tileProperty->NextSiblingElement("property");
      }
      break;
    }
    // Break out of iterating tilesets if we've found the player
    if (playerObjectFound)
      break;
  }
  if (!playerObjectFound) {
    std::cerr << "Warning: player object not found." << std::endl;
  }

  f.close();
  return mapData;
}

MapLoader::~MapLoader() {};

void MapLoader::processSpriteObject(MapObject &mapObject, const json &object) {
  // TODO: at some point, add support for collision boxes specified
  // within the object sprite

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

  std::string mapFileName =
      MapLoader::getProperty<std::string>(object, "map").value_or("");

  if (mapFileName.empty())
    throw std::runtime_error("Map name empty!");

  mapObject.filePath = (assetsPath / "maps" / mapFileName).string();
}

MapObject MapLoader::loadObject(const json &object, PropertyType propertyType) {
  MapObject mapObject;
  mapObject.objectId = object.value("id", -1);
  mapObject.height = object.value("height", 0);
  mapObject.width = object.value("width", 0);
  mapObject.xpos = object.value("x", 0);
  mapObject.ypos = object.value("y", 0);

  // Draw order is set by iteration position, which reflects the draw
  // order in the JSON map file
  mapObject.drawOrderId = drawOrderCounter;
  drawOrderCounter++;

  // Check for linked ID
  mapObject.linkedId =
      MapLoader::getProperty<int>(object, "linked_id").value_or(-1);

  switch (propertyType) {
  case SPRITE:
    processSpriteObject(mapObject, object);
    break;
  case TRANSITION:
    processTransitionObject(mapObject, object);
    break;
  case INTERACTION: {
    fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";

    std::string sceneFileName =
        MapLoader::getProperty<std::string>(object, "scene_file").value_or("");

    mapObject.filePath = (assetsPath / "scenes" / sceneFileName).string();
    break;
  }
  default:
    break;
  }
  return mapObject;
}

void MapLoader::processPlayerProperty(const char *name,
                                      const tinyxml2::XMLElement *tileProperty,
                                      const fs::path tilesetDir) {

  if (strcmp(name, "sprite_offset_x") == 0) {
    float value = 0.0;
    tileProperty->QueryFloatAttribute("value", &value);
    mapData.playerObject.spriteOffset.x = value;

  } else if (strcmp(name, "sprite_offset_y") == 0) {
    float value = 0.0;
    tileProperty->QueryFloatAttribute("value", &value);
    mapData.playerObject.spriteOffset.y = value;

  } else if (strcmp(name, "spritesheet") == 0) {
    const char *value = "";
    tileProperty->QueryStringAttribute("value", &value);
    mapData.playerObject.spriteSheet =
        fs::canonical(tilesetDir / fs::path(value)).string();

  } else if (strcmp(name, "animations") == 0) {
    const char *value = "";
    tileProperty->QueryStringAttribute("value", &value);

    std::string animFilePath = (tilesetDir / fs::path(value)).string();
    std::ifstream animationsFile(animFilePath);
    if (!animationsFile.is_open()) {
      std::stringstream ss;
      ss << "Failed to open animations file: " << value << std::endl;
      throw std::runtime_error(ss.str());
    }

    json animJson = json::parse(animationsFile);
    for (const auto &animVals : animJson["animations"]) {
      std::string animName = animVals.value("name", "");
      Animation anim =
          Animation(animName, animVals.value("index", 0),
                    animVals.value("frames", 1), animVals.value("speed", 100));
      mapData.playerObject.animations.push_back(anim);
    }
    animationsFile.close();
  }
}

std::unordered_map<int, MapObject>
MapLoader::loadMapObjects(std::string layerName, PropertyType propertyType) {
  json objectDataJson;
  json &layers = mapDataJson["layers"];
  for (int i = 0; i < layers.size(); i++) {
    if (layers[i]["name"] == layerName)
      objectDataJson = layers[i];
  }

  std::unordered_map<int, MapObject> mapObjects;
  if (objectDataJson.size() == 0) {
    std::cerr << "Layer " << layerName << " not found." << std::endl;
    return mapObjects;
  }

  drawOrderCounter = 0; // Reset the draw order for new layer
  for (const auto &object : objectDataJson["objects"]) {
    MapObject mapObject = loadObject(object, propertyType);
    mapObjects[mapObject.objectId] = mapObject;
  }

  return mapObjects;
}

void MapLoader::addGidTexturesFromTileset(const fs::path &tilesetFile,
                                          int firstGid) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError eResult = doc.LoadFile(tilesetFile.string().c_str());

  if (eResult != tinyxml2::XML_SUCCESS) {
    throw std::runtime_error("Error loading XML file: " + tilesetFile.string() +
                             " (Error Code: " + std::to_string(eResult) + ")");
  }

  tinyxml2::XMLNode *root = doc.FirstChildElement("tileset");
  if (root == nullptr) {
    throw std::runtime_error("No root element found in tileset file: " +
                             tilesetFile.string());
  }

  tinyxml2::XMLElement *imageElement = root->FirstChildElement("image");
  if (imageElement) {
    // Regular tileset
    const char *imageSource = imageElement->Attribute("source");
    if (imageSource == nullptr) {
      throw std::runtime_error(
          "No 'source' attribute found in 'image' element in tileset file: " +
          tilesetFile.string());
    }
    int tileCount = 0;
    root->ToElement()->QueryIntAttribute("tilecount", &tileCount);

    // Add the same texture path for all tiles in this tileset
    for (int i = 0; i < tileCount; ++i) {
      try {
        fs::path filePath = tilesetFile.parent_path() / fs::path(imageSource);
        gidTextures[firstGid + i] = fs::canonical(filePath).string();
      } catch (const std::exception &e) {
        throw std::runtime_error("Error resolving file path for tile: " +
                                 std::string(e.what()));
      }
    }
  } else {
    // Object tileset, in this case each tile is one image
    tinyxml2::XMLElement *tileNode = root->FirstChildElement("tile");
    while (tileNode) {
      int id = 0;
      eResult = tileNode->QueryIntAttribute("id", &id);
      if (eResult != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "No 'id' attribute found in tile node. Error Code: " +
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
          throw std::runtime_error("Missing 'source' attribute for tile ID " +
                                   std::to_string(id) +
                                   " in tileset file: " + tilesetFile.string());
        }
      } else {
        throw std::runtime_error("No 'image' element found for tile ID " +
                                 std::to_string(id) +
                                 " in tileset file: " + tilesetFile.string());
      }

      tileNode = tileNode->NextSiblingElement("tile");
    }
  }
}
/** Return the path to the tileset definition
 * file given its ID value */
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
std::optional<T> MapLoader::getProperty(const json &object,
                                        const std::string &property) {
  if (object.find("properties") == object.end() ||
      object["properties"].empty()) {
    return std::nullopt;
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
          return prop["value"].get<int>();
      } else if constexpr (std::is_same_v<T, std::string>) {
        if (type == "string")
          return prop["value"].get<std::string>();
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}
