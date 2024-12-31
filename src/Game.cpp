#include "Game.h"
#include "GameObject.h"
#include "Map.h"
#include "SDL3/SDL_render.h"
#include <iostream>
GameObject *player;
Map *map;

SDL_Renderer *Game::renderer = nullptr;

Game::Game() : running(true), window(nullptr) {}

Game::~Game() { clean(); }

bool Game::initialise(SDL_Window *win, SDL_Renderer *rend) {
  window = win;
  renderer = rend;

  if (!window || !renderer) {
    std::cerr << "Invalid window or renderer!" << std::endl;
    return false;
  }
  SDL_Log("Game started successfully!");

  player = new GameObject("assets/characters/player.png", 0, 0);
  map = new Map();

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

void Game::update(float deltaTime) { player->Update(); }

void Game::render() {
  SDL_RenderClear(renderer);
  map->DrawMap();
  player->Render();
  SDL_RenderPresent(renderer);
}

void Game::clean() {
  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
