#pragma once

struct Animation {
  const char *name;
  int index;
  int frames;
  int speed;

  Animation() = default;
  Animation(const char *name, int index, int frames, int speed) {
    this->name = name;
    this->index = index;
    this->frames = frames;
    this->speed = speed;
  }
};
