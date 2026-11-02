#pragma once
#include <SDL3/SDL.h>
#include <PLD/PLD_context.h>

typedef struct PLD_Music PLD_Music;

bool PLD_IsSupportedMusic(const char* path);

bool PLD_InitMusic(PLD_Context* context);

void PLD_QuitMusic();

PLD_Music* PLD_LoadMusic(PLD_Context* context, const char* path);

bool PLD_SetMusicPosition(PLD_Music* music, int position); // in ms

int PLD_GetMusicPosition(PLD_Music* music); // in ms

bool PLD_PlayMusic(PLD_Context* context, PLD_Music* music);

bool PLD_StopMusic(PLD_Music* music);

void PLD_PauseMusic(PLD_Music* music);

void PLD_ResumeMusic(PLD_Music* music);

bool PLD_MusicIsPaused(PLD_Music* music);

void PLD_DestroyMusic(PLD_Music* music);

void PLD_SetMusicVolume(float volume);

void PLD_UpdateMusic();