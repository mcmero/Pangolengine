#pragma once

#include <string>

struct Animation {
  std::string name;
  int index;
  int frames;
  int speed;

  Animation() = default;
  Animation(std::string name, int index, int frames, int speed) {
    this->name = name;
    this->index = index;
    this->frames = frames;
    this->speed = speed;
  }
};
