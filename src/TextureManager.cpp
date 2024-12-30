#include "TextureManager.h"

SDL_Texture *TextureManager::LoadTexture(const char *filePath,
                                         SDL_Renderer *ren) {
  SDL_Surface *tmpSurface = IMG_Load(filePath);
  SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, tmpSurface);
  SDL_DestroySurface(tmpSurface);

  return tex;
}
