#include "Game.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3_image/SDL_image.h"
#include <iostream>

SDL_Texture *playerTex;

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

  SDL_Surface *tmpSurface = IMG_Load("src/logo.png");
  playerTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);
  SDL_DestroySurface(tmpSurface);

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
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, playerTex, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void Game::clean() {
  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
