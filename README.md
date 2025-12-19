## Pangolengine: a 2D top-down game engine with dialogue trees

Pangolengine is a simple 2D top-down game engine inspired by the
[BirchEngine](https://github.com/carlbirch/BirchEngine) built using
[SDL3](https://wiki.libsdl.org/SDL3/FrontPage) and its libraries, with no other
dependencies.

The engine leverages the [Tiled Map Editor](https://thorbjorn.itch.io/tiled)
heavily, which can be used to design levels that can easily be loaded into
the game engine. Simple dialogue trees can be written via [Google
Sheets](https://docs.google.com/spreadsheets) and attached to NPCs.

The primary aim of the project was to learn C++, object-oriented programming and
the structure of simple game engines. It is primarily meant as an educational
project and is not a fully-fledged game engine.

See [src/Game.cpp](src/Game.cpp) as a starting point to understanding the main
game loop.

### Building And Running

If you are a novice, the setup instructions are very similar to the
[sdl3-sample wiki](https://github.com/Ravbug/sdl3-sample/wiki/Setting-up-your-computer),
(this engine is built off this template). Make sure to substitute the
sdl3-sample repository for this one if following those instructions.

If you are familiar with using the command line and have Cmake and a C++
compiler installed, you can use the commands below:

```sh
# You need to clone with submodules, otherwise SDL will not download.
git clone https://github.com/mcmero/Pangolengine --depth=1 --recurse-submodules
cd Pangolengine
cmake -S . -B build
cmake --build build --parallel --target pangolengine
```
You can also use an init script inside [`config/`](config/). Then open the IDE project inside `build/` 
(If you had CMake generate one) and run the executable under `build/Debug/`.

## Supported Platforms

The [sdl3-sample](https://github.com/Ravbug/sdl3-sample)
that this engine is based off has support for a [number of platforms](https://github.com/Ravbug/sdl3-sample?tab=readme-ov-file#supported-platforms).
For the simplicity of my development process however, I am currently only
supporting Windows, but in theory it should be possible to compile easily on
Linux, Mac OS and web via wasm*.

*See further instructions in [`config/`](config/)

## Updating SDL and its libraries

Just update the submodule, for example:
```sh
cd SDL
git pull
```
You don't need to use a submodule, you can also copy the source in directly.
This repository uses a submodule to keep its size to a minimum.

## Making compatible maps in Tiled

This engine supports maps made via the [Tiled Map
Editor](https://thorbjorn.itch.io/tiled) (I've only tested v1.11.0). The
[Objects.tsx](assets/tilesets/Objects.tsx) file contains the sprite objects and
[TilemapOutdoor.tsx](assets/tilesets/TilemapOutdoor.tsx) contains the tile map.
Sprite objects support (only rectangular) collision boxes. (See the [Tiled
docs](https://doc.mapeditor.org/en/stable/manual/objects/) for more info).

## Reporting issues
Is something not working? Create an Issue or send a Pull Request on this repository!

## References and resources

- [sdl3-sample](https://github.com/Ravbug/sdl3-sample) -- template used to make
  this project.
- [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) -- for more information and
documentation on SDL3.
- [Tiled Map Editor](https://thorbjorn.itch.io/tiled) -- used to make
  the maps in this game.
- [Aseprite](https://www.aseprite.org/) -- sprite and pixel-art editor I used to
  make the sprite assets.
- [Furnace](https://github.com/tildearrow/furnace) -- a chip-tune tracker that I
  used to make the sound assets.
- [Let's Make
Games](https://www.youtube.com/playlist?list=PLhfAbcv9cehhkG7ZQK0nfIGJC_C-wSLrx)
-- A great series of tutorials if you want to get started making a game engine
with SDL (note that this series uses SDL2).

