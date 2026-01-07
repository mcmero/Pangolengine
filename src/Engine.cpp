#include "Engine.h"
#include "Constants.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "SDL3_mixer/SDL_mixer.h"

// Define static members
SDL_Renderer* Engine::renderer = nullptr;
MapData Engine::mapData = {};
int Engine::mapPixelHeight = 320;
int Engine::mapPixelWidth = 180;
EntityId Engine::playerId = 0;
EntityId Engine::mapId = 0;

Engine::Engine(const char* windowTitle, int width, int height)
    : window(nullptr), 
      windowTitle(windowTitle), windowWidth(width),
      windowHeight(height), running(true) {}

Engine::~Engine() {
  cleanup();
}

bool Engine::initialise(IGame *game) {
  if (!game) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "Game implementation required!"
    );
    return false;
  }
  this->gameImpl = game;

  // SDL initialisation
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "SDL_Init Error: %s", SDL_GetError()
    );
    return false;
  }

  // TTF (font support) initialisation
  if (!TTF_Init()) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "TTF_Init Error: %s", SDL_GetError()
    );
    return false;
  }

  // SDL window creation
  window = SDL_CreateWindow(
    windowTitle, 
    windowWidth * DEFAULT_RENDER_SCALE,
    windowHeight * DEFAULT_RENDER_SCALE, 
    SDL_WINDOW_RESIZABLE
  );
  if (!window) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "Window creation Error: %s",
      SDL_GetError()
    );
    return false;
  }

  // Create renderer
  renderer = SDL_CreateRenderer(window, NULL);
  if (!renderer) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "Renderer creation Error: %s",
      SDL_GetError()
    );
    return false;
  }
  SDL_SetRenderScale(renderer, DEFAULT_RENDER_SCALE, DEFAULT_RENDER_SCALE);

  // Print some information about the window
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

  // Init audio
  auto audioDevice = SDL_OpenAudioDevice(
    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL
  );
  if (!audioDevice) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "Audio device Error: %s",
      SDL_GetError()
    );
    return false;
  }

  if (!Mix_OpenAudio(audioDevice, NULL)) {
    SDL_LogError(
      SDL_LOG_CATEGORY_CUSTOM,
      "Mix_OpenAudio Error: %s",
      SDL_GetError()
    );
    return false;
  }

  // Initialise game implementation
  if (!gameImpl->onInitialise())
    return false;

  SDL_Log("Engine initialized successfully!");
  return true;
}

void Engine::handleEvent(SDL_Event* event) {
  if (event->type == SDL_EVENT_QUIT)
    quit();
  else
    gameImpl->onEvent(event);
}

void Engine::iterate() {
  if (!running)
    return;

  // Update
  gameImpl->onUpdate();

  // Render
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  gameImpl->onRender();

  SDL_RenderPresent(renderer);
}

void Engine::cleanup() {
  gameImpl->onCleanup();

  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }

  Mix_CloseAudio();
  TTF_Quit();
  SDL_Quit();

  SDL_Log("Engine cleaned up successfully!");
}

void Engine::quit() {
  running = false;
  SDL_Log("Engine quit requested");
}
