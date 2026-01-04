// Pangolengine.h - Public API Header
#pragma once

//==============================================================================
// Core Engine
//==============================================================================
#include "Engine.h"
#include "IGame.h"
#include "Constants.h"

//==============================================================================
// ECS (Entity Component System)
//==============================================================================
#include "Components/ECS.h"
#include "Components/Components.h"
#include "Components/KeyboardController.h"

//==============================================================================
// Core Systems
//==============================================================================
#include "Camera.h"
#include "MapLoader.h"
#include "TextureManager.h"
#include "Collision.h"

//==============================================================================
// UI System
//==============================================================================
#include "UI/UIManager.h"
#include "UI/IUIManager.h"
#include "UI/IUIComponent.h"
#include "UI/UIComponents.h"
#include "UI/Grid.h"

//==============================================================================
// Parsers
//==============================================================================
#include "Parsers/JsonParser.h"
#include "Parsers/TsxParser.h"
#include "Parsers/Tokeniser.h"

//==============================================================================
// Math & Utilities
//==============================================================================
#include "Vector2D.h"

//==============================================================================
// SDL3 (Required for engine users)
//==============================================================================
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>

//==============================================================================
// Version Info
//==============================================================================
#define PANGOLENGINE_VERSION_MAJOR 0
#define PANGOLENGINE_VERSION_MINOR 1
#define PANGOLENGINE_VERSION_PATCH 0

namespace Pangolengine {
  inline const char* GetVersion() {
    return "0.1.0";
  }
}

//==============================================================================
// Usage Guide: 
//
// 1. Implement IGame interface: 
//    class MyGame : public IGame { ... };
//
// 2. Create main.cpp with SDL3 callbacks:
//    IGame* CreateGame(Engine* engine) { return new MyGame(engine); }
//    SDL_AppResult SDL_AppInit(... ) { ... }
//    
// 3. Link against pangolengine_lib in CMake:
//    target_link_libraries(mygame PRIVATE pangolengine_lib)
//
// See examples/demo/ for a complete working example. 
//==============================================================================
