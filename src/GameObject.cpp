#include "GameObject.h"
#include "TextureManager.h"

GameObject::GameObject(const char *textureSheet, SDL_Renderer *ren, int x,
                       int y) {
  renderer = ren;
  objTexture = TextureManager::LoadTexture(textureSheet, ren);

  xpos = x;
  ypos = y;
}

void GameObject::Update() {
  if (xpos > 320)
    xpos = 0;
  if (ypos > 180)
    ypos = 0;

  xpos++;
  ypos++;

  srcRect.h = 32;
  srcRect.w = 32;
  srcRect.x = 0;
  srcRect.y = 0;

  destRect.x = float(xpos);
  destRect.y = float(ypos);
  destRect.w = srcRect.w;
  destRect.h = srcRect.h;
}

void GameObject::Render() {
  SDL_RenderTexture(renderer, objTexture, &srcRect, &destRect);
}
