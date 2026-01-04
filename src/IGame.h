#pragma once

#include "SDL3/SDL_events.h"

class IGame {
public:
    virtual ~IGame() = default;
    virtual bool onInitialise() = 0;
    virtual void onEvent(SDL_Event* event) = 0;
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
    virtual void onCleanup() = 0;
};

