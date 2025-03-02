#include "Game.h"
#include "Camera.h"
#include "Collision.h"
#include "Components/Components.h"
#include "Constants.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <filesystem>
#include <iostream>
#include <ostream>

namespace fs = std::filesystem;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;

entt::entity Game::player = entt::null;
entt::entity Game::npc = entt::null;
std::vector<entt::entity> Game::mapSprites = {};
std::vector<entt::entity> Game::mapColliders = {};

MapData Game::mapData;
int Game::mapPixelHeight = 0;
int Game::mapPixelWidth = 0;

Game::Game() : running(true) {}

Game::~Game() { clean(); }

bool Game::initialise(SDL_Window *win, SDL_Renderer *rend) {
  window = win;
  renderer = rend;

  if (!window || !renderer) {
    std::cerr << "Invalid window or renderer!" << std::endl;
    return false;
  }
  SDL_Log("Game started successfully!");

  // Define paths to assets
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  std::string playerSpriteSheet =
      (assetsPath / "characters" / "player_anim.png").string();
  std::string npcSpriteSheet = (assetsPath / "characters" / "npc.png").string();
  std::string level1Map = (assetsPath / "maps" / "level1.tmj").string();

  // Set up player character
  player = registry.create();
  std::vector<Animation> playerAnims = {{"walk_front", 0, 4, 200},
                                        {"walk_side", 1, 4, 200},
                                        {"walk_back", 2, 4, 200}};
  registry.emplace<Sprite>(player, playerSpriteSheet.c_str(), PLAYER_WIDTH,
                           PLAYER_HEIGHT, playerAnims);
  registry.emplace<Transform>(player, 0.0f, 0.0f, 32.0f, 32.0f, true);
  registry.emplace<Collider>(player, 0.0f, 0.0f, 15.0f, 15.0f, Offset{17, 17});
  registry.emplace<KeyboardController>(player);

  // Set up NPCs
  npc = registry.create();
  registry.emplace<Sprite>(npc, npcSpriteSheet.c_str(), PLAYER_WIDTH,
                           PLAYER_HEIGHT);
  registry.emplace<Transform>(npc, 208.0f, 112.0f, 32.0f, 32.0f);
  registry.emplace<Collider>(npc, 208.0f, 112.0f, 15.0f, 15.0f, Offset{17, 17});
  registry.emplace<Interactable>(npc, 208.0f, 112.0f, 48.0f, 48.0f,
                                 Offset{-16, -16});

  // Set up map data
  mapData = MapLoader::LoadMap(level1Map.c_str());
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
    registry.emplace<Collider>(colliderEntity, collider.xpos, collider.ypos,
                               collider.width, collider.height);
    registry.emplace<Transform>(colliderEntity, collider.xpos, collider.ypos,
                                collider.width, collider.height);
  }

  // Set up the up the UI manager
  uiManager = new UIManager();

  return true;
}

void Game::handleEvents(SDL_Event *event) {
  auto &controller = registry.get<KeyboardController>(player);
  auto &transform = registry.get<Transform>(player);
  auto &sprite = registry.get<Sprite>(player);

  // Iterate through interactable components to check
  // if any can be interacted with
  auto interactView = registry.view<Interactable>();
  Interactable *intObject = nullptr;
  for (auto intEntity : interactView) {
    auto &interactable = interactView.get<Interactable>(intEntity);
    if (interactable.canInteract) {
      intObject = &interactable;
      break; // only one object should be interactable at any one time
    }
  }
  controller.update(event, transform, sprite, intObject);
  uiManager->update(*event);
}

void Game::updateCamera() {
  // Update camera position based on player position
  auto view = registry.view<Transform>();
  auto &playerTransform = view.get<Transform>(player);
  int xpos =
      static_cast<int>(playerTransform.position.x + float(PLAYER_WIDTH) / 2.0f -
                       float(SCREEN_WIDTH) / 2.0f);
  int ypos = static_cast<int>(playerTransform.position.y +
                              float(PLAYER_HEIGHT) / 2.0f -
                              float(SCREEN_HEIGHT) / 2.0f);
  Camera::update(xpos, ypos, mapPixelWidth, mapPixelHeight);
}

void Game::update() {
  // TODO: Fix player getting stuck on level geometry
  // TODO: Fix jerky movement when moving towards a collision object

  // Get player collider and transform components
  auto view = registry.view<Sprite, Transform, Collider>();
  auto &playerCollider = view.get<Collider>(player);
  auto &playerTransform = view.get<Transform>(player);

  // Update all colliders
  auto colliderView = registry.view<Collider, Transform>();
  for (auto entity : colliderView) {
    auto &collider = colliderView.get<Collider>(entity);
    auto &transform = colliderView.get<Transform>(entity);
    transform.update();
    if (entity != player && Collision::AABB(playerCollider, collider)) {
      std::cout << "Player collision!" << std::endl;
      playerTransform.abortMove();
    }
    collider.update(transform);
  }

  // Update all Interactable
  auto interactView = registry.view<Interactable, Transform>();
  for (auto entity : interactView) {
    auto &interact = interactView.get<Interactable>(entity);
    auto &transform = interactView.get<Transform>(entity);
    interact.canInteract = false;
    if (entity != player &&
        Collision::AABB(playerCollider.collider, interact.interactArea)) {
      interact.canInteract = true;
    } else {
      interact.canInteract = false;
    }
    interact.update(transform);
  }

  // Update all sprites
  auto spriteView = registry.view<Sprite, Transform>();
  for (auto entity : spriteView) {
    auto &sprite = spriteView.get<Sprite>(entity);
    auto &transform = spriteView.get<Transform>(entity);
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

  uiManager->render(renderer);

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

  delete uiManager;

  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}
