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
  T& addComponent(EntityId entityId, TArgs&&... mArgs) {
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

    // Return component pointer - cast back to T* and dereference
    return *static_cast<T*>(entityMap[entityId]->componentArray[cid].get());
  };

  /*
   * Return the component associated with the specified entity
   */
  template<typename T>
  T& getComponent(EntityId entityId) {
    // Make sure the entity exists and has this component
    assert(entityMap.contains(entityId) && "Entity not found!");
    assert(hasComponent<T>(entityId) && "Entity does not have this component!");

    // Get component and return pointer to it
    const std::string typeName = typeid(T).name();
    ComponentId cid = componentLookup[typeName];
    return *static_cast<T*>(entityMap[entityId]->componentArray[cid].get());
  }

  /*
   * Get all components of type T and return vector of entity IDs
   * that have those components.
   */
  template<typename... ComponentTypes>
  std::vector<EntityId> getEntitiesWithComponents() {
    std::vector<EntityId> result = {};

    for (auto& [id, entity] : entityMap) {
        if (entity && hasComponents<ComponentTypes...>(id)) {
            result.push_back(id);
        }
    }

    return result;
  }

  /*
   * Remove the entity from the registry.
   */
  void destroy(EntityId entityId) {
    // Do nothing if entity doesnt' exist
    if (!entityMap.contains(entityId))
      return;

    entityMap.erase(entityId);
  }

  /*
  * Clear all entities in the registry and reset entity ID
  * Components lookup is not cleared to allow reuse
  */
  void clear() {
    entityMap.clear();
    lastEntityId = 0;
  };

  ~EntityRegistry() = default;

private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> entityMap = {};
  std::map<std::string, ComponentId> componentLookup;
  EntityId lastEntityId = 0;
  ComponentId lastComponentId = 0;

  /*
   * Return true if all components of a given type are held
   * by the entity
   */
  template<typename... ComponentTypes>
  bool hasComponents(const EntityId entityId) {
      return (hasComponent<ComponentTypes>(entityId) && ...);
  }

  /*
  * Return true if entity has component and false otherwise.
  * Component must exist in the component lookup.
  */
  template<typename T>
  bool hasComponent(EntityId entityId) {
    const std::string typeName = typeid(T).name();
    assert(componentLookup.contains(typeName) && "Component not found in lookup!");

    ComponentId cid = componentLookup[typeName];
    return entityMap[entityId]->componentBitset[cid];
  }
};
