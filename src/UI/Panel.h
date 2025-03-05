#pragma once

#include "IComponent.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3_ttf/SDL_ttf.h"
#include <filesystem>

namespace fs = std::filesystem;

class Panel : public IComponent {
public:
  const fs::path fontPath = std::filesystem::path(SDL_GetBasePath()) /
                            "assets" / "fonts" /
                            "AtlantisInternational-jen0.ttf";

  Panel(float xpos, float ypos, float width, float height)
      : rect{xpos, ypos, width, height} {}

  void render(SDL_Renderer *renderer) override {
    if (show) {
      SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
      SDL_RenderRect(renderer, &rect);
      SDL_RenderFillRect(renderer, &rect);

      // Load font
      TTF_Font *font = TTF_OpenFont(fontPath.string().c_str(), 16);
      if (!font) {
        SDL_Log("TTF_OpenFont: %s\n", SDL_GetError());
        return;
      }

      // TODO: add text rendering to texture manager
      // Render text
      const std::string_view text = "Hey man, fancy seeing you here.";

      SDL_Surface *surfaceMessage =
          TTF_RenderText_Solid(font, text.data(), text.length(), {0, 0, 0});

      // make a texture from the surface
      SDL_Texture *messageTex =
          SDL_CreateTextureFromSurface(renderer, surfaceMessage);
      SDL_SetTextureScaleMode(messageTex, SDL_SCALEMODE_NEAREST);
      SDL_DestroySurface(surfaceMessage);

      // get the on-screen dimensions of the text. this is necessary for
      // rendering it
      auto texprops = SDL_GetTextureProperties(messageTex);
      SDL_FRect text_rect{.x = rect.x + 10,
                          .y = rect.y + 5,
                          .w = float(SDL_GetNumberProperty(
                              texprops, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)),
                          .h = float(SDL_GetNumberProperty(
                              texprops, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0))};
      SDL_RenderTexture(renderer, messageTex, NULL, &text_rect);
    }
  }

  void update(const SDL_Event &event, Interactable *interactable) override {
    if (interactable != nullptr && interactable->isActive) {
      show = true;
    }
  }

private:
  SDL_FRect rect;
  bool show = false;
};
