#include "Game.h"
#include <cmath>
#include <iostream>

Game::Game() : running(true), window(nullptr), renderer(nullptr) {}

Game::~Game() { clean(); }

bool Game::initialise(SDL_Window *win, SDL_Renderer *rend) {
  window = win;
  renderer = rend;

  if (!window || !renderer) {
    std::cerr << "Invalid window or renderer!" << std::endl;
    return false;
  }

  SDL_Log("Game started successfully!");

  return true;
}

void Game::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      running = false;
    }
  }
}

void Game::update(float deltaTime) {
  // Update game state
}

void Game::render() {
  auto time = SDL_GetTicks() / 1000.f;
  auto red = (std::sin(time) + 1) / 2.0 * 255;
  auto green = (std::sin(time / 2) + 1) / 2.0 * 255;
  auto blue = (std::sin(time) * 2 + 1) / 2.0 * 255;

  SDL_SetRenderDrawColor(renderer, red, green, blue, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
}

void Game::clean() {
  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
