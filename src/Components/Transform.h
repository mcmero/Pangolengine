#pragma once

#include "../Vector2D.h"

class Transform {
public:
  Transform() { position = Vector2D(); }

  Transform(float x, float y) { position = Vector2D(x, y); }

  Vector2D position;
};
