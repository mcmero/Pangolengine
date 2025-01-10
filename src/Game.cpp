#include "Game.h"
#include "Components/Sprite.h"
#include "Map.h"
#include "SDL3/SDL_render.h"
#include <iostream>

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

  const entt::entity player = registry.create();
  registry.emplace<Sprite>(player, "assets/characters/player.png", 32, 32);
  registry.emplace<Transform>(player, 0, 0);
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

void Game::update() {
  auto view = registry.view<Sprite, Transform>();
  for (auto entity : view) {
    auto &sprite = view.get<Sprite>(entity);
    auto &transform = view.get<Transform>(entity);
    transform.update();
    sprite.update(transform);
  }
}

void Game::render() {
  SDL_RenderClear(renderer);
  map->DrawMap();
  auto view = registry.view<Sprite>();
  for (auto entity : view) {
    auto &sprite = view.get<Sprite>(entity);
    sprite.render();
  }
  SDL_RenderPresent(renderer);
}

void Game::clean() {
  // Clean up sprites
  auto view = registry.view<Sprite>();
  for (auto entity : view) {
    auto &sprite = view.get<Sprite>(entity);
    sprite.clean();
  }
  registry.clear();

  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
