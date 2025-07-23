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
          SDL_Color innerColour, SDL_Color buttonColour, float pointsize,
          IUIManager &manager)
      : manager(&manager), borderColour(borderColour), innerColour(innerColour),
        buttonColour(buttonColour), pointsize(pointsize) {
    borderRect = UIHelper::getBorderRect(menuRect.x, menuRect.y, menuRect.w,
                                         menuRect.h, borderThickness);
    innerRect =
        UIHelper::getInnerRect(menuRect.x, menuRect.y, menuRect.w, menuRect.h);
  }

  void render(SDL_Renderer *renderer) override {
    if (show) {
      // Render main panel
      TextureManager::DrawPanel(borderRect, innerRect, borderColour,
                                innerColour);

      TextProperties headerProps = {"Options",    pointsize,    SCREEN_WIDTH,
                                    {0.0f, 0.0f}, headerColour, Align::Center,
                                    Align::Top};
      TextureManager::DrawText(headerProps, innerRect);

      // Render buttons
      for (const auto &[idx, item] : menuItems) {
        TextProperties textProps = {item.name,  pointsize,        SCREEN_WIDTH,
                                    textOffset, buttonTextColour, Align::Center,
                                    Align::Top};
        ButtonProperties graphicsButtonProps = {
            buttonSize, buttonColour, Align::Center, Align::Top, textProps};
        float spacingFactor =
            buttonSpacing * (static_cast<float>(idx) + 1.0f) + 5.0f;
        TextureManager::DrawButton(graphicsButtonProps, innerRect,
                                   spacingFactor);
      }
    }
  }

  void update(Interactable *interactable, Dialogue *dialogue) override {}

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
  SDL_Color buttonColour;

  float const pointsize = 14.0f;
  float const buttonSpacing = 15.0f;
  Vector2D const textOffset = {0.0f, -3.0f};
  Size const buttonSize = {60.0f, 12.0f};

  SDL_Color headerColour = {255, 255, 255};
  SDL_Color buttonTextColour = {0, 0, 0};

  struct MenuItem {
    std::string name = "";
    // linked menu items will go here
  };

  std::map<int, MenuItem> menuItems{
      {0, {"Graphics"}}, {1, {"Audio"}}, {2, {"Gameplay"}}};
};
