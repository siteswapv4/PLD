#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_note.h>
#include <PLD/PLD_evaluate.h>
#include <PLD/PLD_hold.h>
#include <PLD/PLD_sound.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_context.h>

void PLD_KeyPress(PLD_Context* context, PLD_ArrayList* currentNotes, int time, int pressedCode, bool keyboard);