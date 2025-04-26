#include "MapLoader.h"
#include "SDL3/SDL_filesystem.h"
#include "third_party/tinyxml2/tinyxml2.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

MapData MapLoader::LoadMap(const char *mapFile, std::string tileLayerName,
                           std::string spriteLayerName,
                           std::string collisionLayerName,
                           std::string transitionLayerName) {
  std::ifstream f(mapFile);
  if (!f.is_open()) {
    std::stringstream ss;
    ss << "Failled to open map file: " << mapFile << std::endl;
    throw std::runtime_error(ss.str());
  }
  MapData mapData;
  json mapDataJson = json::parse(f);
  json tileDataJson, spriteDataJson, collisionDataJson, transitionDataJson;
  json &layers = mapDataJson["layers"];

  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";

  // Find tile, sprite and collision layers by name
  for (int i = 0; i < layers.size(); i++) {
    if (layers[i]["name"] == tileLayerName) {
      tileDataJson = layers[i];
    } else if (layers[i]["name"] == spriteLayerName) {
      spriteDataJson = layers[i];
    } else if (layers[i]["name"] == collisionLayerName) {
      collisionDataJson = layers[i];
    } else if (layers[i]["name"] == transitionLayerName) {
      transitionDataJson = layers[i];
    }
  }

  // Load tile map data
  if (tileDataJson.size() == 0) {
    std::cerr << "Tile layer not found." << std::endl;
  } else {
    // Get image path for tile set
    int tilesetID = static_cast<int>(tileDataJson["id"]);
    fs::path tilesetFile =
        fs::path(MapLoader::getTilesetSource(tilesetID, mapDataJson));
    tilesetFile = fs::path(mapFile).parent_path() / tilesetFile;
    fs::path tilesetImgPath = MapLoader::getTilesetImageFile(tilesetFile);
    mapData.tilesetImg = fs::canonical(tilesetImgPath).string();

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
  }

  // TODO: clean up/standardise these loading functions

  // Load sprite data
  if (spriteDataJson.size() == 0) {
    std::cerr << "Sprite layer not found." << std::endl;
  } else {
    for (auto object : spriteDataJson["objects"]) {
      int spritesetID = static_cast<int>(object["gid"]);
      SpriteData spriteData;
      fs::path spritesetFile =
          fs::path(MapLoader::getTilesetSource(spritesetID, mapDataJson));
      spritesetFile = fs::path(mapFile).parent_path() / spritesetFile;
      fs::path spritesetTex = MapLoader::getTilesetImageFile(spritesetFile);
      spriteData.height = object["height"];
      spriteData.width = object["width"];
      spriteData.xpos = object["x"];
      spriteData.ypos = object["y"];
      spriteData.ypos = spriteData.ypos - spriteData.height; // correct y coord
      spriteData.texPath = fs::canonical(spritesetTex).string();
      mapData.spriteVector.push_back(spriteData);
    }
  }

  // Load the collider data
  if (collisionDataJson.size() == 0) {
    std::cerr << "Collision layer not found." << std::endl;
  } else {
    for (auto object : collisionDataJson["objects"]) {
      ColliderData colliderData;
      colliderData.height = object["height"];
      colliderData.width = object["width"];
      colliderData.xpos = object["x"];
      colliderData.ypos = object["y"];
      mapData.colliderVector.push_back(colliderData);
    }
  }

  // Load the transition data
  if (transitionDataJson.size() == 0) {
    std::cerr << "Transition layer not found." << std::endl;
  } else {
    for (auto object : transitionDataJson["objects"]) {
      TransitionData transitionData;
      transitionData.height = object["height"];
      transitionData.width = object["width"];
      transitionData.xpos = object["x"];
      transitionData.ypos = object["y"];
      transitionData.mapPath =
          (assetsPath / "maps" / MapLoader::getProperty(object, "map"))
              .string();
      mapData.transitionVector.push_back(transitionData);
    }
  }

  return mapData;
}

fs::path MapLoader::getTilesetImageFile(const fs::path &tilsetFile) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError eResult = doc.LoadFile(tilsetFile.string().c_str());

  if (eResult != tinyxml2::XML_SUCCESS) {
    std::cerr << "Error loading XML file: " << eResult << std::endl;
  }

  tinyxml2::XMLNode *root = doc.FirstChildElement("tileset");
  if (root == nullptr) {
    std::cerr << "No root element found!" << std::endl;
  }

  tinyxml2::XMLElement *imageElement = root->FirstChildElement("image");
  if (imageElement == nullptr) {
    std::cerr << "No 'image' element found." << std::endl;
  }

  const char *imageSource = imageElement->Attribute("source");
  if (imageSource == nullptr) {
    std::cerr << "No 'source' attribute found in 'image' element." << std::endl;
  }

  return tilsetFile.parent_path() / fs::path(imageSource);
}

// Return the path to the tileset definition file given its ID value
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

std::string MapLoader::getProperty(const json &object,
                                   const std::string property) {
  if (object["properties"].size() == 0) {
    std::cerr << "Properties field not found." << std::endl;
    return "";
  }
  for (auto prop : object["properties"]) {
    if (prop["name"] == property) {
      return prop["value"];
    }
  }
  std::cerr << "Property " << property << " not found." << std::endl;
  return "";
}
