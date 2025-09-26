#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_evaluate.h>
#include <PLD/PLD_text.h>
#include <PLD/PLD_menu.h>
#include <PLD/PLD_context.h>


typedef enum
{
    PLD_RESULT_INVALID,
    PLD_RESULT_CONTINUE,
    PLD_RESULT_SUCCESS,
    PLD_RESULT_COUNT
}PLD_ResultResult; // ok bad name


typedef struct PLD_Result
{
    int start_time;
    int cool;
    int fine;
    int safe;
    int sad;
    int worst;
    int score;
    int combo;
    PLD_Text* coolText;
    PLD_Text* fineText;
    PLD_Text* safeText;
    PLD_Text* sadText;
    PLD_Text* worstText;
    PLD_Text* scoreText;
    PLD_Text* comboText;
    PLD_Text* title;
    PLD_Image* background;
}PLD_Result;


PLD_Result* PLD_InitResult(PLD_Context* context, const char* backgroundPath, PLD_Text* title);

PLD_ResultResult PLD_ResultLoop(PLD_Context* context, PLD_Result* result);

PLD_ResultResult PLD_ResultEvent(PLD_Context* context, PLD_Result* result, SDL_Event* event);

void PLD_QuitResult(PLD_Result* result);