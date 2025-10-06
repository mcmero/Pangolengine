#include "Game.h"
#include "Camera.h"
#include "Collision.h"
#include "Components/ECS.h"
#include "Components/Components.h"
#include "Components/KeyboardController.h"
#include "Constants.h"
#include "MapLoader.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "UI/UIManager.h"
#include <exception>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>

namespace fs = std::filesystem;

SDL_Renderer *Game::renderer = nullptr;

EntityId Game::playerId = 0;
EntityId Game::mapId = 0;

std::unordered_map<int, EntityId> Game::mapEntities = {};

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

  std::string entryMap = (assetsPath / "maps" / ENTRY_MAP).string();

  // Set up map data
  Game::loadMap(entryMap);

  // Set up player character
  Game::loadPlayer();

  // Set up the up the UI manager
  uiManager = new UIManager();

  // Load music
  // std::string level1Music = (assetsPath / "audio" / "walking.ogg").string();
  // auto music = Mix_LoadMUS(level1Music.c_str());
  // Mix_PlayMusic(music, 2);

  // TESTING of ECS here
  /*
  EntityRegistry entityRegistry = {};
  EntityId entity = entityRegistry.create();
  std::cout << "Created a new entity with ID: " << entity << std::endl;

  Transform transform = Transform(0.0f, 0.0f, 0.0f, 0.0f);
  entityRegistry.addComponent<Collider>(entity, 0.0f, 0.0f, 0.0f, 0.0f, transform);

  auto &collider = entityRegistry.getComponent<Collider>(entity);
  auto ewc = entityRegistry.getEntitiesWithComponents<Collider>();
  entityRegistry.removeComponent<Collider>(entity);
  */

  return true;
}

void Game::handleEvents(SDL_Event *event) {
  auto &controller = registry.getComponent<KeyboardController>(playerId);
  auto &transform = registry.getComponent<Transform>(playerId);
  auto &sprite = registry.getComponent<Sprite>(playerId);

  // Iterate through interactable components to check
  // if any can be interacted with
  auto interactgetEntitiesWithComponents = registry.getEntitiesWithComponents<Interactable>();
  Interactable *intObject = nullptr;
  for (auto intEntity : interactgetEntitiesWithComponents) {
    auto &interactable = registry.getComponent<Interactable>(intEntity);
    if (interactable.canInteract) {
      intObject = &interactable;
      break; // only one object should be interactable at any one time
    }
  }

  uiManager->handleEvents(*event);

  // Handle player interaction events
  controller.update(event, uiManager->isMenuActive(), transform, sprite,
                    intObject);
}

void Game::updateCamera() {
  // Update camera position based on player position
  auto getEntitiesWithComponents = registry.getEntitiesWithComponents<Transform>();
  auto &playerTransform = registry.getComponent<Transform>(playerId);
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
  auto playergetEntitiesWithComponents =
      registry.getEntitiesWithComponents<Sprite, Transform, Collider, KeyboardController>();
  auto &playerCollider = registry.getComponent<Collider>(playerId);
  auto &playerTransform = registry.getComponent<Transform>(playerId);
  auto &playerController = registry.getComponent<KeyboardController>(playerId);
  auto &playerSprite = registry.getComponent<Sprite>(playerId);

  // Update all colliders
  auto collidergetEntitiesWithComponents = registry.getEntitiesWithComponents<Collider, Transform>();
  for (auto entity : collidergetEntitiesWithComponents) {
    auto &collider = registry.getComponent<Collider>(entity);
    auto &transform = registry.getComponent<Transform>(entity);
    transform.update();
    collider.update();
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

    for (auto entity : collidergetEntitiesWithComponents) {
      if (entity == playerId)
        continue;

      auto &collider = registry.getComponent<Collider>(entity);
      if (Collision::AABB(futureCollider, collider.collider)) {
        std::cout << "Player collision!" << std::endl;
        playerTransform.abortMove();
        break;
      }
    }
  }

  // Update all Interactable
  auto interactgetEntitiesWithComponents = registry.getEntitiesWithComponents<Interactable, Transform>();
  Interactable *intObject = nullptr;
  Dialogue *dialogue = nullptr;
  for (auto entity : interactgetEntitiesWithComponents) {
    auto &interact = registry.getComponent<Interactable>(entity);
    auto &transform = registry.getComponent<Transform>(entity);
    interact.canInteract = false;
    if (entity != playerId &&
        Collision::AABB(playerCollider.collider, interact.interactArea)) {
      interact.canInteract = true;
      intObject = &interact;
      try {
        // TODO: better way to handle this?
        dialogue = &registry.getComponent<Dialogue>(entity);
      } catch(const std::exception &e) {
        std::cout << "Could not get dialogue " << e.what() << std::endl;
      }
    } else {
      interact.canInteract = false;
    }
    transform.update();
    interact.update(transform);
  }

  // Update all sprites
  auto spriteEntities = registry.getEntitiesWithComponents<Sprite, Transform>();
  for (auto entity : spriteEntities) {
    auto &sprite = registry.getComponent<Sprite>(entity);
    auto &transform = registry.getComponent<Transform>(entity);
    sprite.update(transform);
  }

  // Update maps
  auto mapEntities = registry.getEntitiesWithComponents<Map>();
  for (auto mapEntity : mapEntities) {
    auto &map = registry.getComponent<Map>(mapEntity);
    map.update();
  }

  // Update all transitions
  auto transitiongetEntitiesWithComponents = registry.getEntitiesWithComponents<Transition, Transform>();
  for (auto entity : transitiongetEntitiesWithComponents) {
    auto &transition = registry.getComponent<Transition>(entity);
    auto &transform = registry.getComponent<Transform>(entity);

    transform.update();
    transition.update(transform);

    if (Collision::AABB(playerCollider.collider, transition.collider)) {
      std::cout << "Trigger transition!" << std::endl;
      std::string mapPath = transition.mapPath;

      Game::unloadMap();
      Game::loadMap(mapPath);

      registry.destroy(playerId);
      Game::loadPlayer();

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
  // Set screen to black and clear renderer
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // draw map
  std::vector<EntityId> mapEntities = registry.getEntitiesWithComponents<Map>();
  for (auto mapEntity : mapEntities) {
    auto &map = registry.getComponent<Map>(mapEntity);
    map.render();
  }

  // Build a vector of sprites, sorted by Y coordinate
  std::vector<std::pair<float, EntityId>> entityDrawOrder = {};
  auto spriteEntities = registry.getEntitiesWithComponents<Sprite, Transform>();
  for (auto entity : spriteEntities) {
    auto &transform = registry.getComponent<Transform>(entity);

    entityDrawOrder.push_back({
      transform.position.y + transform.height,
      entity
    });

    // Sort vector by Y coordinate
    std::sort(entityDrawOrder.begin(), entityDrawOrder.end(),
              [](const auto a,
                       const auto b) {
                return a.first < b.first;
              });
  }

  // Render the sprites based on draw order (topdown assumed)
  for (auto entityOrderEntry : entityDrawOrder) {
    auto &sprite = registry.getComponent<Sprite>(entityOrderEntry.second);
    sprite.render();
  }

  // Render colliders -- this is only for debugging
  if (RENDER_COLLIDERS) {
    auto colliderEntities = registry.getEntitiesWithComponents<Collider>();
    for (auto entity : colliderEntities) {
      auto &collider = registry.getComponent<Collider>(entity);
      collider.render();
    }
  }

  uiManager->render(renderer, window);

  SDL_RenderPresent(renderer);
}

void Game::clean() {
  Game::unloadMap();

  registry.clear();

  // No need to destroy window and renderer as they are managed outside
  SDL_Quit();
}

void Game::unloadMap() {
  clearEntities(mapEntities);
  registry.destroy(mapId);
}

template <typename T>
void Game::clearEntities(std::unordered_map<int, T> entityVector) {
  for (auto &entityEntry : entityVector) {
    EntityId &entity = entityEntry.second;
    registry.destroy(entity);
  }
  entityVector.clear();
}

void Game::loadPlayer() {
  playerId = registry.create();

  registry.addComponent<Sprite>(
      playerId, mapData.playerObject.spriteSheet.c_str(),
      mapData.playerObject.width, mapData.playerObject.height,
      mapData.playerObject.spriteOffset, mapData.playerObject.animations);

  registry.addComponent<Transform>(playerId, mapData.startPos.x, mapData.startPos.y,
                              mapData.playerObject.width,
                              mapData.playerObject.height, true);

  auto getEntitiesWithComponents = registry.getEntitiesWithComponents<Transform>();
  auto &transform = registry.getComponent<Transform>(playerId);
  registry.addComponent<Collider>(playerId, transform.position.x, transform.position.y,
                             mapData.playerObject.collider.width,
                             mapData.playerObject.collider.height,
                             transform,
                             Offset{mapData.playerObject.collider.xpos +
                                        mapData.playerObject.spriteOffset.x,
                                    mapData.playerObject.collider.ypos +
                                        mapData.playerObject.spriteOffset.y});

  registry.addComponent<KeyboardController>(playerId);
}

void Game::loadMap(std::string mapPath) {
  // Get map data
  MapLoader mapLoader = MapLoader(mapPath, TILE_SIZE);
  mapData = mapLoader.LoadMap();

  mapId = registry.create();
  registry.addComponent<Map>(mapId, &mapData, mapData.tilesetImg.c_str(), TILE_SIZE);

  // Create entities from sprites first
  for (auto &spriteObject : mapData.spriteVector) {
    EntityId spriteEntity = registry.create();

    MapObject &sprite = spriteObject.second;
    registry.addComponent<Sprite>(spriteEntity, sprite.filePath.c_str(),
                             sprite.width, sprite.height, Offset{0, 0},
                             std::vector<Animation>{}, sprite.drawOrderId);
    registry.addComponent<Transform>(spriteEntity, sprite.xpos, sprite.ypos,
                                sprite.width, sprite.height);

    mapEntities[spriteObject.first] = spriteEntity;
  }

  // Process sprite colliders
  for (auto &colliderObject : mapData.spriteColliderVector) {
    MapObject &collider = colliderObject.second;

    // Skip this collider if it doesn't have a corresponding sprite
    if (!mapEntities.contains(collider.objectId))
      continue;

    // Get sprite entity -- it has the same ID as the collider
    EntityId spriteEntity = 0;
    spriteEntity = mapEntities[collider.objectId];

    // Fetch the corresponding Transform component
    auto getEntitiesWithComponents = registry.getEntitiesWithComponents<Transform>();
    auto &transform = registry.getComponent<Transform>(spriteEntity);

    // Calculate offsets here because the collider may move
    Offset offset = {collider.xpos - transform.position.x,
                      collider.ypos - transform.position.y};
    registry.addComponent<Collider>(spriteEntity, transform.position.x,
                                transform.position.y, collider.width,
                                collider.height, transform, offset);
  }

  // Process colliders, adding to existing sprite if they are linked
  for (auto &colliderObject : mapData.colliderVector) {
    EntityId colliderEntity = 0;
    MapObject &collider = colliderObject.second;

    // Add the collider to linked (sprite) entity if it exists
    // otherwise create a new entity
    if (collider.linkedId > 0 && mapEntities.contains(collider.linkedId)) {
      colliderEntity = mapEntities[collider.linkedId]; // This is actually the sprite entity

      // Fetch the corresponding Transform component
      auto getEntitiesWithComponents = registry.getEntitiesWithComponents<Transform>();
      auto &transform = registry.getComponent<Transform>(colliderEntity);

      // Calculate offsets here because the collider may move
      Offset offset = {collider.xpos - transform.position.x,
                       collider.ypos - transform.position.y};
      registry.addComponent<Collider>(colliderEntity, transform.position.x,
                                 transform.position.y, collider.width,
                                 collider.height, transform, offset);
    } else {
      // Non-linked static collider, treat as its own entity
      colliderEntity = registry.create();
      registry.addComponent<Transform>(colliderEntity, collider.xpos, collider.ypos,
                                  collider.width, collider.height);

      // Fetch a reference to the transform created above
      auto getEntitiesWithComponents = registry.getEntitiesWithComponents<Transform>();
      auto &transform = registry.getComponent<Transform>(colliderEntity);

      registry.addComponent<Collider>(colliderEntity, collider.xpos, collider.ypos,
                                 collider.width, collider.height, transform);

      mapEntities[colliderObject.first] = colliderEntity;
    }
  }

  // Process transitions -- they are not linked to anything (yet...)
  for (auto &transitionObject : mapData.transitionVector) {
    EntityId transitionEntity = registry.create();
    MapObject &transition = transitionObject.second;

    auto &transform = registry.addComponent<Transform>(
        transitionEntity, transition.xpos, transition.ypos, transition.width,
        transition.height);

    registry.addComponent<Transition>(transitionEntity, transform,
                                 transition.filePath);

    mapEntities[transitionObject.first] = transitionEntity;
  }

  // Process interaction objects and link to existing sprite
  for (auto &interactionObject : mapData.interactionVector) {
    EntityId interactionEntity = 0;
    MapObject &interaction = interactionObject.second;

    // Add the interaction to linked (sprite) entity if it exists
    // otherwise ignore it
    if (interaction.linkedId > 0 &&
        mapEntities.contains(interaction.linkedId)) {
      interactionEntity = mapEntities[interaction.linkedId];

      // Fetch the corresponding Transform component
      auto getEntitiesWithComponents = registry.getEntitiesWithComponents<Transform>();
      auto &transform = registry.getComponent<Transform>(interactionEntity);

      // Calculate offsets here because the interaction may move
      Offset offset = {interaction.xpos - transform.position.x,
                       interaction.ypos - transform.position.y};
      registry.addComponent<Interactable>(interactionEntity, transform.position.x,
                                          transform.position.y, interaction.width,
                                          interaction.height, offset);

      // Add dialogue file if there is one
      if (!interaction.filePath.empty())
        registry.addComponent<Dialogue>(interactionEntity,
                                        interaction.filePath.c_str());
    } else {
      std::cerr << "No linked ID found for interaction object ID: "
                << interaction.objectId << std::endl;
    }
  }
}
