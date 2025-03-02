#include "Camera.h"
#include "Constants.h"

SDL_Rect Camera::position = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

void Camera::update(int xpos, int ypos, int mapPixelWidth, int mapPixelHeight) {
  // Update camera logic
  position.x = xpos;
  position.y = ypos;

  // Keep within bounds of the world
  if (position.x < 0) {
    position.x = 0;
  }
  if (position.y < 0) {
    position.y = 0;
  }
  if (position.x > mapPixelWidth - position.w) {
    position.x = mapPixelWidth - position.w;
  }
  if (position.y > mapPixelHeight - position.h) {
    position.y = mapPixelHeight - position.h;
  }
}
