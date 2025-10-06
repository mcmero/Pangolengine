#pragma once

#include "Components/ECS.h"
#include "MapLoader.h"
#include "SDL3/SDL_events.h"
#include "UI/UIManager.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <unordered_map>

class Game {
public:
  Game();
  ~Game();

  bool initialise(SDL_Window *window, SDL_Renderer *renderer);
  void handleEvents(SDL_Event *event);
  void updateCamera();
  void update();
  void render();
  void clean();

  bool isRunning() const { return running; }

  static SDL_Renderer *renderer;
  static MapData mapData;

  UIManager *uiManager;
  static int mapPixelHeight;
  static int mapPixelWidth;

private:
  bool running;
  SDL_Window *window;

  EntityRegistry registry = {};
  static EntityId playerId;
  static EntityId mapId;

  static std::unordered_map<int, EntityId> mapEntities;

  void loadPlayer();
  void unloadMap();
  void loadMap(const std::string mapPath);

  template <typename T>
  void clearEntities(std::unordered_map<int, T> entityVector);
};
