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
#include <unordered_map>

class Options : public IComponent {
public:
  Options(float borderThickness, SDL_Color borderColour,
          SDL_Color innerColour, SDL_Color buttonColour,
          float pointsize, IUIManager &manager)
      : manager(&manager), borderThickness(borderThickness),
        pointsize(pointsize), borderColour(borderColour),
        innerColour(innerColour), buttonColour(buttonColour) {

    std::unordered_map<std::string, MenuItem> mainMenuItems = {
      {"Graphics", {}},
      {"Audio", {}},
      {"Gameplay", {}}
    };
    Menu mainMenu = {
      "Options",
      mainMenuItems,
      MenuType::Main
    };
    menus["main"] = mainMenu;

    // Graphics options
    //--------------------------------------------------------------------------
    std::unordered_map<std::string, MenuItem> graphicsMenuItems = {
      {"Full screen", {}},
      {"Resolution", {}},
    };

    // Windowed mode option
    OptionItem setWindowed = {"No"};
    setWindowed.function =  [](){ std::cout << "Windowed mode!" << std::endl; };
    graphicsMenuItems["Full screen"].optionItems.push_back(setWindowed);
    graphicsMenuItems["Full screen"].selectedItem = &graphicsMenuItems["Full screen"].optionItems[0]; // currently selected

    // Full screen mode
    OptionItem setFullScreen = {"Yes"};
    setFullScreen.function =  [](){ std::cout << "Full screen mode!" << std::endl; };
    graphicsMenuItems["Full screen"].optionItems.push_back(setFullScreen);

    Menu graphicsMenu = {
      "Graphics",
      graphicsMenuItems,
      MenuType::Sub
    };
    menus["graphics"] = graphicsMenu;
    //--------------------------------------------------------------------------

    // Link Graphics button to Graphics menu
    menus["main"].menuItems["Graphics"].linkedMenu = &menus["graphics"];

    // Initialise main menu as active
    activeMenu = &menus["main"];
  }

  void render(SDL_Renderer *renderer) override {
    if (show && activeMenu && activeMenu->menuType == MenuType::Main) {
      // Main menu
      //------------------------------------------------------------------------
      // Render main panel
      SDL_FRect borderRect = UIHelper::getBorderRect(
          mainMenuRect.x,
          mainMenuRect.y,
          mainMenuRect.w,
          mainMenuRect.h,
          borderThickness
      );
      SDL_FRect innerRect = UIHelper::getInnerRect(
          mainMenuRect.x,
          mainMenuRect.y,
          mainMenuRect.w,
          mainMenuRect.h
      );
      TextureManager::DrawPanel(borderRect, innerRect,
                                borderColour, innerColour);

      TextProperties headerProps = {
          activeMenu->headerText,
          pointsize,
          SCREEN_WIDTH,
          headerColour,
          Align::Center,
          Align::Top
      };
      TextureManager::DrawText(headerProps, innerRect);

      // Render buttons
      int idx = 0;
      for (const auto &item : activeMenu->menuItems) {
        // Change text colour if button is selected
        SDL_Color currentTextColour = buttonTextColour;
        if (selectedItem == idx)
          currentTextColour = buttonTextSelectColour;

        TextProperties textProps = {
            item.first,
            pointsize,
            SCREEN_WIDTH,
            currentTextColour,
            Align::Center,
            Align::Top,
            textOffset
        };
        ButtonProperties buttonProps = {
            buttonSize,
            buttonColour,
            Align::Center,
            Align::Top,
            textProps
        };
        float spacingFactor =
            itemSpacing * (static_cast<float>(idx) + 1.0f) + 5.0f;
        TextureManager::DrawButton(buttonProps, innerRect,
                                   spacingFactor);
        ++idx;
      }
    } else if (show && activeMenu && activeMenu->menuType == MenuType::Sub) {
      // Sub menus
      //------------------------------------------------------------------------
      // Render sub panel
      SDL_FRect borderRect = UIHelper::getBorderRect(
          subMenuRect.x,
          subMenuRect.y,
          subMenuRect.w,
          subMenuRect.h,
          borderThickness
      );
      SDL_FRect innerRect = UIHelper::getInnerRect(
          subMenuRect.x,
          subMenuRect.y,
          subMenuRect.w,
          subMenuRect.h
      );
      TextureManager::DrawPanel(borderRect, innerRect,
                                borderColour, innerColour);

      // Render header
      TextProperties headerProps = {
          activeMenu->headerText,
          pointsize,
          SCREEN_WIDTH,
          headerColour,
          Align::Center,
          Align::Top
      };
      TextureManager::DrawText(headerProps, innerRect);

      // Render options
      int idx = 0;
      for (const auto &item : activeMenu->menuItems) {
        // Left menu label
        //----------------------------------------------------------------------
        // Change text colour if button is selected
        SDL_Color currentTextColour = headerColour;
        if (selectedItem == idx)
          currentTextColour = buttonColour;

        TextProperties textProps = {
            item.first + ":",
            pointsize,
            SCREEN_WIDTH,
            currentTextColour,
            Align::Left,
            Align::Top,
            {0.0f, 0.0f, 5.0f, 0.0f}
        };

        float spacingFactor =
            itemSpacing * (static_cast<float>(idx) + 1.0f) + 5.0f;
        SDL_FRect textContainer = innerRect;
        textContainer.y = textContainer.y + spacingFactor; // Shift text down
 
        TextureManager::DrawText(textProps, textContainer);
  
        // Right option
        //----------------------------------------------------------------------
        if (item.second.selectedItem) {
          TextProperties optTextProps = {
              //item.selectedItem->name, // TODO: issue reading this string for some reason
              "No", // placeholder
              pointsize,
              SCREEN_WIDTH,
              currentTextColour,
              Align::Right,
              Align::Top,
              {0.0f, 0.0f, 0.0f, 20.0f}
          };
          TextureManager::DrawText(optTextProps, textContainer);
        }
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
      else if (manager->isMenuActive() && activeMenu->menuType == MenuType::Sub)
        activeMenu = &menus["main"]; // go back to the main menu
      else
        manager->trySetMenu(false);

      show = manager->isMenuActive();
    }

    // Ignore other events if menu is inactive
    if (!manager->isMenuActive())
      return;

    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.key) {
      case SDLK_DOWN:
        if ((selectedItem + 1) >=
            static_cast<int>(activeMenu->menuItems.size()))
          selectedItem = 0;
        else
          selectedItem++;
        break;
      case SDLK_UP:
        if ((selectedItem - 1) < 0)
          selectedItem = static_cast<int>(activeMenu->menuItems.size()) - 1;
        else
          selectedItem--;
        break;
        case SDLK_RETURN: {
          // Set new active menu
          Menu *linkedMenu = getItemFromIndex<MenuItem>(
            activeMenu->menuItems,
            selectedItem
          )->linkedMenu;
          if (linkedMenu)
            activeMenu = linkedMenu;
          // TODO: trigger option selectItem()
          break;
        }
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
  SDL_FRect subMenuRect = SDL_FRect(100.0f, 42.0f, 140.0f, 60.0f);

  float borderThickness = 2.0f;
  float pointsize = 14.0f;
  float itemSpacing = 15.0f;
  Margin const textOffset = {0.0f, 3.0f, 0.0f, 0.0f};
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
    std::unordered_map<std::string, MenuItem> menuItems = {};
    MenuType menuType = MenuType::Main;
  };
  struct OptionItem {
    std::string name = "";
    std::function<void()> function;
    void selectItem() {
        if (function)
            function();
    }
  };
  struct MenuItem {
    Menu *linkedMenu = nullptr;
    std::vector<OptionItem> optionItems = {};
    OptionItem *selectedItem = nullptr;
  };

  Menu *activeMenu;
  std::unordered_map<std::string, Menu> menus = {};
  int selectedItem = 0;

  // Helper method to get index position of element in unordered_map
  template <typename T>
  T* getItemFromIndex(std::unordered_map<std::string, T> &umap, int idx) {
    if (idx >= umap.size())
      return nullptr;

    auto it = umap.begin();
    std::advance(it, idx);

    return &(it->second);
  }
};
