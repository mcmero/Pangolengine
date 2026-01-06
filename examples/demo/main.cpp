#include "Pangolengine.h"
#include "DemoGame.h"
#include "Constants.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// Game-specific creation function
IGame* CreateGame(Engine* engine) {
  return new DemoGame(engine);
}

struct AppContext {
  Engine* engine;
  IGame* game;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  Engine* engine = new Engine(
    nullptr,
    "Pangolengine Demo",
    SCREEN_WIDTH,
    SCREEN_HEIGHT
  );

  IGame* game = CreateGame(engine);
  engine->setGameImpl(game);

  if (!engine->initialise()) {
    delete engine;
    delete game;
    return SDL_APP_FAILURE;
  }

  auto* app = new AppContext{engine, game};
  *appstate = app;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  auto* app = (AppContext*)appstate;
  app->engine->handleEvent(event);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  auto* app = (AppContext*)appstate;

  if (! app->engine->isRunning()) {
      return SDL_APP_SUCCESS;
  }

  auto frameStart = SDL_GetTicks();

  app->engine->iterate();

  // Frame rate limiting (target 60 FPS)
  const float targetFrameTime = 1000.0f / 60.0f;  // 16.66ms
  auto frameTime = SDL_GetTicks() - frameStart;
  
  if (targetFrameTime > frameTime) {
      SDL_Delay(static_cast<Uint32>(targetFrameTime - frameTime));
  }

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  auto* app = (AppContext*)appstate;
  delete app->engine;
  delete app->game;
  delete app;
}
