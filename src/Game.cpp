#include "Game.h"
#include "Camera.h"
#include "Collision.h"
#include "Components/Components.h"
#include "Components/KeyboardController.h"
#include "Constants.h"
#include "MapLoader.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "UI/UIManager.h"
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

SDL_Renderer *Game::renderer = nullptr;

entt::entity Game::player = entt::null;
entt::entity Game::map = entt::null;

std::unordered_map<int, entt::entity> Game::mapEntities = {};

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
  std::string level1Map = (assetsPath / "maps" / "level1.tmj").string();
  std::string s001_dialogue =
      (assetsPath / "scenes" / "S001_Test.json").string();

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

  // Handle player interaction events
  controller.update(event, transform, sprite, intObject);

  uiManager->handleEvents(*event);
}

void Game::updateCamera() {
  // Update camera position based on player position
  auto view = registry.view<Transform>();
  auto &playerTransform = view.get<Transform>(player);
  int xpos = static_cast<int>(playerTransform.position.x +
                              float(mapData.playerObject.width) / 2.0f -
                              float(SCREEN_WIDTH) / 2.0f);
  int ypos = static_cast<int>(playerTransform.position.y +
                              float(mapData.playerObject.height) / 2.0f -
                              float(SCREEN_HEIGHT) / 2.0f);
  Camera::update(xpos, ypos, mapData.pixelWidth, mapData.pixelHeight);
}

void Game::update() {
  // Get player collider and transform components
  auto playerView =
      registry.view<Sprite, Transform, Collider, KeyboardController>();
  auto &playerCollider = playerView.get<Collider>(player);
  auto &playerTransform = playerView.get<Transform>(player);
  auto &playerController = playerView.get<KeyboardController>(player);
  auto &playerSprite = playerView.get<Sprite>(player);

  // Update all colliders
  auto colliderView = registry.view<Collider, Transform>();
  for (auto entity : colliderView) {
    auto &collider = colliderView.get<Collider>(entity);
    auto &transform = colliderView.get<Transform>(entity);
    transform.update();
    collider.update(transform);
  }

  // Check for collision with player (if moving) and abort move on colllision
  if (playerTransform.isMoving) {
    // Make a collider where the player will be at the end of the move
    SDL_FRect futureCollider;
    futureCollider.x =
        playerTransform.targetPosition.x + playerCollider.offset.x;
    futureCollider.y =
        playerTransform.targetPosition.y + playerCollider.offset.y;
    futureCollider.h = playerCollider.collider.h;
    futureCollider.w = playerCollider.collider.w;

    for (auto entity : colliderView) {
      if (entity == player)
        continue;

      auto &collider = colliderView.get<Collider>(entity);
      if (Collision::AABB(futureCollider, collider.collider)) {
        std::cout << "Player collision!" << std::endl;
        playerTransform.abortMove();
        break;
      }
    }
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
    transform.update();
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
      registry.replace<Transform>(
          player, mapData.startPos.x, mapData.startPos.y,
          mapData.playerObject.width, mapData.playerObject.height, true);

      registry.replace<Collider>(player, playerTransform.position.x,
                                 playerTransform.position.y,
                                 mapData.playerObject.collider.width,
                                 mapData.playerObject.collider.height,
                                 Offset{mapData.playerObject.collider.xpos,
                                        mapData.playerObject.collider.ypos});

      break;
    }
  }

  // Handle player movement via polling for smooth movement
  const bool *keyState = SDL_GetKeyboardState(nullptr);
  playerController.pollInput(keyState, playerTransform, playerSprite);

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

  // Build a vector of sprites, sorted by Y coordinate
  std::vector<std::pair<float, entt::entity>> entityDrawOrder = {};
  auto spriteView = registry.view<Sprite, Transform>();
  for (auto entity : spriteView) {
    auto &transform = spriteView.get<Transform>(entity);

    entityDrawOrder.push_back({transform.position.y, entity});

    // Sort vector by Y coordinate
    std::sort(entityDrawOrder.begin(), entityDrawOrder.end(),
              [](const auto a, const auto b) { return a.first < b.first; });
  }

  // Render the sprites based on draw order (topdown assumed)
  for (auto entityOrderEntry : entityDrawOrder) {
    auto &sprite = spriteView.get<Sprite>(entityOrderEntry.second);
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
  clearEntities(mapEntities);

  if (registry.valid(map)) {
    registry.destroy(map);
  }
}

template <typename T>
void Game::clearEntities(std::unordered_map<int, T> entityVector) {
  for (auto &entityEntry : entityVector) {
    entt::entity &entity = entityEntry.second;
    if (registry.valid(entity)) {
      registry.destroy(entity);
    }
  }
  entityVector.clear();
}

void Game::loadPlayer() {
  player = registry.create();

  registry.emplace<Sprite>(
      player, mapData.playerObject.spriteSheet.c_str(),
      mapData.playerObject.width, mapData.playerObject.height,
      mapData.playerObject.spriteOffset, mapData.playerObject.animations);

  registry.emplace<Transform>(player, mapData.startPos.x, mapData.startPos.y,
                              mapData.playerObject.width,
                              mapData.playerObject.height, true);

  auto view = registry.view<Transform>();
  auto &transform = view.get<Transform>(player);
  registry.emplace<Collider>(player, transform.position.x, transform.position.y,
                             mapData.playerObject.collider.width,
                             mapData.playerObject.collider.height,
                             Offset{mapData.playerObject.collider.xpos,
                                    mapData.playerObject.collider.ypos});

  registry.emplace<KeyboardController>(player);
}

void Game::loadMap(std::string mapPath) {
  // Get map data
  MapLoader mapLoader = MapLoader(mapPath, TILE_SIZE);
  mapData = mapLoader.LoadMap();

  map = registry.create();
  registry.emplace<Map>(map, &mapData, mapData.tilesetImg.c_str(), TILE_SIZE);

  // Create entities from sprites first
  for (auto &spriteObject : mapData.spriteVector) {
    entt::entity spriteEntity = registry.create();

    MapObject &sprite = spriteObject.second;
    registry.emplace<Sprite>(spriteEntity, sprite.filePath.c_str(),
                             sprite.width, sprite.height, Offset{0, 0},
                             std::vector<Animation>{}, sprite.drawOrderId);
    registry.emplace<Transform>(spriteEntity, sprite.xpos, sprite.ypos,
                                sprite.width, sprite.height);

    mapEntities[spriteObject.first] = spriteEntity;
  }

  // Process colliders, adding to existing sprite if they are linked
  for (auto &colliderObject : mapData.colliderVector) {
    entt::entity colliderEntity = entt::null;
    MapObject &collider = colliderObject.second;

    // Add the collider to linked (sprite) entity if it exists
    // otherwise create a new entity
    if (collider.linkedId > 0 && mapEntities.contains(collider.linkedId)) {
      colliderEntity = mapEntities[collider.linkedId];

      // Fetch the corresponding Transform component
      auto view = registry.view<Transform>();
      auto &transform = view.get<Transform>(colliderEntity);

      // Calculate offsets here because the collider may move
      Offset offset = {collider.xpos - transform.position.x,
                       collider.ypos - transform.position.y};
      registry.emplace<Collider>(colliderEntity, transform.position.x,
                                 transform.position.y, collider.width,
                                 collider.height, offset);
    } else {
      // Non-linked static collider, treat as its own entity
      colliderEntity = registry.create();
      registry.emplace<Collider>(colliderEntity, collider.xpos, collider.ypos,
                                 collider.width, collider.height);
      registry.emplace<Transform>(colliderEntity, collider.xpos, collider.ypos,
                                  collider.width, collider.height);

      mapEntities[colliderObject.first] = colliderEntity;
    }
  }

  // Process transitions -- they are not linked to anything (yet...)
  for (auto &transitionObject : mapData.transitionVector) {
    entt::entity transitionEntity = registry.create();
    MapObject &transition = transitionObject.second;

    auto &transform = registry.emplace<Transform>(
        transitionEntity, transition.xpos, transition.ypos, transition.width,
        transition.height);

    registry.emplace<Transition>(transitionEntity, transform,
                                 transition.filePath);

    mapEntities[transitionObject.first] = transitionEntity;
  }

  // Process interaction objects and link to existing sprite
  for (auto &interactionObject : mapData.interactionVector) {
    entt::entity interactionEntity = entt::null;
    MapObject &interaction = interactionObject.second;

    // Add the interaction to linked (sprite) entity if it exists
    // otherwise ignore it
    if (interaction.linkedId > 0 &&
        mapEntities.contains(interaction.linkedId)) {
      interactionEntity = mapEntities[interaction.linkedId];

      // Fetch the corresponding Transform component
      auto view = registry.view<Transform>();
      auto &transform = view.get<Transform>(interactionEntity);

      // Calculate offsets here because the interaction may move
      Offset offset = {interaction.xpos - transform.position.x,
                       interaction.ypos - transform.position.y};
      registry.emplace<Interactable>(interactionEntity, transform.position.x,
                                     transform.position.y, interaction.width,
                                     interaction.height, offset);

      // Add dialogue file if there is one
      if (!interaction.filePath.empty())
        registry.emplace<Dialogue>(interactionEntity,
                                   interaction.filePath.c_str());
    } else {
      std::cerr << "No linked ID found for interaction object ID: "
                << interaction.objectId << std::endl;
    }
  }
}
