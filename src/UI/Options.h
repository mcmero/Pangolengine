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
  Options(float borderThickness, SDL_Color borderColour, SDL_Color innerColour,
          SDL_Color buttonColour, float pointsize, IUIManager &manager)
      : manager(&manager), borderThickness(borderThickness),
        pointsize(pointsize), borderColour(borderColour),
        innerColour(innerColour), buttonColour(buttonColour) {
    std::vector<MenuItem> mainMenuItems = {
        MenuItem{"Graphics"}, MenuItem{"Audio"}, MenuItem{"Gameplay"}};
    Menu mainMenu = {"Options", mainMenuItems, MenuType::Main};
    menus.push_back(mainMenu);
    activeMenu = &menus[0];
  }

  void render(SDL_Renderer *renderer) override {
    if (show && activeMenu && activeMenu->menuType == MenuType::Main) {
      // Render main panel
      SDL_FRect borderRect = UIHelper::getBorderRect(
          mainMenuRect.x, mainMenuRect.y, mainMenuRect.w, mainMenuRect.h,
          borderThickness);
      SDL_FRect innerRect = UIHelper::getInnerRect(
          mainMenuRect.x, mainMenuRect.y, mainMenuRect.w, mainMenuRect.h);
      TextureManager::DrawPanel(borderRect, innerRect, borderColour,
                                innerColour);

      TextProperties headerProps = {
          activeMenu->headerText, pointsize,     SCREEN_WIDTH, {0.0f, 0.0f},
          headerColour,           Align::Center, Align::Top};
      TextureManager::DrawText(headerProps, innerRect);

      // Render buttons
      int idx = 0;
      for (const auto &item : activeMenu->menuItems) {
        // Change text colour if button is selected
        SDL_Color currentTextColour = buttonTextColour;
        if (selectedButton == idx)
          currentTextColour = buttonTextSelectColour;

        TextProperties textProps = {
            item.name,         pointsize,     SCREEN_WIDTH, textOffset,
            currentTextColour, Align::Center, Align::Top};
        ButtonProperties graphicsButtonProps = {
            buttonSize, buttonColour, Align::Center, Align::Top, textProps};
        float spacingFactor =
            buttonSpacing * (static_cast<float>(idx) + 1.0f) + 5.0f;
        TextureManager::DrawButton(graphicsButtonProps, innerRect,
                                   spacingFactor);
        ++idx;
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

    // Ignore other events if menu is inactive
    if (!manager->isMenuActive())
      return;

    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_DOWN:
        if ((selectedButton + 1) >=
            static_cast<int>(activeMenu->menuItems.size()))
          selectedButton = 0;
        else
          selectedButton++;
        break;
      case SDLK_UP:
        if ((selectedButton - 1) < 0)
          selectedButton = static_cast<int>(activeMenu->menuItems.size()) - 1;
        else
          selectedButton--;
        break;
      case SDLK_RETURN:
        // Handle menu change here
        break;
      default:
        break;
      }
    }
  }

  void clean() override {}

private:
  bool show = false;
  IUIManager *manager;

  SDL_FRect mainMenuRect = SDL_FRect(120.0f, 42.0f, 80.0f, 80.0f);
  // SDL_FRect subMenuRect = SDL_FRect(100.0f, 42.0f, 140.0f, 60.0f);
  int selectedButton = 0;

  float borderThickness = 2.0f;
  float pointsize = 14.0f;
  float buttonSpacing = 15.0f;
  Vector2D const textOffset = {0.0f, -3.0f};
  Size const buttonSize = {60.0f, 12.0f};

  SDL_Color borderColour;
  SDL_Color innerColour;
  SDL_Color buttonColour;

  SDL_Color headerColour = {255, 255, 255};
  SDL_Color buttonTextColour = {0, 0, 0};
  SDL_Color buttonTextSelectColour = {104, 31, 31};

  enum class MenuType { Main, Sub };
  struct MenuItem; // Forward definition
  struct Menu {
    std::string headerText = "";
    std::vector<MenuItem> menuItems = {};
    MenuType menuType = MenuType::Main;
  };
  struct MenuItem {
    std::string name = "";
    Menu *linkedMenu;
  };

  Menu *activeMenu;
  std::vector<Menu> menus = {};
};
