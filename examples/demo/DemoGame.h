#pragma once

#include "IGame.h"
#include "Components/ECS.h"
#include "SDL3/SDL_events.h"
#include <SDL3/SDL.h>

// Forward declaration to avoid circular dependency
class Engine;

class DemoGame : public IGame {
public:
  explicit DemoGame(Engine* engine);
  ~DemoGame() override;

  // IGame interface implementation
  bool onInitialise() override;
  void onEvent(SDL_Event* event) override;
  void onUpdate() override;
  void onRender() override;
  void onCleanup() override;

private:
  Engine* engine;

  EntityId playerId;
  EntityId mapId;

  std::unordered_map<int, EntityId> mapEntities;

  void loadPlayer();
  void loadDemoMap(const std::string& mapPath = "");
  void updateCamera();
  void unloadMap();

  template <typename T>
  void clearEntities(std::unordered_map<int, T> entityVector);
};
