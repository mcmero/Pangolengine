#pragma once

#include "../TextureManager.h"
#include "IUIComponent.h"
#include "IUIManager.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"
#include "UIHelper.h"
#include <unordered_map>

namespace fs = std::filesystem;

class Options : public IUIComponent {
public:
  Options(float borderThickness, SDL_Color borderColour,
          SDL_Color innerColour, SDL_Color buttonColour,
          float pointsize, IUIManager &manager)
      : manager(&manager), borderThickness(borderThickness),
        pointsize(pointsize), borderColour(borderColour),
        innerColour(innerColour), buttonColour(buttonColour) {

    // Load any textures
    selectIconTex = TextureManager::LoadTexture(selectIconPath.string().c_str());

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
    menus["Main"] = mainMenu;

    // Graphics options
    //--------------------------------------------------------------------------
    std::unordered_map<std::string, MenuItem> graphicsMenuItems = {
      {"Resolution", {}},
    };

    // Resolution options
    // 180p (native)
    OptionItem setRes180p = {"320x180"};
    setRes180p.function = [](SDL_Renderer *renderer, SDL_Window *window){
      SDL_SetWindowFullscreen(window, false);
      SDL_SetWindowSize(window, 320, 180);
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    };
    graphicsMenuItems["Resolution"].optionItems.push_back(setRes180p);

    // 720p
    OptionItem setRes720p = {"1280x720"};
    setRes720p.function = [](SDL_Renderer *renderer, SDL_Window *window){
      SDL_SetWindowFullscreen(window, false);
      SDL_SetWindowSize(window, 1280, 720);
      SDL_SetRenderScale(renderer, 4.0f, 4.0f);
    };
    graphicsMenuItems["Resolution"].optionItems.push_back(setRes720p);
    
    // Full screen mode
    OptionItem setFullScreen = {"Max"};
    setFullScreen.function = [](SDL_Renderer *renderer, SDL_Window *window){
      SDL_SetWindowFullscreen(window, true);

      // Get the render size so that we can adapt it to full screen resolution
      int w = 0; int h = 0;
      SDL_GetWindowSizeInPixels(window, &w, &h);
      float scaleX = static_cast<float>(w) / static_cast<float>(SCREEN_WIDTH);
      float scaleY = static_cast<float>(h) / static_cast<float>(SCREEN_HEIGHT);
      SDL_SetRenderScale(renderer, scaleX, scaleY);
    };
    graphicsMenuItems["Resolution"].optionItems.push_back(setFullScreen);

    Menu graphicsMenu = {
      "Graphics",
      graphicsMenuItems,
      MenuType::Sub
    };
    menus["Graphics"] = graphicsMenu;
    //--------------------------------------------------------------------------

    // Link Graphics button to Graphics menu
    menus["Main"].menuItems["Graphics"].linkedMenu = &menus["Graphics"];

    // Set up default option settings
    // Resolution = 1280x720
    menus["Graphics"].menuItems["Resolution"].selectedItem = 1;

    // Initialise main menu as active
    activeMenu = &menus["Main"];
  }

  void render(SDL_Renderer *renderer, SDL_Window *window) override {
    // Handle item settings involving render changes
    if (itemSet) {
      itemSet->selectItem(renderer, window);
      itemSet = nullptr; // Release pointer
    }

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
          textColour,
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
          textColour,
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
        SDL_Color currentTextColour = textColour;
        if (selectedItem == idx)
          currentTextColour = textSelectColour;

        TextProperties textProps = {
            item.first + ":",
            pointsize,
            SCREEN_WIDTH,
            currentTextColour,
            Align::Left,
            Align::Top,
            {0.0f, 0.0f, 10.0f, 0.0f}
        };

        float spacingFactor =
            itemSpacing * (static_cast<float>(idx) + 1.0f) + 5.0f;
        SDL_FRect textContainer = innerRect;
        textContainer.y = textContainer.y + spacingFactor; // Shift text down
 
        TextureManager::DrawText(textProps, textContainer);
 
        // Right option
        //----------------------------------------------------------------------
        // Change text colour if button is selected
        SDL_Color optionTextColour = textColour;
        if (mode == SelectMode::Option && selectedItem == idx) {
          optionTextColour = textSelectColour; // Highlight option if in option mode

          // Left triangle
          float tw = static_cast<float>(selectIconTex->w);
          float th = static_cast<float>(selectIconTex->h);
          SDL_FRect srcRect = {0.0f, 0.0f, tw, th};
          SDL_FRect leftRect = {textContainer.x + (textContainer.w / 2.0f),
                                textContainer.y + 5.0f, tw, th};
          TextureManager::Draw(selectIconTex, srcRect, leftRect,
                               SDL_FLIP_HORIZONTAL);

          // Right triangle
          SDL_FRect rightRect = {textContainer.x + textContainer.w - 10.0f,
                                 textContainer.y + 5.0f, tw, th};
          TextureManager::Draw(selectIconTex, srcRect, rightRect,
                               SDL_FLIP_NONE);
        }
        if (item.second.selectedItem >= 0 && item.second.selectedItem < item.second.optionItems.size()) {
          TextProperties optTextProps = {
              item.second.optionItems[item.second.selectedItem].name,
              pointsize,
              SCREEN_WIDTH,
              optionTextColour,
              Align::Center,
              Align::Top,
              {0.0f, 0.0f, 0.0f, 0.0f}
          };

          // Align the option item to the right
          SDL_FRect optionContainer = textContainer;
          optionContainer.w = optionContainer.w / 2.0f;
          UIHelper::alignRelativeToContainer(
            optionContainer,
            textContainer,
            Align::Right,
            Align::Top
          );
          TextureManager::DrawText(optTextProps, optionContainer);

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
      else if (manager->isMenuActive() &&
               activeMenu->menuType == MenuType::Sub &&
               mode == SelectMode::Item)
        activeMenu = &menus["Main"]; // go back to the main menu
      else if (manager->isMenuActive() &&
               activeMenu->menuType == MenuType::Sub &&
               mode == SelectMode::Option)
        mode = SelectMode::Item; // go back to item selection
      else
        manager->trySetMenu(false);

      show = manager->isMenuActive();
    }

    // Ignore other events if menu is inactive
    if (!manager->isMenuActive())
      return;

    if (event.type == SDL_EVENT_KEY_DOWN) {
      // Get the selected menu and option items (need this to inform selection
      // and scrolling)
      auto *selectedMenu = getItemFromIndex<MenuItem>(
          activeMenu->menuItems,
          selectedItem
      );
      std::vector<OptionItem> *options = &selectedMenu->second.optionItems;

      switch (event.key.key) {

      // Button selection scrolling
      //----------------------------------------------------------------------
      case SDLK_UP:
        if (mode == SelectMode::Item)
          scroll(selectedItem,
                 static_cast<int>(activeMenu->menuItems.size()),
                 Scroll::Back);
        break;

      case SDLK_DOWN:
        if (mode == SelectMode::Item)
          scroll(selectedItem,
                 static_cast<int>(activeMenu->menuItems.size()),
                 Scroll::Forward);
        break;

      // Option item scrolling
      //----------------------------------------------------------------------
      case SDLK_LEFT:
        if (mode == SelectMode::Option)
          scroll(selectedMenu->second.selectedItem,
                 static_cast<int>(options->size()), Scroll::Forward);
        break;

      case SDLK_RIGHT:
        if (mode == SelectMode::Option)
          scroll(selectedMenu->second.selectedItem,
                 static_cast<int>(options->size()), Scroll::Back);
        break;

      // Item selection
      //----------------------------------------------------------------------
      case SDLK_RETURN: {
        if (mode == SelectMode::Item && selectedMenu->second.linkedMenu) {
          // Set new active menu
          activeMenu = selectedMenu->second.linkedMenu;
        } else if (mode == SelectMode::Item && options && options->size() > 0) {
          // Switch to option selection mode
          mode = SelectMode::Option;
        } else if (mode == SelectMode::Option && options && options->size() > 0) {
          // Make sure selection is valid
          if (selectedMenu->second.selectedItem >= 0 &&
              selectedMenu->second.selectedItem < options->size()) {
            itemSet = &(*options)[selectedMenu->second.selectedItem];
          }
          // Back to item select mode
          mode = SelectMode::Item;
        }
        break;
      }
      default:
        break;
      }
    }
  }

  void clean() override {
    SDL_DestroyTexture(selectIconTex);
  }

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

  SDL_Color textColour = {255, 255, 255};            // White
  SDL_Color textSelectColour = {208, 199, 125};      // Yellow
  SDL_Color buttonTextColour = {0, 0, 0};            // Black
  SDL_Color buttonTextSelectColour = {104, 31, 31};  // Dark red

  enum class MenuType { Main, Sub };
  struct MenuItem; // Forward definition
  struct Menu {
    std::string headerText = "";
    std::unordered_map<std::string, MenuItem> menuItems = {};
    MenuType menuType = MenuType::Main;
  };
  struct OptionItem {
    std::string name = "";
    std::function<void(SDL_Renderer*, SDL_Window*)> function;
    void selectItem(SDL_Renderer* ren, SDL_Window* win) {
        if (function)
            function(ren, win);
    }
  };
  struct MenuItem {
    Menu *linkedMenu = nullptr;
    std::vector<OptionItem> optionItems = {};
    int selectedItem = -1;
  };
  Menu *activeMenu;
  std::unordered_map<std::string, Menu> menus = {};

  // Item mode means we are selecting menu items
  // Option mode means we are slecting options within items
  enum class SelectMode { Item, Option };

  // Back/forward scrolling handles both up/down and left/right scrolling
  enum class Scroll { Back, Forward };
  SelectMode mode = SelectMode::Item;
  int selectedItem = 0;
  OptionItem *itemSet = nullptr;

  fs::path selectIconPath = fs::path(SDL_GetBasePath()) /
                              "assets" / "textures" / "select_icon.png";
  SDL_Texture *selectIconTex;

  // Helper method to element from index position in unordered_map
  template<typename T>
  std::pair<const std::string, T>* getItemFromIndex(std::unordered_map<std::string, T> &umap, int idx) {
    if (idx >= umap.size())
      return nullptr;

    auto it = umap.begin();
    std::advance(it, idx);

    return &(*it);
  }

  // Helper method for scrolling
  void scroll(int &selected, int size, Scroll dir) {
    if (dir == Scroll::Forward && (selected + 1) >= size) {
      selected = 0;
    } else if (dir == Scroll::Forward) {
      selected++;
    } else if (dir == Scroll::Back && (selected - 1) < 0) {
      selected = size - 1;
    } else {
      selected--;
    }
  }
};
