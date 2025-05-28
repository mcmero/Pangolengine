#pragma once

#include "Components/Animation.h"
#include "Components/Transform.h"
#include "Vector2D.h"
#include <filesystem>
#include <third_party/nlohmann/json.hpp>
#include <third_party/tinyxml2/tinyxml2.h>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

using namespace nlohmann;

struct MapObject {
  int objectId = -1;
  int linkedId = -1;
  int drawOrderId = -1;
  std::string filePath;
  float width = 32;
  float height = 32;
  float xpos = 0;
  float ypos = 0;
};

struct PlayerObject {
  int objectId = -1;
  int globalId = -1;
  float width = 32;
  float height = 32;
  Offset spriteOffset = {0, 0};
  MapObject collider;
  std::string spriteSheet;
  std::vector<Animation> animations;
};

struct MapData {
  std::vector<std::vector<int>> map;
  float height = 0;
  float width = 0;
  int pixelHeight = 0;
  int pixelWidth = 0;
  Vector2D startPos = {0, 0};
  PlayerObject playerObject = {};
  std::string tilesetImg = "";
  std::unordered_map<int, MapObject> spriteVector;
  std::unordered_map<int, MapObject> colliderVector;
  std::unordered_map<int, MapObject> transitionVector;
  std::unordered_map<int, MapObject> interactionVector;
};

class MapLoader {
public:
  MapLoader(std::string mapFile, const int tileSize,
            std::string tileLayerName = "Tiles",
            std::string spriteLayerName = "Sprites",
            std::string collisionLayerName = "Collision",
            std::string transitionLayerName = "Transition",
            std::string interactionLayerName = "Interaction",
            std::string playerLayerName = "Player");

  ~MapLoader();

  MapData LoadMap();

private:
  MapData mapData;

  std::string mapFile;
  json mapDataJson;
  fs::path mapDir;
  const int tileSize;
  int drawOrderCounter = 0;

  std::string tileLayerName;
  std::string spriteLayerName;
  std::string collisionLayerName;
  std::string transitionLayerName;
  std::string interactionLayerName;
  std::string playerLayerName;

  // Map where index = global ID and string = texture path
  std::unordered_map<int, std::string> gidTextures = {};

  enum PropertyType { TILE, SPRITE, COLLISION, TRANSITION, INTERACTION };

  void processSpriteObject(MapObject &mapObject, const json &object);

  void processTransitionObject(MapObject &mapObject, const json &object);

  void processPlayerProperty(const char *name,
                             const tinyxml2::XMLElement *property,
                             const fs::path tilesetDir);

  MapObject loadObject(const json &object, PropertyType propertyType);

  std::unordered_map<int, MapObject> loadMapObjects(std::string layerName,
                                                    PropertyType propertyType);

  void addGidTexturesFromTileset(const fs::path &tilesetFile, int firstGid);

  static std::string getTilesetSource(int tilesetID, const json &mapDataJson);

  template <typename T>
  static std::optional<T> getProperty(const json &object,
                                      const std::string &property);
};
