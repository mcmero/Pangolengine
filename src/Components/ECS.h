#pragma once

#include <bitset>
#include <cstdint>
#include <map>
#include <memory>
#include <array>
#include <unordered_map>
#include <vector>
#include <cassert>

constexpr std::size_t maxComponents = 32;

class IComponent {
public:
	virtual void update() {}
	virtual void render() {}
  virtual ~IComponent() = default;
};

using EntityId = std::size_t;
using ComponentId = std::uint8_t;

struct Entity {
  std::array<std::unique_ptr<IComponent>, maxComponents> componentArray = {};
  std::bitset<maxComponents> componentBitset = 0;
};

class EntityRegistry {
public:
  EntityRegistry() {};

  /*
   * Add an entity to the manager and return its ID
   */
  EntityId create() {
    lastEntityId++;
    entityMap[lastEntityId] = std::unique_ptr<Entity>(new Entity{});
    return lastEntityId;
  };

  /*
   * Add component to entity and initialise
   */
  template<typename T, typename... TArgs>
  void addComponent(EntityId entityId, TArgs&&... mArgs) {
    // Make sure the entity exists
    assert(entityMap.contains(entityId) && "Entity not found!");

    // Make sure the component inherits from IComponent
    static_assert(std::is_base_of<IComponent, T>::value, "Supplied class is not a component!");

    // Register component in lookup if needed and return component ID
    const std::string typeName = typeid(T).name();
    if (!componentLookup.contains(typeName)) {
      lastComponentId++;
      assert(lastComponentId <= maxComponents && "Too many component types!");
      componentLookup[typeName] = lastComponentId;
    }
    ComponentId cid = componentLookup[typeName];

    Entity* entity = entityMap[entityId].get();

    // An entity cannot have more than one component of the same type
    assert(!entity->componentBitset[cid] &&
           "Component exists for entity!");

    // Set component bit
    entity->componentBitset[cid] = true;

    // Initialise the component and add to the entity's component array
    entity->componentArray[cid] = std::unique_ptr<IComponent>(
        static_cast<IComponent*>(new T(std::forward<TArgs>(mArgs)...))
    );
  };

  /*
   * Return the component associated with the specified entity
   */
  template<typename T>
  T& getComponent(EntityId entityId) {
    // Make sure the entity exists
    assert(entityMap.contains(entityId) && "Entity not found!");

    // Make sure the component exists in the lookup
    const std::string typeName = typeid(T).name();
    asset(componentLookup.contains(typeName) && "Component not found in lookup!");

    // TODO: Return the component for the given entity
  }

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
  std::unordered_map<EntityId, std::unique_ptr<Entity>> entityMap = {};
  std::map<std::string, ComponentId> componentLookup;
  EntityId lastEntityId = 0;
  ComponentId lastComponentId = 0;
};
