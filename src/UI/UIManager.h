#pragma once

#include "../Components/Dialogue.h"
#include "DialoguePanel.h"
#include "DialogueResponsePanel.h"
#include "Grid.h"
#include "PortraitPanel.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_render.h"
#include <memory>

class UIManager {
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
  }
  ~UIManager() { grid.clean(); }

  void render(SDL_Renderer *renderer) { grid.render(renderer); }
  void update(Interactable *interactable, Dialogue *dialogue) {
    grid.update(interactable, dialogue);
  };
  void handleEvents(const SDL_Event &event) { grid.handleEvents(event); }

private:
  Grid grid;

  float pointsize = 14.0f;
  SDL_Color fontColour = {255, 255, 255};
  SDL_Color selectColour = {208, 199, 125};
  SDL_Color dialogueBorderColour = {0, 0, 0};
  SDL_Color dialogueBoxColour = {51, 64, 113};

  fs::path assetsPath = fs::path(SDL_GetBasePath()) / "assets";
  std::string npcPortrait =
      (assetsPath / "portraits" / "npc_portrait.png").string();
};
