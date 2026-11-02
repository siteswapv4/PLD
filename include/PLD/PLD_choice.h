#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <PLD/PLD_context.h>

typedef enum PLD_ChoicePhase
{
    PLD_CHOICE_INVALID = -1,
    PLD_CHOICE_OPENING,
    PLD_CHOICE_OPENED,
    PLD_CHOICE_SWITCHING,
    PLD_CHOICE_CLOSING,
    PLD_CHOICE_CLOSED
}PLD_ChoicePhase;

typedef enum PLD_ChoiceDirection
{
    PLD_CHOICE_HORIZONTAL,
    PLD_CHOICE_VERTICAL
}PLD_ChoiceDirection;

typedef struct PLD_Choice PLD_Choice;

PLD_Choice* PLD_CreateChoice(PLD_Context* context, SDL_FPoint position, int width, const char* const* choices, int num_choices, PLD_ChoiceDirection direction);

void PLD_DestroyChoice(PLD_Choice* choice);

bool PLD_SetChoiceOptions(PLD_Context* context, PLD_Choice* choice, const char* const* const* options, const int* selected_options, const int* num_options);

bool PLD_OpenChoice(PLD_Choice* choice, Uint64 time);

bool PLD_CloseChoice(PLD_Choice* choice, Uint64 time);

bool PLD_RenderChoice(PLD_Context* context, PLD_Choice* choice, Uint64 time);

PLD_ChoicePhase PLD_GetChoicePhase(PLD_Choice* choice);

int PLD_GetSelectedChoice(PLD_Choice* choice);

bool PLD_SetSelectedChoice(PLD_Choice* choice, int index, Uint64 time);

int PLD_GetSelectedOption(PLD_Choice* choice);

bool PLD_SetSelectedOption(PLD_Choice* choice, int index, Uint64 time);