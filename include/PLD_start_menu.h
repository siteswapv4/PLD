#pragma once
#include <SDL3/SDL.h>
#include "PLD_context.h"

typedef struct PLD_StartMenu PLD_StartMenu;

typedef enum PLD_StartMenuResult
{
    PLD_START_MENU_INVALID = -1,
    PLD_START_MENU_CONTINUE,
    PLD_START_MENU_PREVIOUS,
    PLD_START_MENU_NEXT
}PLD_StartMenuResult;

PLD_StartMenu* PLD_LoadStartMenu(PLD_Context* context, bool first);

void PLD_QuitStartMenu(PLD_StartMenu* menu);

PLD_StartMenuResult PLD_StartMenuKeyPress(PLD_Context* context, PLD_StartMenu* menu, SDL_Event* event);

PLD_StartMenuResult PLD_StartMenuLoop(PLD_Context* context, PLD_StartMenu* menu);