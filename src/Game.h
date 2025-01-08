#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <third_party/entt/entt.hpp>

class Game {
public:
  Game();
  ~Game();

  bool initialise(SDL_Window *window, SDL_Renderer *renderer);
  void handleEvents();
  void update();
  void render();
  void clean();

  bool isRunning() const { return running; }

  static SDL_Renderer *renderer;

private:
  bool running;
  SDL_Window *window;
  entt::registry registry;
};
