#pragma once
#include <SDL3/SDL.h>
#include <PLD/PLD_context.h>

typedef struct PLD_StartMenuState PLD_StartMenuState;

typedef enum PLD_StartMenuResult
{
    PLD_START_MENU_FAILURE,
    PLD_START_MENU_CONTINUE,
    PLD_START_MENU_PREVIOUS,
    PLD_START_MENU_NEXT
}PLD_StartMenuResult;

PLD_StartMenuState* PLD_LoadStartMenu(PLD_Context* context, bool first);

void PLD_QuitStartMenu(PLD_StartMenuState* menu);

PLD_StartMenuResult PLD_StartMenuEvent(PLD_Context* context, PLD_StartMenuState* menu, SDL_Event* event);

PLD_StartMenuResult PLD_StartMenuIterate(PLD_Context* context, PLD_StartMenuState* menu);