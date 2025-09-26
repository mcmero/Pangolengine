#pragma once

#include "../TextureManager.h"
#include "IUIComponent.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "UIHelper.h"

class PortraitPanel : public IUIComponent {
public:
  PortraitPanel(float xpos, float ypos, float width, float height,
                float borderThickness, SDL_Color borderColour,
                SDL_Color innerColour, const char *texturePath)
      : borderRect(UIHelper::getBorderRect(xpos, ypos, width, height,
                                           borderThickness)),
        innerRect(UIHelper::getInnerRect(xpos, ypos, width, height)),
        portraitRect({xpos, ypos, width, height}), borderColour(borderColour),
        innerColour(innerColour) {
    portraitTex = TextureManager::LoadTexture(texturePath);
  }

  void render(SDL_Renderer *renderer, SDL_Window *window) override {
    if (show) {
      TextureManager::DrawPanel(borderRect, innerRect, borderColour,
                                innerColour);

      SDL_FRect srcRect = {0, 0, portraitRect.w, portraitRect.h};
      TextureManager::Draw(portraitTex, srcRect, portraitRect, SDL_FLIP_NONE);
    }
  }

  void update(Interactable *interactable, Dialogue *dialogue) override {
    if (interactable != nullptr && interactable->active) {
      show = true;
    } else
      show = false;
  }

  void clean() override { SDL_DestroyTexture(portraitTex); }

private:
  bool show = false;

  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_FRect portraitRect;
  SDL_Color borderColour;
  SDL_Color innerColour;

  SDL_Texture *portraitTex;
};
