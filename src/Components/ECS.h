#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <array>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cassert>

constexpr std::size_t maxComponents = 32;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
class IComponent {
public:
	virtual void update() {}
	virtual void render() {}
  virtual ~IComponent() = default;
};

using ComponentId = std::uint8_t;
class ComponentIdGenerator {
private:
    static inline ComponentId nextId = 0;

public:
    template<typename T>
    static ComponentId getId() {
        static ComponentId id = nextId++;
        return id;
    }
};

template<typename T>
ComponentId getComponentId() {
    return ComponentIdGenerator::getId<T>();
}

//------------------------------------------------------------------------------
// Entity
//------------------------------------------------------------------------------
using EntityId = std::size_t;
struct Entity {
  std::array<std::unique_ptr<IComponent>, maxComponents> componentArray = {};
  std::bitset<maxComponents> componentBitset = 0;
};

//------------------------------------------------------------------------------
// Registry
//------------------------------------------------------------------------------
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

    Entity* entity = entityMap[entityId].get();

    // An entity cannot have more than one component of the same type
    ComponentId cid = getComponentId<T>();
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
   * Remove component from entity
   */
  template<typename T, typename... TArgs>
  void removeComponent(EntityId entityId) {
    // Make sure the entity exists
    assert(entityMap.contains(entityId) && "Entity not found!");

    // Make sure the component inherits from IComponent
    static_assert(std::is_base_of<IComponent, T>::value, "Supplied class is not a component!");

    // Get entity pointer and component ID
    Entity* entity = entityMap[entityId].get();
    ComponentId cid = getComponentId<T>();

    // An entity cannot have more than one component of the same type
    assert(entity->componentBitset[cid] &&
           "Component does not exist for entity!");

    // Now we can remove it
    entity->componentBitset[cid] = false;
    entity->componentArray[cid].reset();
  }

  /*
   * Replace component belonging to a given entity
   */
  template<typename T, typename... TArgs>
  T& replaceComponent(EntityId entityId, TArgs&&... mArgs) {
    removeComponent<T>(entityId);
    addComponent<T>(entityId, std::forward<TArgs>(mArgs)...);
  }

  /*
   * Return the component associated with the specified entity
   */
  template<typename T>
  T& getComponent(EntityId entityId) {
    // Make sure the entity exists and has this component
    assert(entityMap.contains(entityId) && "Entity not found!");
    assert(hasComponent<T>(entityId) && "Entity does not have this component!");

    // Get component and return pointer to it
    ComponentId cid = getComponentId<T>();
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
  EntityId lastEntityId = 0;

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
    ComponentId cid = getComponentId<T>();
    return entityMap[entityId]->componentBitset[cid];
  }
};
