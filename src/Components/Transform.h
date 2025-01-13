#pragma once

#include "../Vector2D.h"

class Transform {
public:
  Vector2D position;

  Transform() { position = Vector2D(); }

  Transform(float x, float y) { position = Vector2D(x, y); }

  void update() {}
};
