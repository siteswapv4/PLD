#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef enum UI_ChoicePhase
{
    UI_CHOICE_INVALID = -1,
    UI_CHOICE_OPENING,
    UI_CHOICE_OPENED,
    UI_CHOICE_SWITCHING,
    UI_CHOICE_CLOSING,
    UI_CHOICE_CLOSED
}UI_ChoicePhase;

typedef enum UI_ChoiceDirection
{
    UI_CHOICE_HORIZONTAL,
    UI_CHOICE_VERTICAL
}UI_ChoiceDirection;

typedef struct UI_Choice UI_Choice;

UI_Choice* UI_CreateChoice(SDL_Renderer* renderer, SDL_FPoint position, int width, TTF_TextEngine* engine, TTF_Font* font, const char* const* choices, int num_choices, UI_ChoiceDirection direction);

void UI_DestroyChoice(UI_Choice* choice);

bool UI_SetChoiceOptions(UI_Choice* choice, TTF_TextEngine* engine, TTF_Font* font, const char* const* const* options, const int* selected_options, const int* num_options);

bool UI_OpenChoice(UI_Choice* choice, Uint64 time);

bool UI_CloseChoice(UI_Choice* choice, Uint64 time);

bool UI_RenderChoice(SDL_Renderer* renderer, UI_Choice* choice, Uint64 time);

UI_ChoicePhase UI_GetChoicePhase(UI_Choice* choice);

int UI_GetSelectedChoice(UI_Choice* choice);

bool UI_SetSelectedChoice(UI_Choice* choice, int index, Uint64 time);

int UI_GetSelectedOption(UI_Choice* choice);

bool UI_SetSelectedOption(UI_Choice* choice, int index, Uint64 time);