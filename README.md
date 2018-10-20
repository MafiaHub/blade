# Blade Engine

Blade engine is a generic-purpose game engine used for developing games in classic Quake 1/2 vibe. It offers extra stability, 
as well as features more suitable for today's world of gaming. 

## Features

- Blade engine is a fork of Yamagi Quake 2, which consists of many bugfixes and QoL improvements.
- OpenGL 3 support.
- Full-color support, game engine drops .WAL support (except for animated textures)
- Simplified map entity models. Map models only need to start with `model_` prefix to be loaded.
- Map entity code improvements.
- Quake 2 specific code cleanup. This engine is more generic, hence the reason of quake2 specific code removal.
- Mirror entity implementation (WIP)
- OpenGL Render-To-Texture support and Post-FX (WIP)
- Fog support (easily tweakable.) Fog is configurable via cvars and `worldspawn` entity.
- Modular weapon ammo counter visualisation (change weapon skin based on ammo status.)
- Weapon reloading.
- Revamped weapon and item system.
- Inventory management, ability to assign items to quickbar.

Note that this engine is specifically made for a game under active development. While it might work without our WIP assets, the process is not optimal.
If you want to participate in development of the game/game engine, please let me know.

## License

This code is build upon Blade Engine, which itself is based on Yamagi Quake II. 
Blade Engine is released under the terms of the GPL version 2. See the
LICENSE file for further information.

## Installation

Blade Engine is installed in 2 steps:

- Game data setup.
- Download and extract the executables.

### Game data Setup

There is currently no game data available at the moment.

Quake 2 game data and/or associated mods are not supported!

### Download and extract the executables

How the Blade Engine executables are installed depends on the platform:

- For Windows a package with all Blade Engine executables *will be* provided.
- There is no Linux or *BSD package available so far;
- On OS X you need to compile from source.
- Of course Blade Engine can be compiled from source on all platforms.


#### Microsoft Windows

1. Get the latest release from your game archive.
2. Extract it into the gamedata directory created above. blade.exe must be
   placed next to the baseb/ subdirectory.

On Windows Blade Engine is fully portable, you can move the installation
directory wherever and whenever you like. To update Blade Engine just
overwrite the old files with the new ones.


#### Binary Package from your Linux distribution or BSD system

TBD

### Compiling from source

To compile Blade Engine from source, you need the following dependencies,
including development headers:

- A GCC-compatible compiler like GCC, MinGW (see below) or clang.
- GNU make.
- A libGL implementation with OpenGL system headers.
- SDL 2.0 or SDL 1.2 (2.0 recommended, edit the Makefile to use 1.2 instead).
- libogg, libvorbis and libvorbisfile.
- A OpenAL implementation, openal-soft is highly recommended.
- zlib.

#### On Linux distribution or BSD systems

On debian based distributions (including Ubuntu and Mint) the dependencies can
be installed with: `apt-get install build-essential libgl1-mesa-dev libsdl2-dev
libogg-dev libvorbis-dev libopenal-dev zlib1g-dev`

On OS X we recommend using [homebrew](https://brew.sh) to install the
dependencies: `brew install sdl2 libvorbis libogg openal-soft`

On FreeBSD you'll need something like: `pkg install gmake libGL sdl2 libogg
libvorbis openal-soft`


#### On Windows

On Windows a MinGW environment is needed. A preinstalled environment with all
dependencies can be found at https://madaraszd.net/blade/
Just extract it into C:\MSYS2\ and start either the 32 bit or 64 bit version
through *C:\MSYS2\msys32.exe* or *C:\MSYS2\msys64.exe*. With the preinstalled
MinGW environment GNU Make is highly recommended.


#### Compiling

Change into the code/ source directory and type *make*. 
After that copy everything from the release/
directory to your Game installation directory.
