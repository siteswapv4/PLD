#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_note.h>
#include <PLD/PLD_sound.h>
#include "PLD/PLD_effect.h"
#include <PLD/PLD_text.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_context.h>

#define PLD_EVALUATE_POS 100
#define PLD_EVALUATE_TIMEOUT 500
#define PLD_DIGIT_MAX 7

#define PLD_COOL_POINTS 500
#define PLD_FINE_POINTS 300
#define PLD_SAFE_POINTS 100
#define PLD_SAD_POINTS 50
#define PLD_WRONG_COOL_POINTS 250
#define PLD_WRONG_FINE_POINTS 150
#define PLD_WRONG_SAFE_POINTS 50
#define PLD_WRONG_SAD_POINTS 30
#define PLD_WORST_POINTS 0

#define PLD_CHAIN_BIT_POINTS 10
#define PLD_CHAIN_MAX_BONUS 1000

#define PLD_COOL_TIMING 40
#define PLD_FINE_TIMING 80
#define PLD_SAFE_TIMING 120
#define PLD_SAD_TIMING 150

typedef enum
{
    PLD_EVALUATE_COOL,
    PLD_EVALUATE_FINE,
    PLD_EVALUATE_SAFE,
    PLD_EVALUATE_SAD,
    PLD_EVALUATE_WORST
}PLD_EvaluateType;

int PLD_InitEvaluate();

int PLD_RenderScore(PLD_Context* context, float foreground_coeff);

int PLD_RenderNumber(PLD_Context* context, int posx, int posy, int number, double scale, bool complete);

int PLD_RenderEvaluate(PLD_Context* context, int time);

bool PLD_EvaluateNote(PLD_Note* note, int time, bool isHit);

int PLD_FreeEvaluate();

void PLD_GetEvaluates(int* cool, int* fine, int* safe, int* sad, int* worst, int* score, int* combo);

void PLD_AddScore(int value);