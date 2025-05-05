#include "Game.h"
#include "Camera.h"
#include "Collision.h"
#include "Components/Components.h"
#include "Constants.h"
#include "MapLoader.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "UI/UIManager.h"
#include <filesystem>
#include <iostream>
#include <ostream>

namespace fs = std::filesystem;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;

entt::entity Game::player = entt::null;
entt::entity Game::npc = entt::null;
entt::entity Game::map = entt::null;

std::vector<entt::entity> Game::mapSprites = {};
std::vector<entt::entity> Game::mapColliders = {};
std::vector<entt::entity> Game::mapTransitions = {};

MapData Game::mapData;

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
  std::string s001_dialogue =
      (assetsPath / "scenes" / "S001_Test.json").string();

  // Set up NPCs
  // TODO: NPCs should be part of the map components?
  npc = registry.create();
  registry.emplace<Sprite>(npc, npcSpriteSheet.c_str(), PLAYER_WIDTH,
                           PLAYER_HEIGHT, Offset{8, 0});
  registry.emplace<Transform>(npc, 192.0f, 128.0f, 32.0f, 32.0f);
  registry.emplace<Collider>(npc, 192.0f, 128.0f, 15.0f, 15.0f, Offset{17, 17});
  registry.emplace<Interactable>(npc, 192.0f, 128.0f, 48.0f, 48.0f,
                                 Offset{-16, -16});
  registry.emplace<Dialogue>(npc, s001_dialogue.c_str());

  // Set up map data
  Game::loadMap(level1Map);

  // Set up player character
  Game::loadPlayer();

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
  uiManager->handleEvents(*event);
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
  Camera::update(xpos, ypos, mapData.pixelWidth, mapData.pixelHeight);
}

void Game::update() {
  // TODO: Use templates
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
  Interactable *intObject = nullptr;
  Dialogue *dialogue = nullptr;
  for (auto entity : interactView) {
    auto &interact = interactView.get<Interactable>(entity);
    auto &transform = interactView.get<Transform>(entity);
    interact.canInteract = false;
    if (entity != player &&
        Collision::AABB(playerCollider.collider, interact.interactArea)) {
      interact.canInteract = true;
      intObject = &interact;
      dialogue = registry.try_get<Dialogue>(entity);
    } else {
      interact.canInteract = false;
    }
    interact.update(transform);
  }

  // Update all sprites
  // TODO: draw sprite order based on Y axis position
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

  // Update all transitions
  auto transitionView = registry.view<Transition, Transform>();
  for (auto entity : transitionView) {
    auto &transition = transitionView.get<Transition>(entity);
    auto &transform = transitionView.get<Transform>(entity);

    transform.update();
    transition.update(transform);

    if (Collision::AABB(playerCollider.collider, transition.collider)) {
      std::cout << "Trigger transition!" << std::endl;
      std::string mapPath = transition.mapPath;

      Game::unloadMap();
      Game::loadMap(mapPath);

      // Replace transform and collider components on the player to reset
      // position
      registry.replace<Transform>(player, mapData.startPos.x,
                                  mapData.startPos.y, 32.0f, 32.0f, true);
      registry.replace<Collider>(player, mapData.startPos.x, mapData.startPos.y,
                                 15.0f, 15.0f, Offset{17, 17});

      break;
    }
  }

  uiManager->update(intObject, dialogue);
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
  Game::unloadMap();

  registry.clear();

  delete uiManager;

  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}

void Game::unloadMap() {
  clearEntities(mapSprites);
  clearEntities(mapColliders);
  clearEntities(mapTransitions);

  if (registry.valid(map)) {
    registry.destroy(map);
  }
}

template <typename T> void Game::clearEntities(std::vector<T> entityVector) {
  for (auto entity : entityVector) {
    if (registry.valid(entity)) {
      registry.destroy(entity);
    }
  }
  entityVector.clear();
}

void Game::loadPlayer() {
  player = registry.create();
  // TODO: the animation is a bit jerky when walking -- can it be fixed by
  // animation speed?
  // TODO: initialise collision component using transform values
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  std::string playerSpriteSheet =
      (assetsPath / "characters" / "player_anim.png").string();
  std::vector<Animation> playerAnims = {{"walk_front", 0, 4, 200},
                                        {"walk_side", 1, 4, 200},
                                        {"walk_back", 2, 4, 200}};
  registry.emplace<Sprite>(player, playerSpriteSheet.c_str(), PLAYER_WIDTH,
                           PLAYER_HEIGHT, Offset{8, 0}, playerAnims);
  registry.emplace<Transform>(player, mapData.startPos.x, mapData.startPos.y,
                              32.0f, 32.0f, true);
  registry.emplace<Collider>(player, mapData.startPos.x, mapData.startPos.y,
                             15.0f, 15.0f, Offset{17, 17});
  registry.emplace<KeyboardController>(player);
}

void Game::loadMap(std::string mapPath) {
  // Get map data
  MapLoader mapLoader = MapLoader(mapPath, TILE_SIZE);
  mapData = mapLoader.LoadMap();

  map = registry.create();
  registry.emplace<Map>(map, &mapData, mapData.tilesetImg.c_str(), TILE_SIZE);

  // TODO: make these into template functions

  // Set up map sprites
  for (auto sprite : mapData.spriteVector) {
    entt::entity spriteEntity = registry.create();
    mapSprites.push_back(spriteEntity);
    registry.emplace<Sprite>(spriteEntity, sprite.filePath.c_str(),
                             sprite.width, sprite.height);
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

  // Set up map transitions
  for (auto transition : mapData.transitionVector) {
    entt::entity transitionEntity = registry.create();
    mapTransitions.push_back(transitionEntity);
    registry.emplace<Transform>(transitionEntity, transition.xpos,
                                transition.ypos, transition.width,
                                transition.height);

    auto &transform = registry.get<Transform>(transitionEntity);
    registry.emplace<Transition>(transitionEntity, transform,
                                 transition.filePath);
  }
}
