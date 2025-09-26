#pragma once
#include <SDL3/SDL.h>
#include <PLD/PLD_array_list.h>

#define PLD_WRITE_CHECK(stream, value, size) \
    do { \
        if (SDL_WriteIO((stream), (value), (size)) != (size)) goto error; \
    } while (0)


#define PLD_READ_CHECK(stream, value, size) \
    do { \
        if (SDL_ReadIO((stream), (value), (size)) != (size)) goto error; \
    } while (0)


int PLD_Center(SDL_FRect* value);

int* PLD_DupInt(int value);

bool PLD_StartsWith(const char* source, const char* target);

bool PLD_EndsWith(const char* source, const char* target);

int PLD_ExtractSequence(SDL_IOStream* file, char** string, char* cut, int cutLen);

PLD_ArrayList* PLD_GetDirectoryContent(const char* path, bool onlyFiles, bool onlyDirs, bool concatPath);

char* PLD_GetLastOccurenceInString(const char* source, size_t length, char* targets);

bool PLD_RemoveExtension(char* source);

char* PLD_asprintf(const char* fmt, ...);

bool PLD_ShowDirectoryPicker(void (*callback)(void* userdata, const char* directory), void* userdata);