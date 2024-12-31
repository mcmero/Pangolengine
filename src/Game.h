#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Game {
public:
  Game();
  ~Game();

  bool initialise(SDL_Window *window, SDL_Renderer *renderer);
  void handleEvents();
  void update(float deltaTime);
  void render();
  void clean();

  bool isRunning() const { return running; }

  static SDL_Renderer *renderer;

private:
  bool running;
  SDL_Window *window;
};
