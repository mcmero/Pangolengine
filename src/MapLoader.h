#pragma once

#include <filesystem>
#include <third_party/nlohmann/json.hpp>
#include <vector>

namespace fs = std::filesystem;

using namespace nlohmann;

struct ColliderData {
  float width;
  float height;
  float xpos;
  float ypos;
};

struct SpriteData {
  std::string texPath;
  float width;
  float height;
  float xpos;
  float ypos;
};

struct MapData {
  std::vector<std::vector<int>> map;
  float height;
  float width;
  std::string tilesetImg;
  std::vector<SpriteData> spriteVector;
  std::vector<ColliderData> colliderVector;
};

class MapLoader {
public:
  MapLoader() = delete;

  static MapData LoadMap(const char *mapFile,
                         std::string tileLayerName = "Tiles",
                         std::string spriteLayerName = "Sprites",
                         std::string collisionLayerName = "Collision");

private:
  static std::string getTilesetSource(int tilesetID, const json &mapDataJson);
  static fs::path getTilesetImageFile(const fs::path &tilesetFile);
};
