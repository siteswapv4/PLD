#pragma once
#include <SDL3/SDL.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_utils.h>

typedef struct PLD_TouchConfigState PLD_TouchConfigState;

typedef enum PLD_TouchConfigResult
{
    PLD_TOUCH_CONFIG_FAILURE,
    PLD_TOUCH_CONFIG_SUCCESS,
    PLD_TOUCH_CONFIG_CONTINUE
}PLD_TouchConfigResult;

PLD_TouchConfigState* PLD_InitTouchConfig(PLD_Context* context);

void PLD_QuitTouchConfig(PLD_Context* context, PLD_TouchConfigState* config);

PLD_TouchConfigResult PLD_TouchConfigEvent(PLD_Context* context, PLD_TouchConfigState* config, SDL_Event* event);

PLD_TouchConfigResult PLD_TouchConfigIterate(PLD_Context* context, PLD_TouchConfigState* config);