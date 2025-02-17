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
std::vector<entt::entity> Game::mapSprites = {};
std::vector<entt::entity> Game::mapColliders = {};
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
  registry.emplace<Transform>(npc, 208.0f, 112.0f, 32.0f, 32.0f);
  registry.emplace<Collider>(npc, 208.0f, 112.0f, 32.0f, 32.0f,
                             Offset{17, 17, -17, -17});

  // Set up map data
  mapData = MapLoader::LoadMap("assets/maps/level1.tmj");
  mapPixelHeight = static_cast<int>(mapData.height) * TILE_SIZE;
  mapPixelWidth = static_cast<int>(mapData.width) * TILE_SIZE;
  const entt::entity map = registry.create();
  registry.emplace<Map>(map, &mapData, mapData.tilesetImg.c_str(), TILE_SIZE);

  // Set up map sprites
  for (auto sprite : mapData.spriteVector) {
    entt::entity spriteEntity = registry.create();
    mapSprites.push_back(spriteEntity);
    registry.emplace<Sprite>(spriteEntity, sprite.texPath.c_str(), sprite.width,
                             sprite.height);
    registry.emplace<Transform>(spriteEntity, sprite.xpos, sprite.ypos,
                                sprite.width, sprite.height);
  }

  // Set up map colliders
  for (auto collider : mapData.colliderVector) {
    entt::entity colliderEntity = registry.create();
    mapColliders.push_back(colliderEntity);
    std::cout << "Collider entity " << " x: " << collider.xpos
              << " y: " << collider.ypos << std::endl;
    registry.emplace<Collider>(colliderEntity, collider.xpos, collider.ypos,
                               collider.width, collider.height);
    registry.emplace<Transform>(colliderEntity, collider.xpos, collider.ypos,
                                collider.width, collider.height);
  }
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
  // Define view for sprites with colliders
  auto spriteView = registry.view<Sprite, Transform, Collider>();

  // Player collider and transform components
  auto &playerCollider = spriteView.get<Collider>(player);
  auto &playerTransform = spriteView.get<Transform>(player);

  // Update all sprites without colliders
  auto bgSpriteView = registry.view<Sprite, Transform>(entt::exclude<Collider>);
  for (auto entity : bgSpriteView) {
    auto &sprite = bgSpriteView.get<Sprite>(entity);
    auto &transform = bgSpriteView.get<Transform>(entity);
    transform.update();
    sprite.update(transform);
  }

  // Update all colliders without sprites
  auto colliderView = registry.view<Collider, Transform>(entt::exclude<Sprite>);
  for (auto entity : colliderView) {
    auto &collider = colliderView.get<Collider>(entity);
    auto &transform = bgSpriteView.get<Transform>(entity);
    transform.update();
    if (Collision::AABB(playerCollider, collider)) {
      std::cout << "Player collision!" << std::endl;
      playerTransform.abortMove();
    }
    collider.update(transform);
  }

  // Update sprites that have colliders
  for (auto entity : spriteView) {
    auto &sprite = spriteView.get<Sprite>(entity);
    auto &transform = spriteView.get<Transform>(entity);
    auto &collider = spriteView.get<Collider>(entity);
    transform.update();
    collider.update(transform);
    if (entity != player && Collision::AABB(playerCollider, collider)) {
      std::cout << "Player collision!" << std::endl;
      playerTransform.abortMove();
    }
    sprite.update(transform);
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
