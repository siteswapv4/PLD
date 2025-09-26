#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_evaluate.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_context.h>

#define PLD_HOLD_TIMEOUT 5000

#define PLD_HOLD_POINTS     10
#define PLD_HOLD_MAX_BONUS  1500
#define PLD_HOLD_MAX_FRAMES 300


int PLD_InitHolds();

int PLD_DropHolds();

int PLD_IsHeld(int type);

int PLD_AddHoldKey(int type, int key, bool keyboard);

int PLD_SetHoldTimeout(int time);

int PLD_ProcessHolds(PLD_Context* context, int time);

int PLD_RenderHolds(PLD_Context* context, int time);

int PLD_FreeHolds();
