#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <PLD_sound.h>

#include <PLD_array_list.h>

#define PLD_SUCCESS 0
#define PLD_FAILURE 1

#define CHAIN_TIME_OUT 0.4

#define PLD_MENU_TEXT_WIDTH 750

typedef enum
{
    PLD_DIFFICULTY_NONE = -1,
    PLD_DIFFICULTY_EASY,
    PLD_DIFFICULTY_NORMAL,
    PLD_DIFFICULTY_HARD,
    PLD_DIFFICULTY_EXTREME,
	PLD_DIFFICULTY_EXTRA_EXTREME,
    PLD_DIFFICULTY_BASE,
    PLD_DIFFICULTY_MAX
}PLD_DifficultyType;

typedef struct
{
    bool autoplay;
    char* path;
    char* chartPath;
    int offset;
    PLD_Music* music;
    PLD_ArrayList* notes;
    PLD_ArrayList* currentNotes;
    PLD_DifficultyType difficulty;
    PLD_DataIni* dataIni;
}PLD_Song;

extern const char* PLD_DIFFICULTY_STRING[];
