#pragma once
#include <SDL3/SDL.h>


#define PLD_WRITE_CHECK(stream, value, size) \
    do { \
        if (SDL_WriteIO((stream), (value), (size)) != (size)) goto error; \
    } while (0)


#define PLD_READ_CHECK(stream, value, size) \
    do { \
        if (SDL_ReadIO((stream), (value), (size)) != (size)) goto error; \
    } while (0)
