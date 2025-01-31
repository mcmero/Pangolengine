#pragma once

#include "MapLoader.h"
#include "SDL3/SDL_events.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <third_party/entt/entt.hpp>

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
  static SDL_Event event;
  static SDL_Rect camera;
  static MapData mapData;

  int mapPixelHeight;
  int mapPixelWidth;

private:
  bool running;
  SDL_Window *window;
  entt::registry registry;
  static entt::entity player;
  static entt::entity npc;
};
