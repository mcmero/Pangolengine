#include "Game.h"
#include "SDL3/SDL_render.h"
#include "TextureManager.h"
#include <iostream>

SDL_Texture *playerTex;
SDL_FRect srcR, destR;
int cnt = 0;

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

  playerTex =
      TextureManager::LoadTexture("assets/characters/player.png", renderer);

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

  cnt++;
  if (cnt >= 320)
    cnt = 0;
  destR.h = 32;
  destR.w = 32;
  destR.x = float(cnt);
  std::cout << cnt << std::endl;
}

void Game::render() {
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, playerTex, NULL, &destR);
  SDL_RenderPresent(renderer);
}

void Game::clean() {
  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
