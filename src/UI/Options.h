#pragma once

#include "../TextureManager.h"
#include "IComponent.h"
#include "IUIManager.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "UIHelper.h"

class Options : public IComponent {
public:
  Options(SDL_FRect menuRect, float borderThickness, SDL_Color borderColour,
          SDL_Color innerColour, float pointsize, IUIManager &manager)
      : manager(&manager), borderColour(borderColour), innerColour(innerColour),
        pointsize(pointsize) {
    borderRect = UIHelper::getBorderRect(menuRect.x, menuRect.y, menuRect.w,
                                         menuRect.h, borderThickness);
    innerRect =
        UIHelper::getInnerRect(menuRect.x, menuRect.y, menuRect.w, menuRect.h);
  }

  void render(SDL_Renderer *renderer) override {
    if (show) {
      // Render main panel
      TextureManager::Panel(borderRect, innerRect, borderColour, innerColour);

      // Render header text
      SDL_FRect dest = {0, 0, headerDims.width, headerDims.height};
      UIHelper::centerRectRelativeToContainer(dest, innerRect);
      SDL_RenderTexture(renderer, headerTex, NULL, &dest);
    }
  }

  void update(Interactable *interactable, Dialogue *dialogue) override {
    if (show) {
      // Set up texture for header
      SDL_DestroyTexture(headerTex);
      headerTex = TextureManager::LoadMessageTexture(headerText, pointsize, 99,
                                                     headerColour);
      headerDims = TextureManager::GetMessageTextureDimensions(headerTex);
    }
  }

  void handleEvents(const SDL_Event &event) override {
    // Open or close menu with escape
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
      if (!show)
        manager->trySetMenu(true);
      else
        manager->trySetMenu(false);
      show = manager->isMenuActive();
      std::cout << "Menu active: " << manager->isMenuActive() << std::endl;
    }
  }

  void clean() override {}

private:
  bool show = false;
  IUIManager *manager;

  SDL_FRect borderRect;
  SDL_FRect innerRect;
  SDL_Color borderColour;
  SDL_Color innerColour;

  float pointsize = 14;
  std::string const headerText = "Options";
  SDL_Texture *headerTex;
  MessageDims headerDims;
  SDL_Color headerColour = {255, 255, 255};
};
