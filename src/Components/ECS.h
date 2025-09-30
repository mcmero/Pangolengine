#pragma once

#include <bitset>
#include <memory>
#include <array>
#include <unordered_map>
#include <vector>

constexpr std::size_t maxComponents = 32;

class IComponent {
public:
	virtual void update() {}
	virtual void render() {}
  virtual ~IComponent() = default;
};

using EntityId = std::size_t;

struct Entity {
  std::array<std::unique_ptr<IComponent>, maxComponents> componentArray = {};
  std::bitset<maxComponents> componentBitset = {};
};

class EntityRegistry {
public:
  EntityRegistry() {
    lastId = 0;
  };

  /*
   * Add an entity to the manager and return its ID
   */
  EntityId create() {
    lastId++;
    entityMap[lastId] = std::unique_ptr<Entity>(new Entity{});
    return lastId;
  };

  /*
   * Add component to entity and initialise
   */
  template<typename T, typename... TArgs>
  T& addComponent(EntityId entityId, TArgs&&... mArgs) {};

  /*
   * Get all components of type T and return vector of entity IDs
   * that have those components.
   */
  template<typename T, typename... TArgs>
  std::vector<EntityId> getEntitiesWithComponents() {}

  /*
   * Remove the entity from the registry.
   */
  void remove(Entity &entity) {}

  /*
  * Clear all entities in the registry
  */
  void clear() {};

  ~EntityRegistry() = default;

private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> entityMap;
  std::size_t lastId;
};
