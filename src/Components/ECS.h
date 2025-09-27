#pragma once

class IComponent;
class Entity;

class IComponent {
public:
	Entity* entity;

	virtual void init() {}
	virtual void update() {}
	virtual void render() {}
  virtual ~IComponent() = default;
};
