#include "TextureManager.h"
#include "Game.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
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

void TextureManager::DrawRect(SDL_FRect rect, SDL_Color colour) {
  SDL_SetRenderDrawColor(Game::renderer, colour.r, colour.g, colour.b,
                         SDL_ALPHA_OPAQUE);
  SDL_RenderRect(Game::renderer, &rect);
  SDL_RenderFillRect(Game::renderer, &rect);
}

void TextureManager::DrawPanel(SDL_FRect borderRect, SDL_FRect innerRect,
                               SDL_Color borderColour, SDL_Color innerColour) {
  TextureManager::DrawRect(borderRect, borderColour);
  TextureManager::DrawRect(innerRect, innerColour);
}

void TextureManager::DrawText(TextProperties textProps,
                              SDL_FRect const &containerRect) {

  SDL_Texture *textTex = TextureManager::LoadMessageTexture(
      textProps.text, textProps.pointsize, textProps.wraplength,
      textProps.colour);
  Size textDims = TextureManager::GetMessageTextureDimensions(textTex);

  // Render button text
  SDL_FRect textRect = {0, 0, textDims.width, textDims.height};
  UIHelper::alignRelativeToContainer(textRect, containerRect,
                                     textProps.horizontalAlign,
                                     textProps.verticalAlign);
  textRect.y = textRect.y + textProps.margin.top - textProps.margin.bottom;
  textRect.x = textRect.x + textProps.margin.left - textProps.margin.right;
  SDL_RenderTexture(Game::renderer, textTex, NULL, &textRect);

  // Cleanup
  SDL_DestroyTexture(textTex);
}

void TextureManager::DrawButton(ButtonProperties buttonProps,
                                SDL_FRect const &containerRect,
                                float buttonSpacing) {
  // Make button container and align
  SDL_FRect buttonContainer = {containerRect.x, containerRect.y,
                               buttonProps.size.width, buttonProps.size.height};
  UIHelper::alignRelativeToContainer(buttonContainer, containerRect,
                                     buttonProps.horizontalAlign,
                                     buttonProps.verticalAlign);
  buttonContainer.y = buttonContainer.y + buttonSpacing; // Shift button down

  // Render
  TextureManager::DrawRect(buttonContainer, buttonProps.colour);
  TextureManager::DrawText(buttonProps.textProps, buttonContainer);
}

/**
 * Get message texture dimensions (width and height)
 */
Size TextureManager::GetMessageTextureDimensions(SDL_Texture *messageTex) {
  assert(messageTex != nullptr && "Message texture does not exist!");

  auto texprops = SDL_GetTextureProperties(messageTex);
  float textWidth =
      float(SDL_GetNumberProperty(texprops, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0));
  float textHeight =
      float(SDL_GetNumberProperty(texprops, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0));

  return {textWidth, textHeight};
}
