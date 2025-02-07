#include "Game.h"
#include "Collision.h"
#include "Components/Components.h"
#include "Constants.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include <iostream>
#include <ostream>

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
SDL_Rect Game::camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

entt::entity Game::player = entt::null;
entt::entity Game::npc = entt::null;
MapData Game::mapData;
int mapPixelHeight = 0;
int mapPixelWidth = 0;

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

  // Set up player character
  player = registry.create();
  std::vector<Animation> player_anims = {{"walk_front", 0, 4, 200},
                                         {"walk_side", 1, 4, 200},
                                         {"walk_back", 2, 4, 200}};
  registry.emplace<Sprite>(player, "assets/characters/player_anim.png",
                           PLAYER_WIDTH, PLAYER_HEIGHT, player_anims);
  registry.emplace<Transform>(player, 0.0f, 0.0f, 32.0f, 32.0f, true);
  registry.emplace<Collider>(player, 0.0f, 0.0f, 32.0f, 32.0f,
                             Offset{17, 17, -17, -17});
  registry.emplace<KeyboardController>(player);

  // Set up NPCs
  npc = registry.create();
  registry.emplace<Sprite>(npc, "assets/characters/npc.png", PLAYER_WIDTH,
                           PLAYER_HEIGHT);
  registry.emplace<Transform>(npc, 208.0f, 80.0f, 32.0f, 32.0f);
  registry.emplace<Collider>(npc, 208.0f, 80.0f, 32.0f, 32.0f,
                             Offset{17, 17, -17, -17});

  // Set up map data
  mapData = MapLoader::LoadMap("assets/maps/level1.tmj");
  mapPixelHeight = mapData.height * TILE_SIZE;
  mapPixelWidth = mapData.width * TILE_SIZE;
  const entt::entity map = registry.create();
  registry.emplace<Map>(map, &mapData, "assets/tilesets/TilemapOutdoor.png",
                        TILE_SIZE);

  return true;
}

void Game::handleEvents(SDL_Event *event) {
  auto view = registry.view<Transform, Sprite, KeyboardController>();
  for (auto entity : view) {
    auto &controller = view.get<KeyboardController>(entity);
    auto &transform = view.get<Transform>(entity);
    auto &sprite = view.get<Sprite>(entity);
    controller.update(event, transform, sprite);
  }
}

void Game::updateCamera() {
  // Update camera position based on player position
  auto view = registry.view<Transform>();
  auto &playerTransform = view.get<Transform>(player);
  camera.x =
      static_cast<int>(playerTransform.position.x + float(PLAYER_WIDTH) / 2.0f -
                       float(SCREEN_WIDTH) / 2.0f);
  camera.y = static_cast<int>(playerTransform.position.y +
                              float(PLAYER_HEIGHT) / 2.0f -
                              float(SCREEN_HEIGHT) / 2.0f);

  // Keep the camera within bounds of the world
  if (camera.x < 0) {
    camera.x = 0;
  }
  if (camera.y < 0) {
    camera.y = 0;
  }
  if (camera.x > mapPixelWidth - camera.w) {
    camera.x = mapPixelWidth - camera.w;
  }
  if (camera.y > mapPixelHeight - camera.h) {
    camera.y = mapPixelHeight - camera.h;
  }
}

void Game::update() {
  // Update sprite and transform components
  auto view = registry.view<Sprite, Transform, Collider>();
  auto &playerCollider = view.get<Collider>(player);
  for (auto entity : view) {
    auto &sprite = view.get<Sprite>(entity);
    auto &transform = view.get<Transform>(entity);
    auto &collider = view.get<Collider>(entity);
    sprite.update(transform);
    transform.update();
    collider.update(transform);
    if (entity != player) {
      if (Collision::AABB(playerCollider, collider)) {
        std::cout << "Player collision!" << std::endl;
      }
    }
  }

  // Update maps
  auto mapView = registry.view<Map>();
  for (auto mapEntity : mapView) {
    auto &map = mapView.get<Map>(mapEntity);
    map.update();
  }

  updateCamera();
}

void Game::render() {
  SDL_RenderClear(renderer);

  // draw map
  auto mapView = registry.view<Map>();
  for (auto mapEntity : mapView) {
    auto &map = mapView.get<Map>(mapEntity);
    map.render();
  }

  // draw sprites
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
  // Clean up maps
  auto mapView = registry.view<Map>();
  for (auto mapEntity : mapView) {
    auto &map = mapView.get<Map>(mapEntity);
    map.clean();
  }
  registry.clear();

  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
