#pragma once

#include "../Components/Dialogue.h"
#include "Grid.h"
#include "IUIManager.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_render.h"
#include "UIComponents.h"
#include <memory>

class UIManager : public IUIManager {
public:
  UIManager() {
    grid.addChild(std::make_shared<DialoguePanel>(
        80.0f, 130.0f, 220.0f, 36.0f, 2.0f, dialogueBorderColour,
        dialogueBoxColour, pointsize, fontColour));
    grid.addChild(std::make_shared<PortraitPanel>(
        36.0f, 130.0f, 36.0f, 36.0f, 2.0f, dialogueBorderColour,
        dialogueBoxColour, npcPortrait.c_str()));
    grid.addChild(std::make_shared<DialogueResponsePanel>(
        80.0f, 10.0f, 220.0f, 40.0f, 2.0f, dialogueBorderColour,
        dialogueBoxColour, pointsize, fontColour, selectColour));
    grid.addChild(std::make_shared<Options>(2.0f, menuBorderColour,
                                            dialogueBoxColour, menuBorderColour,
                                            pointsize, *this));
  }
  ~UIManager() { grid.clean(); }

  void render(SDL_Renderer *renderer, SDL_Window *window) {
    grid.render(renderer, window);
  }

  void update(Interactable *interactable, Dialogue *dialogue) {
    grid.update(interactable, dialogue);

    if (interactable && interactable->active)
      interactionActive = true;
    else {
      interactionActive = false;
    }
  };

  void handleEvents(const SDL_Event &event) { grid.handleEvents(event); }

  void trySetMenu(bool active) override {
    // Only allow menu to be activated if interaction is not active
    if (!interactionActive && active)
      menuActive = true;
    else
      menuActive = false;
  }

  bool isMenuActive() const override { return menuActive; }

private:
  Grid grid;
  bool menuActive = false;
  bool interactionActive = false;

  float pointsize = 14.0f;
  SDL_Color fontColour = {255, 255, 255};
  SDL_Color selectColour = {208, 199, 125};
  SDL_Color dialogueBorderColour = {0, 0, 0};
  SDL_Color dialogueBoxColour = {51, 64, 113};
  SDL_Color menuBorderColour = {208, 199, 125};

  // TODO: add portrait location to dialogue sheet
  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  std::string npcPortrait =
      (assetsPath / "textures" / "npc_portrait.png").string();
};
