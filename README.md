# PLD

## Introduction

PLD (Project Light Diva) is a fangame inspired by Project Diva, and particularly PPD (Project Project Dxxx).
The goal of this project is to make a light and portable chart player that most people can enjoy, even with weak hardware.

## Supported Platforms

PLD compiles and runs for

- PSP
- PSVITA
- Linux
- Windows

It should compile and run on all other platforms supported by SDL2, others are not tested yet.

## To-Do List

- [ ] Console Style notes
- [ ] DSC file support
- [ ] Menus and UI
- [ ] Chart downloader (better than a python script)

## Compile

PLD depends on

- [SDL2](https://github.com/libsdl-org/SDL)
- [SDL2_image](https://github.com/libsdl-org/SDL_image)
- [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf)

### PSP

Install [PSPDEV](https://github.com/pspdev/pspdev), get SDL libs (and libxmp) with psp-pacman, then run the PSP Makefile

### PSVITA

Install the [VITA SDK](https://vitasdk.org/) then run
```
cmake . && make
```

### Windows

Get [MinGW-w64/w32 | std >= c11](https://winlibs.com/), get the SDL2 dev resources, get [dirent.h](https://github.com/tronkko/dirent), then run (in powershell)
```
gcc src/*.c -o PLD.exe -I "include" -L "lib" -lmingw32 -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -D_POSIX_C_SOURCE=200809L -std=c11 -municode -mwindows
```

### Linux

Get gcc and SDL2 packages then run
```
gcc src/*.c -o PLD -I "include" -lm -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
```
