#pragma once

#include "Vector2D.h"
#include <filesystem>
#include <third_party/nlohmann/json.hpp>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

using namespace nlohmann;

struct MapObject {
  std::string filePath;
  float width;
  float height;
  float xpos;
  float ypos;
};

struct MapData {
  std::vector<std::vector<int>> map;
  float height;
  float width;
  Vector2D startPos;
  std::string tilesetImg;
  std::vector<MapObject> spriteVector;
  std::vector<MapObject> colliderVector;
  std::vector<MapObject> transitionVector;
};

class MapLoader {
public:
  MapLoader() = delete;

  static MapData LoadMap(const char *mapFile,
                         std::string tileLayerName = "Tiles",
                         std::string spriteLayerName = "Sprites",
                         std::string collisionLayerName = "Collision",
                         std::string transitionLayerName = "Transition");

private:
  enum PropertyType { TILE, SPRITE, COLLISION, TRANSITION };

  static std::string getTilesetSource(int tilesetID, const json &mapDataJson);

  static void processTileObject(MapObject &mapObject, const json &object,
                                const json &mapDataJson,
                                const fs::path &mapDir);

  static void
  processSpriteObject(MapObject &mapObject, const json &object,
                      const json &mapDataJson, const fs::path &mapDir,
                      const std::unordered_map<int, std::string> &gidTextures);

  static void processTransitionObject(MapObject &mapObject, const json &object);

  static MapObject
  loadObject(const json &object, const json &mapDataJson,
             const fs::path &mapDir, PropertyType propertyType,
             const std::unordered_map<int, std::string> &gidTextures);

  static std::vector<MapObject>
  loadMapObjects(json &mapDataJson, std::string layerName,
                 PropertyType propertyType, fs::path mapDir,
                 const std::unordered_map<int, std::string> &gidTextures);

  static void
  addGidTexturesFromTileset(std::unordered_map<int, std::string> &gidTextures,
                            const fs::path &tilesetFile, int firstGid);
  template <typename T>
  static T getProperty(const json &object, const std::string &property);
};
