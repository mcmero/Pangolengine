#pragma once

#include "Components/ECS.h"
#include "MapLoader.h"
#include "IGame.h"
#include "UI/UIManager.h"
#include "SDL3/SDL_events.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Engine {
public:
  Engine(const char* windowTitle, int width, int height);
  ~Engine();

  bool initialise(IGame* game);
  void handleEvent(SDL_Event *event);
  void updateCamera();
  void iterate();
  void render();
  void cleanup();

  SDL_Renderer* getRenderer() { return renderer; }
  SDL_Window* getWindow() { return window; }
  EntityRegistry& getRegistry() { return registry; }

  bool isRunning() const { return running; }
  void quit();

  static SDL_Renderer *renderer;
  static MapData mapData;

  UIManager *uiManager;
  static int mapPixelHeight;
  static int mapPixelWidth;

private:
  IGame* gameImpl;
  SDL_Window *window;
  const char* windowTitle;

  int windowWidth;
  int windowHeight;
  bool running;

  EntityRegistry registry = {};
  static EntityId playerId;
  static EntityId mapId;
};
