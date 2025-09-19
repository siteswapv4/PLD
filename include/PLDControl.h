#pragma once
#include "PLD.h"
#include "PLD_image.h"
#include "PLDNotes.h"
#include "PLDEvaluate.h"
#include "PLDHolds.h"
#include <PLD_sound.h>
#include "PLDUtils.h"

#include <PLD_context.h>

void PLD_KeyPress(PLD_Context* context, PLD_ArrayList* currentNotes, int time, int pressedCode, bool keyboard);