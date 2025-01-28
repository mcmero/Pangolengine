#include "TextureManager.h"
#include "Game.h"
#include "SDL3_image/SDL_image.h"

SDL_Texture *TextureManager::LoadTexture(const char *filePath) {
  SDL_Surface *tmpSurface = IMG_Load(filePath);
  SDL_Texture *tex = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
  SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
  SDL_DestroySurface(tmpSurface);

  return tex;
}

void TextureManager::Draw(SDL_Texture *tex, SDL_FRect srcRect,
                          SDL_FRect destRect, SDL_FlipMode flip) {
  SDL_RenderTextureRotated(Game::renderer, tex, &srcRect, &destRect, NULL, NULL,
                           flip);
}
