#pragma once

#include <filesystem>
#include <third_party/nlohmann/json.hpp>
#include <vector>

namespace fs = std::filesystem;

using namespace nlohmann;

struct MapData {
  std::vector<std::vector<int>> map;
  int height;
  int width;
  std::string tilesetImg;
};

class MapLoader {
public:
  MapLoader() = delete;

  static MapData LoadMap(const char *mapFile);

private:
  static std::string getTilesetSource(int tilesetID, const json &mapDataJson);
  static fs::path getTilesetImageFile(const fs::path &tilesetFile);
};
