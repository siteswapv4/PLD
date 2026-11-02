#include <PLD/PLD_result.h>

typedef struct PLD_Result
{
    Uint64 start_time;
    PLD_UserData user_data;
    PLD_Text* coolText;
    PLD_Text* fineText;
    PLD_Text* safeText;
    PLD_Text* sadText;
    PLD_Text* worstText;
    PLD_Text* comboText;
    PLD_Text* gradeText;
    PLD_Text* title;
    PLD_Image* background;
    PLD_Image* dashboard;
    PLD_Image* highscore;
}PLD_Result;

PLD_Result* PLD_InitResult(PLD_Context* context, PLD_Text* title, const PLD_UserData* user_data, bool highscore)
{
    PLD_Result* result = SDL_calloc(1, sizeof(PLD_Result));

    PLD_LoadImages(context, PLD_IMAGE_CATEGORY_RESULT);

    result->user_data = *user_data;

    result->coolText = PLD_CreateText(context, "COOL", 0);
    result->fineText = PLD_CreateText(context, "FINE", 0);
    result->safeText = PLD_CreateText(context, "SAFE", 0);
    result->sadText = PLD_CreateText(context, "SAD", 0);
    result->worstText = PLD_CreateText(context, "WORST", 0);
    result->comboText = PLD_CreateText(context, "COMBO", 0);
    if (user_data->grade != PLD_GRADE_INVALID)
        result->gradeText = PLD_CreateText(context, PLD_GetStringFromGrade(user_data->grade), 0);
    result->title = title;

    result->background = PLD_GetImage(PLD_RESULT_IMAGE_BACKGROUND);
    result->dashboard = PLD_GetImage(PLD_RESULT_IMAGE_DASHBOARD);

    PLD_SetTextColor(result->coolText, (SDL_Color){255, 255, 0, 255});
    PLD_SetTextColor(result->fineText, (SDL_Color){100, 100, 255, 255});
    PLD_SetTextColor(result->safeText, (SDL_Color){0, 128, 0, 255});
    PLD_SetTextColor(result->sadText, (SDL_Color){25, 25, 255, 255});
    PLD_SetTextColor(result->worstText, (SDL_Color){128, 0, 128, 255});
    PLD_SetTextColor(result->gradeText, (SDL_Color){200, 200, 200, 255});

    if (highscore)
        result->highscore = PLD_GetImage(PLD_RESULT_IMAGE_HIGHSCORE);

    result->start_time = SDL_GetTicks();

    return result;
}

PLD_ResultResult PLD_ResultLoop(PLD_Context* context, PLD_Result* result)
{
    float coeff = 0.0f;

    if (SDL_GetTicks() - result->start_time < 500)
    {
        coeff = 1.0f - ((SDL_GetTicks() - result->start_time) / 500.0f);
    }

    SDL_FRect rect = {0.0f, 0.0f, PLD_LOGICAL_WIDTH, PLD_LOGICAL_HEIGHT};
    PLD_RenderImage(context, result->background, NULL, &rect, false, 0, NULL, 255);
    PLD_RenderImage(context, result->dashboard, NULL, &rect, false, 0, NULL, 255);

    PLD_RenderText(context, result->title, (SDL_Point){PLD_LOGICAL_WIDTH / 2, (30 + PLD_FONT_SIZE / 2)}, PLD_LOGICAL_WIDTH - 100, 255, true, 0.0f);

    if (result->gradeText)
        PLD_RenderText(context, result->gradeText, (SDL_Point){705, 190}, 0, 255, true, 0.0f);

    int numbers[6] = {result->user_data.cool_count,
                      result->user_data.fine_count,
                      result->user_data.safe_count,
                      result->user_data.sad_count,
                      result->user_data.worst_count,
                      result->user_data.combo};

    PLD_Text* texts[6] = {result->coolText,
                          result->fineText,
                          result->safeText,
                          result->sadText,
                          result->worstText,
                          result->comboText};

    int x = 325;
    int y = 315;

    for (int i = 0; i < 6; i++)
    {
        PLD_RenderText(context, texts[i], (SDL_Point){x, y + 89 * i}, 0, 255, true, 0.0f);
        
        int digits = PLD_CountDigits(numbers[i]);
        PLD_RenderNumber(context, x + 575 - digits * 56, y + 89 * i - 23, numbers[i], 1.0, false);
    }

    PLD_RenderNumber(context, 510, 965, result->user_data.score, 1.0, true);

    if (result->highscore)
        PLD_RenderImage(context, result->highscore, NULL, &(SDL_FRect){700, 850, result->highscore->width, result->highscore->height}, false, 0, NULL, 255);

    PLD_DrawRectangle(context, &rect, 0, 0, 0, coeff * 255);

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
    PLD_DestroyImages(PLD_IMAGE_CATEGORY_RESULT);
    PLD_DestroyText(result->coolText);
    PLD_DestroyText(result->fineText);
    PLD_DestroyText(result->safeText);
    PLD_DestroyText(result->sadText);
    PLD_DestroyText(result->worstText);
    PLD_DestroyText(result->comboText);
    if (result->gradeText)
        PLD_DestroyText(result->gradeText);

    SDL_free(result);
}
