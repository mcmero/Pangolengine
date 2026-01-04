#include "DemoGame.h"
#include "Engine.h"
#include "Camera.h"
#include "Collision.h"
#include "Components/Components.h"
#include "Components/KeyboardController.h"
#include "Constants.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3_mixer/SDL_mixer.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

DemoGame::DemoGame(Engine* engine)
  : engine(engine), playerId(0), mapId(0) {}

DemoGame::~DemoGame() {
  onCleanup();
}

bool DemoGame::onInitialise() {
  SDL_Log("Demo game initializing...");

  // Define paths to demo assets
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  std::string entryMap = (assetsPath / "maps" / ENTRY_MAP).string();

  // Set up map data
  loadDemoMap();

  // Set up player character
  loadPlayer();

  // Set up the UI manager (accessed via engine)
  engine->uiManager = new UIManager();

  // Load demo music
  std::string demoMusic = (assetsPath / "audio" / "walking.ogg").string();
  auto music = Mix_LoadMUS(demoMusic.c_str());
  Mix_PlayMusic(music, -1); // Loop indefinitely
  Mix_VolumeMusic(
    static_cast<int>(std::round(MIX_MAX_VOLUME * 0.5f)) // half volume
  );

  SDL_Log("Demo game initialized successfully!");
  return true;
}

void DemoGame::onEvent(SDL_Event* event) {
  auto& registry = engine->getRegistry();
  auto& controller = registry.getComponent<KeyboardController>(playerId);
  auto& transform = registry.getComponent<Transform>(playerId);
  auto& sprite = registry.getComponent<Sprite>(playerId);

  // Iterate through interactable components to check
  // if any can be interacted with
  auto interactEntities = registry.getEntitiesWithComponents<Interactable>();
  Interactable* intObject = nullptr;
  for (auto intEntity : interactEntities) {
    auto& interactable = registry.getComponent<Interactable>(intEntity);
    if (interactable.canInteract) {
      intObject = &interactable;
      break; // only one object should be interactable at any one time
    }
  }

  engine->uiManager->handleEvents(*event);

  // Handle player interaction events
  controller.update(event, engine->uiManager->isMenuActive(), transform, sprite, intObject);
}

void DemoGame::onUpdate() {
  auto& registry = engine->getRegistry();

  // Get player collider and transform components
  auto& playerCollider = registry.getComponent<Collider>(playerId);
  auto& playerTransform = registry.getComponent<Transform>(playerId);
  auto& playerController = registry.getComponent<KeyboardController>(playerId);
  auto& playerSprite = registry.getComponent<Sprite>(playerId);

  // Update all colliders
  auto colliderEntities = registry.getEntitiesWithComponents<Collider, Transform>();
  for (auto entity : colliderEntities) {
    auto& collider = registry. getComponent<Collider>(entity);
    auto& transform = registry.getComponent<Transform>(entity);
    transform.update();
    collider.update(transform);
  }

  // Check for collision with player (if moving) and abort move on collision
  if (playerTransform.isMoving) {
    // Make a collider where the player will be at the end of the move
    SDL_FRect futureCollider;
    futureCollider.x = playerTransform.targetPosition.x + playerCollider.offset.x;
    futureCollider.y = playerTransform.targetPosition.y + playerCollider.offset.y;
    futureCollider.h = playerCollider.collider. h;
    futureCollider.w = playerCollider.collider.w;

    for (auto entity : colliderEntities) {
      if (entity == playerId)
        continue;

      auto& collider = registry.getComponent<Collider>(entity);
      if (Collision::AABB(futureCollider, collider.collider)) {
        std::cout << "Player collision!" << std::endl;
        playerTransform.abortMove();
        break;
      }
    }
  }

  // Update all Interactable
  auto interactEntities = registry.getEntitiesWithComponents<Interactable, Transform>();
  Interactable* intObject = nullptr;
  Dialogue* dialogue = nullptr;
  for (auto entity : interactEntities) {
    auto& interact = registry. getComponent<Interactable>(entity);
    auto& transform = registry.getComponent<Transform>(entity);
    interact.canInteract = false;
    if (entity != playerId &&
      Collision::AABB(playerCollider.collider, interact. interactArea)) {
      interact.canInteract = true;
      intObject = &interact;
      dialogue = registry.tryGetComponent<Dialogue>(entity);
      if (! dialogue)
        throw std::runtime_error(
          "No dialogue component found for interaction entity"
        );
    } else {
      interact. canInteract = false;
    }
    transform.update();
    interact.update(transform);
  }

  // Update all sprites
  auto spriteEntities = registry.getEntitiesWithComponents<Sprite, Transform>();
  for (auto entity : spriteEntities) {
    auto& sprite = registry.getComponent<Sprite>(entity);
    auto& transform = registry.getComponent<Transform>(entity);
    sprite.update(transform);
  }

  // Update maps
  auto mapEntities = registry.getEntitiesWithComponents<Map>();
  for (auto mapEntity : mapEntities) {
    auto& map = registry.getComponent<Map>(mapEntity);
    map.update();
  }

  // Update all transitions
  auto transitionEntities = registry. getEntitiesWithComponents<Transition, Transform>();
  for (auto entity : transitionEntities) {
    auto& transition = registry.getComponent<Transition>(entity);
    auto& transform = registry.getComponent<Transform>(entity);

    transform. update();
    transition.update(transform);

    if (Collision::AABB(playerCollider.collider, transition.collider)) {
      std::cout << "Trigger transition!" << std::endl;
      std::string mapPath = transition.mapPath;

      // Play transition sound (if there is one)
      if (transition.sound)
        Mix_PlayChannel(-1, transition.sound, 0);

      unloadMap();
      loadDemoMap(mapPath);

      registry.getComponent<Sprite>(playerId).clean();
      registry.destroy(playerId);
      loadPlayer();

      break;
    }
  }

  // Handle player movement via polling for smooth movement
  const bool* keyState = SDL_GetKeyboardState(nullptr);
  playerController.pollInput(keyState, playerTransform, playerSprite);

  engine->uiManager->update(intObject, dialogue);
  updateCamera();

  // Check whether exit was requested by in menu
  if (engine->uiManager->getRequestExit())
    engine->quit();
}

void DemoGame::onRender() {
  auto& registry = engine->getRegistry();
  SDL_Renderer* renderer = engine->getRenderer();
  SDL_Window* window = engine->getWindow();

  // Set screen to black and clear renderer
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // Draw map
  std::vector<EntityId> mapEntities = registry.getEntitiesWithComponents<Map>();
  for (auto mapEntity : mapEntities) {
    auto& map = registry.getComponent<Map>(mapEntity);
    map.render();
  }

  // Build a vector of sprites, sorted by Y coordinate
  std::vector<std::pair<float, EntityId>> entityDrawOrder = {};
  auto spriteEntities = registry.getEntitiesWithComponents<Sprite, Transform>();
  for (auto entity : spriteEntities) {
    auto& transform = registry.getComponent<Transform>(entity);

    entityDrawOrder.push_back({
      transform.position.y + transform.height,
      entity
    });

    // Sort vector by Y coordinate
    std::sort(
      entityDrawOrder.begin(), entityDrawOrder.end(),
      [](const auto a, const auto b) {
        return a.first < b.first;
      }
    );
  }

  // Render the sprites based on draw order (topdown assumed)
  for (auto entityOrderEntry : entityDrawOrder) {
    auto& sprite = registry.getComponent<Sprite>(entityOrderEntry.second);
    sprite.render();
  }

  // Render colliders -- this is only for debugging
  if (RENDER_COLLIDERS) {
    auto colliderEntities = registry. getEntitiesWithComponents<Collider>();
    for (auto entity : colliderEntities) {
      auto& collider = registry.getComponent<Collider>(entity);
      collider.render();
    }
  }

  engine->uiManager->render(renderer, window);

  SDL_RenderPresent(renderer);
}

void DemoGame:: onCleanup() {
  auto& registry = engine->getRegistry();

  unloadMap();

  // Clean player sprite -- other sprites are cleared by unloadMap()
  Sprite* sprite = registry.tryGetComponent<Sprite>(playerId);
  if (sprite)
    sprite->clean();

  registry.clear();

  SDL_Log("Demo game cleaned up!");
}

void DemoGame::updateCamera() {
  auto& registry = engine->getRegistry();
  auto& playerTransform = registry.getComponent<Transform>(playerId);

  int xpos = static_cast<int>(playerTransform.position.x +
                float(Engine::mapData.playerObject.width) / 2.0f -
                float(SCREEN_WIDTH) / 2.0f);
  int ypos = static_cast<int>(playerTransform.position.y +
                float(Engine::mapData.playerObject. height) / 2.0f -
                float(SCREEN_HEIGHT) / 2.0f);
  Camera:: update(xpos, ypos, Engine::mapData.pixelWidth, Engine::mapData.pixelHeight);
}

void DemoGame::loadPlayer() {
  auto& registry = engine->getRegistry();

  playerId = registry.create();

  registry.addComponent<Sprite>(
    playerId, Engine::mapData.playerObject.spriteSheet. c_str(),
    Engine::mapData.playerObject.width, Engine::mapData.playerObject.height,
    Engine::mapData. playerObject.spriteOffset, Engine::mapData.playerObject. animations);

  registry.addComponent<Transform>(playerId, Engine::mapData.startPos.x, Engine::mapData.startPos. y,
                Engine::mapData.playerObject.width,
                Engine::mapData.playerObject.height, true);

  auto& transform = registry.getComponent<Transform>(playerId);
  registry.addComponent<Collider>(playerId, transform.position.x, transform.position.y,
                 Engine::mapData.playerObject. collider.width,
                 Engine::mapData.playerObject.collider.height,
                 transform,
                 Offset{Engine::mapData.playerObject.collider.xpos +
                      Engine::mapData.playerObject. spriteOffset.x,
                    Engine::mapData.playerObject. collider.ypos +
                      Engine::mapData.playerObject.spriteOffset.y});

  registry.addComponent<KeyboardController>(playerId);
}

void DemoGame::loadDemoMap(const std::string& mapPath) {
  auto& registry = engine->getRegistry();

  // Define paths to demo assets
  fs:: path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  std::string entryMap = (assetsPath / "maps" / ENTRY_MAP).string();

  // Use provided map path, or default to ENTRY_MAP
  std::string mapToLoad;
  if (mapPath.empty()) {
    mapToLoad = (assetsPath / "maps" / ENTRY_MAP).string();
  } else {
    // mapPath from transition is already relative to assets
    mapToLoad = (assetsPath / mapPath).string();
  }

  // Get map data
  MapLoader mapLoader = MapLoader(mapToLoad, TILE_SIZE);
  Engine::mapData = mapLoader.LoadMap();

  mapId = registry.create();
  registry.addComponent<Map>(mapId, &Engine::mapData, Engine::mapData.tilesetImg. c_str(), TILE_SIZE);

  // Create entities from sprites first
  for (auto& spriteObject : Engine::mapData.spriteVector) {
    EntityId spriteEntity = registry.create();

    MapObject& sprite = spriteObject.second;
    registry.addComponent<Sprite>(spriteEntity,
                    sprite.properties["file_path"].c_str(),
                    sprite.width, sprite.height, Offset{0, 0},
                    std::vector<Animation>{}, sprite.drawOrderId);
    registry.addComponent<Transform>(spriteEntity, sprite.xpos, sprite.ypos,
                  sprite.width, sprite.height);

    mapEntities[spriteObject.first] = spriteEntity;
  }

  // Process sprite colliders
  for (auto& colliderObject : Engine::mapData. spriteColliderVector) {
    MapObject& collider = colliderObject.second;

    // Skip this collider if it doesn't have a corresponding sprite
    if (!mapEntities.contains(collider.objectId))
      continue;

    // Get sprite entity -- it has the same ID as the collider
    EntityId spriteEntity = mapEntities[collider.objectId];

    // Fetch the corresponding Transform component
    auto& transform = registry.getComponent<Transform>(spriteEntity);

    // Calculate offsets here because the collider may move
    Offset offset = {collider.xpos - transform.position.x,
             collider.ypos - transform. position.y};
    registry. addComponent<Collider>(spriteEntity, transform.position.x,
                   transform.position.y, collider.width,
                   collider.height, transform, offset);
  }

  // Process colliders, adding to existing sprite if they are linked
  for (auto& colliderObject : Engine::mapData.colliderVector) {
    EntityId colliderEntity = 0;
    MapObject& collider = colliderObject.second;

    // Add the collider to linked (sprite) entity if it exists
    // otherwise create a new entity
    if (collider.linkedId > 0 && mapEntities.contains(collider.linkedId)) {
      colliderEntity = mapEntities[collider.linkedId];

      // Fetch the corresponding Transform component
      auto& transform = registry. getComponent<Transform>(colliderEntity);

      // Calculate offsets here because the collider may move
      Offset offset = {collider. xpos - transform.position.x,
               collider.ypos - transform.position.y};
      registry.addComponent<Collider>(colliderEntity, transform.position.x,
                     transform.position.y, collider.width,
                     collider.height, transform, offset);
    } else {
      // Non-linked static collider, treat as its own entity
      colliderEntity = registry.create();
      registry.addComponent<Transform>(colliderEntity, collider. xpos, collider.ypos,
                    collider.width, collider.height);

      // Fetch a reference to the transform created above
      auto& transform = registry.getComponent<Transform>(colliderEntity);

      registry.addComponent<Collider>(colliderEntity, collider.xpos, collider.ypos,
                     collider.width, collider. height, transform);

      mapEntities[colliderObject.first] = colliderEntity;
    }
  }

  // Process transitions
  for (auto& transitionObject : Engine::mapData.transitionVector) {
    EntityId transitionEntity = registry.create();
    MapObject& transition = transitionObject.second;

    auto& transform = registry.addComponent<Transform>(
      transitionEntity, transition.xpos, transition.ypos, transition.width,
      transition. height);

    Mix_Chunk* transitionSound = nullptr;
    auto transitionProperties = &transitionObject.second.properties;
    if (transitionProperties->contains("sound"))
      transitionSound = Mix_LoadWAV(transitionProperties->at("sound").c_str());

    registry.addComponent<Transition>(transitionEntity, transform,
                      transition.properties["file_path"],
                      transitionSound);

    mapEntities[transitionObject.first] = transitionEntity;
  }

  // Process interaction objects and link to existing sprite
  for (auto& interactionObject : Engine::mapData.interactionVector) {
    EntityId interactionEntity = 0;
    MapObject& interaction = interactionObject.second;

    // Add the interaction to linked (sprite) entity if it exists
    if (interaction.linkedId > 0 && mapEntities.contains(interaction.linkedId)) {
      interactionEntity = mapEntities[interaction.linkedId];

      // Fetch the corresponding Transform component
      auto& transform = registry.getComponent<Transform>(interactionEntity);

      // Calculate offsets here because the interaction may move
      Offset offset = {interaction.xpos - transform.position.x,
               interaction.ypos - transform.position.y};
      registry.addComponent<Interactable>(interactionEntity, transform.position.x,
                        transform.position.y, interaction. width,
                        interaction. height, offset);

      // Add dialogue file if there is one
      if (interaction. properties.contains("file_path"))
        registry.addComponent<Dialogue>(
          interactionEntity,
          interaction.properties["file_path"].c_str()
        );
    } else {
      std::cerr << "No linked ID found for interaction object ID: "
            << interaction.objectId << std::endl;
    }
  }
}

void DemoGame::unloadMap() {
  auto& registry = engine->getRegistry();

  // Clear all map entities
  for (auto& entityEntry : mapEntities) {
    EntityId entityId = entityEntry.second;

    // Clear sprite textures
    Sprite* sprite = registry.tryGetComponent<Sprite>(entityId);
    if (sprite)
      sprite->clean();

    registry.destroy(entityId);
  }
  mapEntities.clear();

  // Clean and destroy the map itself
  Map* map = registry.tryGetComponent<Map>(mapId);
  if (map)
    map->clean();
  registry.destroy(mapId);
}
