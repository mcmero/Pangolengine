#include "MapLoader.h"
#include "Components/Transform.h"
#include "JsonParser.h"
#include "TsxParser.h"
#include "SDL3/SDL_filesystem.h"
#include "third_party/tinyxml2/tinyxml2.h"
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <array>

namespace fs = std::filesystem;

MapLoader::MapLoader(std::string mapFile, int tileSize,
                     std::string tileLayerName, std::string spriteLayerName,
                     std::string collisionLayerName,
                     std::string transitionLayerName,
                     std::string interactionLayerName,
                     std::string playerLayerName)
    : mapFile(mapFile), tileSize(tileSize), tileLayerName(tileLayerName),
      spriteLayerName(spriteLayerName), collisionLayerName(collisionLayerName),
      transitionLayerName(transitionLayerName),
      interactionLayerName(interactionLayerName),
      playerLayerName(playerLayerName) {}

MapData MapLoader::LoadMap() {
  mapDataJson = JsonParser::parseJson(mapFile);

  // We need at least one tileset, otherwise the map has no textures...
  if (!mapDataJson.contains("tilesets")) {
    throw std::runtime_error("No tilesets found!");
  }

  // Make a global ID to texture location lookup
  JsonArray tilesets = mapDataJson["tilesets"].getArray();
  mapDir = fs::path(mapFile).parent_path();
  for (const auto &tileset : tilesets) {
    int gid = static_cast<int>(tileset.at("firstgid").getNumber());
    if (gid < 0)
      continue;
    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(gid, tilesets));
    tilesetFile = mapDir / tilesetFile;
    MapLoader::addGidTexturesFromTileset(tilesetFile, gid);
  }

  // Find tile layer
  JsonObject tileDataJson;
  JsonArray &layers = mapDataJson["layers"].getArray();
  for (const auto &layer : layers) {
    std::string layerName = layer.at("name").getString();
    if (layerName == tileLayerName) {
      tileDataJson = layer.getObject();
    }
  }

  // Load tile map data
  if (tileDataJson.size() == 0) {
    throw std::runtime_error("Map file must have a tile layer");
  } else {
    // Validate required keys
    const std::array<std::string, 4> keys = {"id", "height", "width", "data"};
    for(const std::string& key : keys) {
      if (!tileDataJson.contains(key)) {
        throw std::runtime_error(
          "Map tile layer is missing required key: " + key
        );
      }
    }

    // Get image path for tile set
    int tilesetID = static_cast<int>(tileDataJson["id"].getNumber());
    mapData.tilesetImg = gidTextures[tilesetID].texPath;

    // Set up tile set dimensions
    mapData.height = static_cast<float>(tileDataJson["height"].getNumber());
    mapData.width = static_cast<float>(tileDataJson["width"].getNumber());

    int h = static_cast<int>(mapData.height);
    int w = static_cast<int>(mapData.width);
    JsonArray tileData = tileDataJson["data"].getArray();
    for (int i = 0; i < h; i++) {
      std::vector<int> row;
      for (int j = 0; j < w; j++) {
        int val = static_cast<int>(tileData[i * w + j].getNumber());
        row.push_back(val);
      }
      mapData.map.push_back(row);
    }

    // Calculate pixel height and width
    mapData.pixelHeight = static_cast<int>(mapData.height) * tileSize;
    mapData.pixelWidth = static_cast<int>(mapData.width) * tileSize;
  }

  mapData.spriteVector = MapLoader::loadMapObjects(spriteLayerName, SPRITE);
  mapData.spriteColliderVector =
      MapLoader::loadMapObjects(spriteLayerName, SPRITECOLLIDER);
  mapData.colliderVector =
      MapLoader::loadMapObjects(collisionLayerName, COLLISION);
  mapData.transitionVector =
      MapLoader::loadMapObjects(transitionLayerName, TRANSITION);
  mapData.interactionVector =
      MapLoader::loadMapObjects(interactionLayerName, INTERACTION);

  // Load start position from first object in player layer
  std::unordered_map<int, MapObject> playerMap =
      MapLoader::loadMapObjects(playerLayerName, SPRITE);
  if (playerMap.size() > 1)
    std::cerr
        << "Warning: player layer has multiple objects. Expected only one."
        << std::endl;
  else if (playerMap.size() == 0)
    std::cerr << "Warning: player layer has no objects. Assuming player starts "
                 "at 0,0."
              << std::endl;

  // Start pos is first object's x and y position rounded to the tile size
  for (const auto &object : playerMap) {
    mapData.startPos.x = std::round(object.second.xpos / TILE_SIZE) * TILE_SIZE;
    mapData.startPos.y = std::round(object.second.ypos / TILE_SIZE) * TILE_SIZE;
    break;
  }

  // Find and load player object
  // Iterate through tilesets
  bool playerObjectFound = false;
  for (const auto &tileset : tilesets) {
    int gid = static_cast<int>(tileset.at("firstgid").getNumber());
    if (gid < 0)
      continue;

    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(gid, tilesets));
    tilesetFile = mapDir / tilesetFile;

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.LoadFile(tilesetFile.string().c_str());

    // JUST FOR TESTING
    std::vector<TsxNode> nodes = TsxParser::parseTsx(tilesetFile.string());

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

        // Get sprite dimensions
        tinyxml2::XMLElement *image = tileNode->FirstChildElement("image");
        if (image) {
          image->QueryFloatAttribute("width", &mapData.playerObject.width);
          image->QueryFloatAttribute("height", &mapData.playerObject.height);
        }

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

  return mapData;
}

MapLoader::~MapLoader() {};

/*
 * Gets any collision objects attached to sprites and returns true if successful
 */
bool MapLoader::processSpriteCollider(MapObject &mapObject, const JsonObject &object) {
  // Get the tileset source from the map file using the GID value
  int objectGid = static_cast<int>(object.at("gid").getNumber());
  int firstGid = gidTextures[objectGid].firstGid;
  std::string source = "";
  for (const auto &tileset : mapDataJson["tilesets"].getArray()) {
    int tilesetGid = static_cast<int>(tileset.at("firstgid").getNumber());
    if (tilesetGid == firstGid) {
      source = tileset.at("source").getString();
      break;
    }
  }

  // Exit function if there's no tilemap file
  if (source == "") {
    std::cerr << "Warning: no tilemap found for map object id: " <<
      mapObject.objectId << std::endl;
    return false;
  }

  fs::path tilesetFile = mapDir / fs::path(source);

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

  tinyxml2::XMLElement *tileNode = root->FirstChildElement("tile");
  while (tileNode) {
    int id = 0;
    eResult = tileNode->QueryIntAttribute("id", &id);
    if (eResult != tinyxml2::XML_SUCCESS) {
      throw std::runtime_error(
          "No 'id' attribute found in tile node. Error Code: " +
          std::to_string(eResult));
    }

    // Find the correct file using global ID
    if (firstGid + id == objectGid) {
      // Get collider attached to tile
      tinyxml2::XMLElement *objectGroup =
          tileNode->FirstChildElement("objectgroup");
      if (objectGroup) {
        tinyxml2::XMLElement *tileObject = objectGroup->FirstChildElement("object");
        if (tileObject) {
          float xpos = 0; float ypos = 0; float width = 0; float height = 0;
          tileObject->QueryFloatAttribute("x", &xpos);
          tileObject->QueryFloatAttribute("y", &ypos);
          tileObject->QueryFloatAttribute("width", &width);
          tileObject->QueryFloatAttribute("height", &height);

          // Set coordinates relative to sprite position
          mapObject.xpos = mapObject.xpos + xpos;
          mapObject.ypos = mapObject.ypos + ypos;
          mapObject.width = width;
          mapObject.height = height;

          break; // No need to keep iterating tiles
        }
      }
      // If we've reached this code, there is no collider
      return false;
    }
    tileNode = tileNode->NextSiblingElement("tile");
  }
  return true;
}

/*
 * Process sprite object and return true if successful
 */
bool MapLoader::processSpriteObject(MapObject &mapObject, const JsonObject &object) {
  // Get path to texture for sprite object
  int spritesetID = static_cast<int>(object.at("gid").getNumber());
  auto it = gidTextures.find(spritesetID);
  if (it != gidTextures.end()) {
    mapObject.filePath = it->second.texPath;
    return true;
  }

  std::cerr << "spritesetID not found in gidTextures: " +
    std::to_string(spritesetID) << std::endl;
  return false;
}

/*
 * Process transition object and return true if successful
 */
bool MapLoader::processTransitionObject(MapObject &mapObject,
                                        const JsonObject &object) {
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";

  std::string mapFileName =
      MapLoader::getProperty<std::string>(object, "map").value_or("");

  if (mapFileName.empty()) {
    std::cerr << "Warning: Map name empty!" << std::endl;
    return false;
  }

  mapObject.filePath = (assetsPath / "maps" / mapFileName).string();
  return true;
}

std::unique_ptr<MapObject> MapLoader::loadObject(const JsonObject &object, PropertyType propertyType) {
  std::unique_ptr<MapObject> mapObject = std::make_unique<MapObject>();
  mapObject->objectId = static_cast<int>(object.at("id").getNumber());
  mapObject->height = static_cast<float>(object.at("height").getNumber());
  mapObject->width = static_cast<float>(object.at("width").getNumber());
  mapObject->xpos = static_cast<float>(object.at("x").getNumber());
  mapObject->ypos = static_cast<float>(object.at("y").getNumber());

  // Draw order is set by iteration position, which reflects the draw
  // order in the JSON map file
  mapObject->drawOrderId = drawOrderCounter;
  drawOrderCounter++;

  // Check for linked ID
  mapObject->linkedId =
      MapLoader::getProperty<int>(object, "linked_id").value_or(-1);

  bool loadSuccess = false;
  switch (propertyType) {
  case COLLISION:
    // Already processed, all we needed are the values already captured
    loadSuccess = true;
    break;
  case SPRITE:
    loadSuccess = processSpriteObject(*mapObject, object);
    break;
  case SPRITECOLLIDER:
    loadSuccess = processSpriteCollider(*mapObject, object);
    break;
  case TRANSITION:
    loadSuccess = processTransitionObject(*mapObject, object);
    break;
  case INTERACTION: {
    fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";

    std::string sceneFileName =
        MapLoader::getProperty<std::string>(object, "scene_file").value_or("");

    mapObject->filePath = (assetsPath / "scenes" / sceneFileName).string();
    loadSuccess = true;
    break;
  }
  default:
    break;
  }

  if (loadSuccess)
    return mapObject;
  else
   return nullptr;
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
    JsonObject animJson = JsonParser::parseJson(animFilePath);
    for (const auto &animVals : animJson["animations"].getArray()) {
      std::string animName = animVals.at("name").getString();
      int index = static_cast<int>(
        animVals.at("index").getNumber()
      );
      int frames = static_cast<int>(
        animVals.at("frames").getNumber()
      );
      int speed = static_cast<int>(
        animVals.at("speed").getNumber()
      );
      Animation anim = Animation(animName, index, frames, speed);
      mapData.playerObject.animations.push_back(anim);
    }
  }
}

std::unordered_map<int, MapObject>
MapLoader::loadMapObjects(std::string layerName, PropertyType propertyType) {
  JsonObject objectDataJson;
  JsonArray &layers = mapDataJson["layers"].getArray();

  for (const auto &layer : layers) {
    std::string currentLayerName = layer.at("name").getString();
    if (layerName == currentLayerName) {
      objectDataJson = layer.getObject();
    }
  }

  std::unordered_map<int, MapObject> mapObjects;
  if (objectDataJson.size() == 0) {
    std::cerr << "Warning: Layer " << layerName << " not found." << std::endl;
    return mapObjects;
  }

  drawOrderCounter = 0; // Reset the draw order for new layer
  for (const auto &object : objectDataJson["objects"].getArray()) {
    std::unique_ptr<MapObject> mapObject = loadObject(object.getObject(),
                                                      propertyType);
    if (mapObject)
      mapObjects[mapObject->objectId] = *mapObject;
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
        gidTextures[firstGid + i] = {
          firstGid,
          fs::canonical(filePath).string()
        };
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
          gidTextures[firstGid + id] = {
            firstGid,
            fs::canonical(filePath).string()
          };
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
                                        const JsonArray &tilesets) {
  for (const auto &tileset : tilesets) {
    JsonObject tilesetObj = tileset.getObject();
    if (!tilesetObj.contains("firstgid") || !tilesetObj.contains("source"))
      throw std::runtime_error(
        "Tileset does not contain 'firstgid' and/or 'source'"
      );

    std::string source = tilesetObj["source"].getString();
    int gid = static_cast<int>(tilesetObj["firstgid"].getNumber());
    if (gid > 0 && gid == tilesetID)
      return source;
  }

  std::cerr << "ID value not found in tileset definition." << std::endl;
  return "";
}

template <typename T>
std::optional<T> MapLoader::getProperty(const JsonObject &object,
                                        const std::string &property) {
  if (!object.contains("properties"))
    return std::nullopt;

  for (const auto &prop : object.at("properties").getArray()) {
    if (prop.at("name").getString() == property) {
      // Check if type matches
      JsonObject propObj = prop.getObject();
      const std::string type = propObj["type"].getString();
      if constexpr (std::is_same_v<T, float>) {
        if (type == "float")
          return static_cast<float>(propObj["value"].getNumber());
      } else if constexpr (std::is_same_v<T, int>) {
        if (type == "int")
          return static_cast<int>(propObj["value"].getNumber());
      } else if constexpr (std::is_same_v<T, std::string>) {
        if (type == "string")
          return propObj["value"].getString();
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}
