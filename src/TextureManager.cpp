#include "TextureManager.h"

SDL_Texture *TextureManager::LoadTexture(const char *filePath) {
  SDL_Surface *tmpSurface = IMG_Load(filePath);
  SDL_Texture *tex = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
  SDL_DestroySurface(tmpSurface);

  return tex;
}
