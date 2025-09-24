#include "UI_utils.h"
#include <SDL3_image/SDL_image.h>

SDL_Texture* UI_LoadSVG(SDL_Renderer* renderer, const char* path, int width, int height)
{
    return UI_LoadSVG_IO(renderer, SDL_IOFromFile(path, "rb"), true, width, height);
}

SDL_Texture* UI_LoadSVG_IO(SDL_Renderer* renderer, SDL_IOStream* stream, bool close, int width, int height)
{
    if (!stream) { SDL_Log("Failed to load SVG : stream NULL"); return NULL; }
    SDL_Surface* surface = IMG_LoadSizedSVG_IO(stream, width, height);
    if (!surface) { goto error; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) { goto error; }
    SDL_DestroySurface(surface);

    if (close) { SDL_CloseIO(stream); }
    return texture;

error:
    SDL_Log("Failed to load SVG : %s", SDL_GetError());
    if (surface) { SDL_DestroySurface(surface); }
    if (texture) { SDL_DestroyTexture(texture); }
    if (stream && close) { SDL_CloseIO(stream); }
    return NULL;
}