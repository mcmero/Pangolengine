#include "Constants.h"
#include "Game.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "SDL3_ttf/SDL_ttf.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdint>

struct AppContext {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_AppResult app_quit = SDL_APP_CONTINUE;
  Game game;
};

SDL_AppResult SDL_Fail() {
  SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  // init the library, here we make a window so we only need the Video
  // capabilities.
  if (not SDL_Init(SDL_INIT_VIDEO)) {
    return SDL_Fail();
  }

  if (not TTF_Init()) {
    return SDL_Fail();
  }

  // create a window
  SDL_Window *window =
      SDL_CreateWindow("Window", SCREEN_WIDTH * RENDER_SCALE,
                       SCREEN_HEIGHT * RENDER_SCALE, SDL_WINDOW_RESIZABLE);
  if (not window) {
    return SDL_Fail();
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  if (not renderer) {
    return SDL_Fail();
  }

  SDL_SetRenderScale(renderer, RENDER_SCALE, RENDER_SCALE);

  // print some information about the window
  SDL_ShowWindow(window);
  {
    int width, height, bbwidth, bbheight;
    SDL_GetWindowSize(window, &width, &height);
    SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
    SDL_Log("Window size: %ix%i", width, height);
    SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
    if (width != bbwidth) {
      SDL_Log("This is a highdpi environment.");
    }
  }

  // Create the AppContext and initialize the game
  auto *app = new AppContext{window, renderer};
  entt::registry registry;

  if (!app->game.initialise(window, renderer)) {
    return SDL_Fail();
  }

  *appstate = app;
  SDL_Log("Application started successfully!");

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  auto *app = (AppContext *)appstate;

  if (event->type == SDL_EVENT_QUIT) {
    app->app_quit = SDL_APP_SUCCESS;
  } else {
    app->game.handleEvents(event);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  auto *app = (AppContext *)appstate;

  auto frameStart = SDL_GetTicks();
  float deltaTime = 1.0f / 60.0f;
  float frameDelay = 1000.0f * deltaTime;

  app->game.update();
  app->game.render();

  auto frameTime = SDL_GetTicks() - frameStart;

  if (frameDelay > frameTime) {
    SDL_Delay(static_cast<uint32_t>(frameDelay - frameTime));
  }

  return app->app_quit;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  auto *app = (AppContext *)appstate;
  app->game.clean();
  if (app) {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    delete app;
  }

  SDL_Log("Application quit successfully!");
}
