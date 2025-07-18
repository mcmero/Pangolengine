#include "TextureManager.h"
#include "Game.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_rect.h"
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

SDL_Texture *TextureManager::LoadMessageTexture(const std::string_view text,
                                                float pointsize, int wraplength,
                                                SDL_Color colour) {
  // Load font
  TTF_Font *font = TTF_OpenFont(fontPath.string().c_str(), pointsize);
  if (!font) {
    SDL_Log("TTF_OpenFont: %s\n", SDL_GetError());
    return nullptr;
  }

  // Make surface then load texture from it
  // TODO: make wraplength optional? if set to 0 do not wrap
  SDL_Surface *surfaceMessage = TTF_RenderText_Solid_Wrapped(
      font, text.data(), text.length(), colour, wraplength);

  SDL_Texture *messageTex =
      SDL_CreateTextureFromSurface(Game::renderer, surfaceMessage);

  // Set scale mode to ensure pixel-perfect rendering
  SDL_SetTextureScaleMode(messageTex, SDL_SCALEMODE_NEAREST);

  // Free memory
  SDL_DestroySurface(surfaceMessage);
  TTF_CloseFont(font);

  return messageTex;
}

void TextureManager::Panel(SDL_FRect borderRect, SDL_FRect innerRect,
                           SDL_Color borderColour, SDL_Color innerColour) {
  // Draw border rect
  SDL_SetRenderDrawColor(Game::renderer, borderColour.r, borderColour.g,
                         borderColour.b, SDL_ALPHA_OPAQUE);
  SDL_RenderRect(Game::renderer, &borderRect);
  SDL_RenderFillRect(Game::renderer, &borderRect);

  // Draw inner rect
  SDL_SetRenderDrawColor(Game::renderer, innerColour.r, innerColour.g,
                         innerColour.b, SDL_ALPHA_OPAQUE);
  SDL_RenderRect(Game::renderer, &innerRect);
  SDL_RenderFillRect(Game::renderer, &innerRect);
}

/**
 * Get message texture dimensions (width and height)
 */
MessageDims
TextureManager::GetMessageTextureDimensions(SDL_Texture *messageTex) {
  assert(messageTex != nullptr && "Message texture does not exist!");

  auto texprops = SDL_GetTextureProperties(messageTex);
  float textWidth =
      float(SDL_GetNumberProperty(texprops, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0));
  float textHeight =
      float(SDL_GetNumberProperty(texprops, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0));

  return {textWidth, textHeight};
}
