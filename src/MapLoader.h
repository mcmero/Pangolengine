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
  int pixelHeight;
  int pixelWidth;
  Vector2D startPos;
  std::string tilesetImg;
  std::vector<MapObject> spriteVector;
  std::vector<MapObject> colliderVector;
  std::vector<MapObject> transitionVector;
};

class MapLoader {
public:
  MapLoader(std::string mapFile, const int tileSize,
            std::string tileLayerName = "Tiles",
            std::string spriteLayerName = "Sprites",
            std::string collisionLayerName = "Collision",
            std::string transitionLayerName = "Transition");

  ~MapLoader();

  MapData LoadMap();

private:
  MapData mapData;

  std::string mapFile;
  json mapDataJson;
  fs::path mapDir;
  const int tileSize;

  std::string tileLayerName;
  std::string spriteLayerName;
  std::string collisionLayerName;
  std::string transitionLayerName;

  // Map where index = global ID and string = texture path
  std::unordered_map<int, std::string> gidTextures = {};

  enum PropertyType { TILE, SPRITE, COLLISION, TRANSITION };

  void processTileObject(MapObject &mapObject, const json &object);

  void processSpriteObject(MapObject &mapObject, const json &object);

  void processTransitionObject(MapObject &mapObject, const json &object);

  MapObject loadObject(const json &object, PropertyType propertyType);

  std::vector<MapObject> loadMapObjects(std::string layerName,
                                        PropertyType propertyType);

  void addGidTexturesFromTileset(const fs::path &tilesetFile, int firstGid);

  static std::string getTilesetSource(int tilesetID, const json &mapDataJson);

  template <typename T>
  static T getProperty(const json &object, const std::string &property);
};
