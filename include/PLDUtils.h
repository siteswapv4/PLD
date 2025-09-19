#pragma once
#include <PLD_context.h>

int PLD_Center(SDL_FRect* value);

int PLD_GetController(PLD_Context* context);

int PLD_EventLoop(PLD_Context* context, SDL_Event* event);

int* PLD_DupInt(int value);

bool PLD_StartsWith(const char* source, const char* target);

bool PLD_EndsWith(const char* source, const char* target);

int PLD_ExtractSequence(SDL_IOStream* file, char** string, char* cut, int cutLen);

PLD_ArrayList* PLD_GetDirectoryContent(const char* path, bool onlyFiles, bool onlyDirs, bool concatPath);

PLD_DataIni* PLD_LoadDataIni(const char* path);

void PLD_RenderTouch(PLD_Context* context);

char* PLD_GetLastOccurenceInString(const char* source, size_t length, char* targets);

bool PLD_RemoveExtension(char* source);

char* PLD_asprintf(const char* fmt, ...);

bool PLD_ShowDirectoryPicker(void (*callback)(void* userdata, const char* directory), void* userdata);