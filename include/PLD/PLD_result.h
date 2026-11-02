#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_evaluate.h>
#include <PLD/PLD_text.h>
#include <PLD/PLD_menu.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_user.h>


typedef enum
{
    PLD_RESULT_INVALID,
    PLD_RESULT_CONTINUE,
    PLD_RESULT_SUCCESS,
    PLD_RESULT_COUNT
}PLD_ResultResult; // ok bad name

typedef struct PLD_Result PLD_Result;

PLD_Result* PLD_InitResult(PLD_Context* context, PLD_Text* title, const PLD_UserData* user_data, bool highscore);

PLD_ResultResult PLD_ResultLoop(PLD_Context* context, PLD_Result* result);

PLD_ResultResult PLD_ResultEvent(PLD_Context* context, PLD_Result* result, SDL_Event* event);

void PLD_QuitResult(PLD_Result* result);