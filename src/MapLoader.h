#pragma once

#include "Components/Animation.h"
#include "Components/Transform.h"
#include "Parsers/JsonParser.h"
#include "Parsers/TsxParser.h"
#include "Vector2D.h"
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

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
  std::unordered_map<int, MapObject> spriteColliderVector;
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
  JsonObject mapDataJson;
  fs::path mapDir;
  const int tileSize;
  int drawOrderCounter = 0;

  std::string tileLayerName;
  std::string spriteLayerName;
  std::string collisionLayerName;
  std::string transitionLayerName;
  std::string interactionLayerName;
  std::string playerLayerName;

  struct GidTexture {
    int firstGid = 0;
    std::string texPath = "";
  };
  std::unordered_map<int, GidTexture> gidTextures = {};

  enum PropertyType { TILE, SPRITE, SPRITECOLLIDER, COLLISION, TRANSITION, INTERACTION };

  bool processSpriteObject(MapObject &mapObject, const JsonObject &object);

  bool processSpriteCollider(MapObject &mapObject, const JsonObject &object);

  bool processTransitionObject(MapObject &mapObject, const JsonObject &object);

  void processPlayerProperty(const std::string name,
                             const TsxNode tileProperty,
                             const fs::path tilesetDir);

  std::unique_ptr<MapObject> loadObject(const JsonObject &object, PropertyType propertyType);

  std::unordered_map<int, MapObject> loadMapObjects(std::string layerName,
                                                    PropertyType propertyType);

  void addGidTexturesFromTileset(const fs::path &tilesetFile, int firstGid);

  static std::string getTilesetSource(int tilesetID, const JsonArray &tilesets);

  template <typename T>
  static std::optional<T> getProperty(const JsonObject &object,
                                      const std::string &property);
};
