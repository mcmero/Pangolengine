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
  MapData mapData;

  MapLoader();
  ~MapLoader();

  void LoadMap(const char *mapFile);
};
