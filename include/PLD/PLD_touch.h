#pragma once
#include <SDL3/SDL.h>

#include <PLD/PLD_array_list.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_utils.h>

extern PLD_ArrayList* PLD_touch_images;

bool PLD_InitTouch(PLD_Context* context);

void PLD_RenderTouch(PLD_Context* context, bool force);

void PLD_TouchEvent(PLD_Context* context, SDL_Event* event);

void PLD_ResetTouchState(PLD_Context* context);

void PLD_QuitTouch();