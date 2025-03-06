#include "TextureManager.h"
#include "Game.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"

fs::path TextureManager::fontPath = fs::path(SDL_GetBasePath()) / "assets" /
                                    "fonts" / "AtlantisInternational-jen0.ttf";

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

void TextureManager::Text(const std::string_view text, float pointsize,
                          float xpos, float ypos) {
  // Load font
  TTF_Font *font = TTF_OpenFont(fontPath.string().c_str(), pointsize);
  if (!font) {
    SDL_Log("TTF_OpenFont: %s\n", SDL_GetError());
    return;
  }

  // Make surface then load texture from it
  SDL_Surface *surfaceMessage =
      TTF_RenderText_Solid(font, text.data(), text.length(), {0, 0, 0});
  SDL_Texture *messageTex =
      SDL_CreateTextureFromSurface(Game::renderer, surfaceMessage);
  SDL_SetTextureScaleMode(messageTex, SDL_SCALEMODE_NEAREST);
  SDL_DestroySurface(surfaceMessage);

  // get the on-screen dimensions of the text. this is necessary for
  // rendering it
  auto texprops = SDL_GetTextureProperties(messageTex);
  SDL_FRect text_rect{.x = xpos,
                      .y = ypos,
                      .w = float(SDL_GetNumberProperty(
                          texprops, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)),
                      .h = float(SDL_GetNumberProperty(
                          texprops, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0))};
  SDL_RenderTexture(Game::renderer, messageTex, NULL, &text_rect);
}
