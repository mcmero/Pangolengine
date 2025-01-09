#pragma once

#include <iostream>

class Transform {
public:
  Transform() {
    xpos = 0;
    xpos = 0;
  }

  Transform(int x, int y) {
    xpos = x;
    ypos = y;
  }

  int x() { return xpos; }
  int y() { return ypos; }

  void update() {
    if (xpos > 320)
      xpos = 0;
    if (ypos > 180)
      ypos = 0;

    xpos++;
    ypos++;

    std::cout << "x: " << xpos << " y: " << ypos << std::endl;
  }

private:
  int xpos, ypos;
};
