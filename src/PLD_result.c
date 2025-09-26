#include <PLD/PLD_result.h>


PLD_Result* PLD_InitResult(PLD_Context* context, const char* backgroundPath, PLD_Text* title)
{
    PLD_Result* result = SDL_calloc(1, sizeof(PLD_Result));

    PLD_GetEvaluates(&result->cool, &result->fine, &result->safe, &result->sad, &result->worst, &result->score, &result->combo);

    result->coolText = PLD_CreateText(context, "COOL");
    result->fineText = PLD_CreateText(context, "FINE");
    result->safeText = PLD_CreateText(context, "SAFE");
    result->sadText = PLD_CreateText(context, "SAD");
    result->worstText = PLD_CreateText(context, "WORST");
    result->scoreText = PLD_CreateText(context, "SCORE");
    result->comboText = PLD_CreateText(context, "COMBO");
    result->title = title;

    if (backgroundPath != NULL)
    {
        result->background = PLD_LoadImage(context, backgroundPath);
    }

    PLD_SetTextColor(result->coolText, (SDL_Color){255, 255, 0, 255});
    PLD_SetTextColor(result->fineText, (SDL_Color){100, 100, 255, 255});
    PLD_SetTextColor(result->safeText, (SDL_Color){0, 128, 0, 255});
    PLD_SetTextColor(result->sadText, (SDL_Color){25, 25, 255, 255});
    PLD_SetTextColor(result->worstText, (SDL_Color){128, 0, 128, 255});

    result->start_time = SDL_GetTicks();

    return result;
}

PLD_ResultResult PLD_ResultLoop(PLD_Context* context, PLD_Result* result)
{
    float coeff = 1.0f;

    if (SDL_GetTicks() - result->start_time < 100)
    {
        coeff = (SDL_GetTicks() - result->start_time) / 100.0f;
    }

    SDL_FRect rect = {50, PLD_LOGICAL_HEIGHT / 2 - (PLD_LOGICAL_HEIGHT / 2 - 50) * coeff, PLD_LOGICAL_WIDTH - 100, (PLD_LOGICAL_HEIGHT - 100) * coeff};

    PLD_RenderImageLetterbox(context, result->background);

    SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 210);
    SDL_RenderFillRect(context->renderer, &rect);

    PLD_RenderText(context, result->title, PLD_LOGICAL_WIDTH / 2, (100 + PLD_FONT_SIZE / 2), PLD_LOGICAL_WIDTH - 100, 255, true);

    int posx_1 = 650;
    int posx_2 = 850;

    PLD_RenderText(context, result->coolText, posx_1, 272, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 250, result->cool, 1.0, true);

    PLD_RenderText(context, result->fineText, posx_1, 372, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 350, result->fine, 1.0, true);

    PLD_RenderText(context, result->safeText, posx_1, 472, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 450, result->safe, 1.0, true);

    PLD_RenderText(context, result->sadText, posx_1, 572, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 550, result->sad, 1.0, true);

    PLD_RenderText(context, result->worstText, posx_1, 672, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 650, result->worst, 1.0, true);

    PLD_RenderText(context, result->comboText, posx_1 , 772, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 750, result->combo, 1.0, true);

    PLD_RenderText(context, result->scoreText, posx_1, 872, PLD_LOGICAL_WIDTH, 255, false);
    PLD_RenderNumber(context, posx_2, 850, result->score, 1.0, true);

    return PLD_RESULT_CONTINUE;
}


PLD_ResultResult PLD_ResultEvent(PLD_Context* context, PLD_Result* result, SDL_Event* event)
{
    PLD_MenuInput input = PLD_GetMenuPressedInput(context, event);

    switch (input)
    {
        case PLD_MENU_INPUT_EAST:
            return PLD_RESULT_SUCCESS;
            break;

        default:
            break;
    }

    return PLD_RESULT_CONTINUE;
}


void PLD_QuitResult(PLD_Result* result)
{
    PLD_DestroyText(result->coolText);
    PLD_DestroyText(result->fineText);
    PLD_DestroyText(result->safeText);
    PLD_DestroyText(result->sadText);
    PLD_DestroyText(result->worstText);
    PLD_DestroyText(result->scoreText);
    PLD_DestroyText(result->comboText);
    PLD_DestroyImage(result->background);

    SDL_free(result);
}