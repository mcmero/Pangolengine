#pragma once

#include <third_party/nlohmann/json.hpp>
#include <vector>

using namespace nlohmann;

struct MapData {
  std::vector<std::vector<int>> map;
  int height;
  int width;
};

class MapLoader {
public:
  MapLoader() = delete;

  static MapData LoadMap(const char *mapFile);

private:
  static std::string getTilesetSource(int tilesetID, const json &mapDataJson);
};
