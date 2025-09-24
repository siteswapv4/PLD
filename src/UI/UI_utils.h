#pragma once
#include <SDL3/SDL.h>

#define UI_READ_CHECK(stream, value, size) if (SDL_ReadIO(stream, value, size) != size) goto error;

#define UI_WRITE_CHECK(stream, value, size) if (SDL_WriteIO(stream, value, size) != size) goto error;

SDL_Texture* UI_LoadSVG(SDL_Renderer* renderer, const char* path, int width, int height);

SDL_Texture* UI_LoadSVG_IO(SDL_Renderer* renderer, SDL_IOStream* stream, bool close, int width, int height);