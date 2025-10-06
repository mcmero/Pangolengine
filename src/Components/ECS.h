#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cassert>

constexpr std::size_t maxComponents = 32;

//------------------------------------------------------------------------------
// Entity
//------------------------------------------------------------------------------
using EntityId = std::size_t;
struct Entity {
  std::bitset<maxComponents> componentBitset = 0;
};

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------

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

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void removeComponent(EntityId entityId) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public:
  T& addComponent(EntityId entityId, T &component) {
    std::size_t index = components.size();
    components.push_back(std::move(component));
    entityToIndex[entityId] = index;
    indexToEntity[index] = entityId;
    return components[entityToIndex[entityId]];
  }

  T& getComponent(EntityId entityId) {
      return components[entityToIndex[entityId]];
  }

  void removeComponent(EntityId entityId) override {
    if (!entityToIndex.contains(entityId)) {
        return;
    }

    std::size_t indexToRemove = entityToIndex[entityId];
    std::size_t lastIndex = components.size() - 1;

    if (indexToRemove != lastIndex) {
      components[indexToRemove] = std::move(components[lastIndex]);
      EntityId lastEntityId = indexToEntity[lastIndex];
      entityToIndex[lastEntityId] = indexToRemove;
      indexToEntity[indexToRemove] = lastEntityId;
    }

    components.pop_back();
    entityToIndex.erase(entityId);
    indexToEntity.erase(lastIndex);
  }

private:
    std::vector<T> components = {};
    std::unordered_map<EntityId, std::size_t> entityToIndex = {};
    std::unordered_map<std::size_t, EntityId> indexToEntity = {};
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
    EntityId entityId = ++entityIdCounter;
    entityMap[entityId] = std::unique_ptr<Entity>(new Entity{});
    return entityId;
  };

  /*
   * Add component to entity and initialise
   */
  template<typename T, typename... TArgs>
  T& addComponent(EntityId entityId, TArgs&&... mArgs) {
    // Make sure the entity exists
    assert(entityMap.contains(entityId) && "Entity not found!");
    Entity* entity = entityMap[entityId].get();

    // An entity cannot have more than one component of the same type
    ComponentId cid = getComponentId<T>();
    assert(!entity->componentBitset[cid] &&
           "Component exists for entity!");

    // Set component bit
    entity->componentBitset[cid] = true;

    // Create the component
    T component(std::forward<TArgs>(mArgs)...);

    // Create component array if it doesn't exist
    if (!componentArrays.contains(cid)) {
        componentArrays[cid] = std::make_unique<ComponentArray<T>>();
    }

    // Get component array and add component
    auto* typedArray = static_cast<ComponentArray<T>*>(componentArrays[cid].get());
    return typedArray->addComponent(entityId, component);
  };

  /*
   * Remove component from entity
   */
  template<typename T, typename... TArgs>
  void removeComponent(EntityId entityId) {
    // Make sure the entity exists
    assert(entityMap.contains(entityId) && "Entity not found!");

    // Get entity pointer and component ID
    Entity* entity = entityMap[entityId].get();
    ComponentId cid = getComponentId<T>();

    // An entity cannot have more than one component of the same type
    assert(entity->componentBitset[cid] &&
           "Component does not exist for entity!");

    // Now we can remove it
    entity->componentBitset[cid] = false;
    auto* typedArray = static_cast<ComponentArray<T>*>(componentArrays[cid].get());
    typedArray->removeComponent(entityId);
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
    auto* typedArray = static_cast<ComponentArray<T>*>(componentArrays[cid].get());
    return typedArray->getComponent(entityId);
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
    // Do nothing if entity doesn't exist
    if (!entityMap.contains(entityId))
      return;

    // Remove all components from entity
    for (auto& [cid, array] : componentArrays) {
        array->removeComponent(entityId);
    }

    entityMap.erase(entityId);
  }

  /*
  * Clear all entities in the registry and reset entity ID
  * Components lookup is not cleared to allow reuse
  */
  void clear() {
    entityMap.clear();
    componentArrays.clear();
    entityIdCounter = 0;
  };

  ~EntityRegistry() = default;

private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> entityMap = {};
  std::unordered_map<ComponentId, std::unique_ptr<IComponentArray>> componentArrays = {};
  EntityId entityIdCounter = 0;

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
