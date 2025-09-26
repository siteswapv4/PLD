#include <PLD/PLD_evaluate.h>


typedef struct
{
    int combo;
    int maxCombo;
    int coolCount;
    int fineCount;
    int safeCount;
    int sadCount;
    int worstCount;
    int score;
    int numSize;
    int timeOut;
    PLD_Image* image;
    PLD_Image* score_image;
    SDL_FRect rect;
    SDL_FRect** scoreRect;
}PLD_Evaluate;

PLD_Evaluate* evaluate = NULL;

int PLD_InitEvaluate()
{
    int scoreWidth;

    evaluate = SDL_calloc(1, sizeof(PLD_Evaluate));

    evaluate->score_image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_NUM);
    evaluate->scoreRect = SDL_calloc(PLD_DIGIT_MAX, sizeof(SDL_FRect*));
        
    scoreWidth = evaluate->score_image->width;
    evaluate->numSize = evaluate->score_image->height;

    for (int i = 0; i < PLD_DIGIT_MAX; i++)
    {
        SDL_FRect rect = {1500 + (i * (scoreWidth / 10)), 40, evaluate->numSize, evaluate->numSize};
        evaluate->scoreRect[i] = SDL_calloc(2, sizeof(SDL_FRect));
        evaluate->scoreRect[i][1] = rect;
    }

    return PLD_SUCCESS;
}

int PLD_RenderNumber(PLD_Context* context, int posx, int posy, int number, double scale, bool complete)
{
    int count = 0;

    SDL_FRect numberRect[PLD_DIGIT_MAX];

    while ((number != 0) && (count < PLD_DIGIT_MAX))
    {
        SDL_FRect rect = {(number % 10) * evaluate->numSize, 0, evaluate->numSize, evaluate->numSize};
        numberRect[count] = rect;

        count++;
        number /= 10;
    }

    if (complete)
    {
        for (int i = 0; i < PLD_DIGIT_MAX - count; i++)
        {
            SDL_FRect src = {0, 0, evaluate->numSize, evaluate->numSize};
            SDL_FRect res = {posx, posy, evaluate->numSize * scale, evaluate->numSize * scale};

            PLD_RenderImage(context, evaluate->score_image, &src, &res, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

            posx += evaluate->numSize * scale;
        }
    }

    for (int i = 0; i < count; i++)
    {
        SDL_FRect res = {posx, posy, evaluate->numSize * scale, evaluate->numSize * scale};

        PLD_RenderImage(context, evaluate->score_image, &numberRect[count - 1 - i], &res, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

        posx += evaluate->numSize * scale;
    }

    return PLD_SUCCESS;
}

int PLD_RenderScore(PLD_Context* context, float foreground_coeff)
{
    PLD_RenderNumber(context, 1500, 10 - ((10 + PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP)->height) * foreground_coeff), evaluate->score, 1, true);

    return PLD_SUCCESS;
}

int PLD_RenderEvaluate(PLD_Context* context, int time)
{
    if (time < evaluate->timeOut)
    {
        float scale = 0.9f;
        SDL_FRect rect = evaluate->rect;

        if ((evaluate->timeOut - time >= PLD_EVALUATE_TIMEOUT - 100) && (evaluate->timeOut - time <= PLD_EVALUATE_TIMEOUT - 50))
        {
            rect.w *= 1.1f;
            rect.h *= 1.1f;
            scale *= 1.1f;
        }

        if (evaluate->image != NULL)
        {
            PLD_RenderImage(context, evaluate->image, NULL, &rect, true, 0, NULL, PLD_IMAGE_ALPHA_FULL);
        }

        if (evaluate->combo >= 5)
        {
            PLD_RenderNumber(context, rect.x + rect.w / 2 + 5, rect.y - rect.h / 2, evaluate->combo, scale, false);
        }
    }

    return PLD_SUCCESS;
}

bool PLD_EvaluateNote(PLD_Note* note, int time, bool isHit)
{
    bool valid = false;
    int diff;

    if (isHit)
    {
        diff = SDL_fabs(note->time - time);

        if (!PLD_IsChainBitType(note->type))
        {
            if (diff <= PLD_COOL_TIMING)
            {
                valid = true;
                PLD_PlayEffect(PLD_EFFECT_HIT, PLD_GAMEPLAY_IMAGE_HIT, note->targetX, note->targetY);

                evaluate->image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_COOL);

                evaluate->coolCount++;
                evaluate->combo++;
                evaluate->score += PLD_COOL_POINTS;
            }
            else if (diff <= PLD_FINE_TIMING)
            {
                valid = true;
                PLD_PlayEffect(PLD_EFFECT_HIT, PLD_GAMEPLAY_IMAGE_HIT, note->targetX, note->targetY);
                evaluate->image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_FINE);

                evaluate->fineCount++;
                evaluate->combo++;
                evaluate->score += PLD_FINE_POINTS;
            }
            else if (diff <= PLD_SAFE_TIMING)
            {
                evaluate->image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_SAFE);

                evaluate->safeCount++;
                evaluate->combo = 0;
                evaluate->score += PLD_SAFE_POINTS;
            }
            else
            {
                evaluate->image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_SAD);

                evaluate->sadCount++;
                evaluate->combo = 0;
                evaluate->score += PLD_SAD_POINTS;
            }
        }
        else
        {
            PLD_AddScore(PLD_CHAIN_BIT_POINTS);
            PLD_PlayEffect(PLD_EFFECT_HIT, PLD_GAMEPLAY_IMAGE_HIT, note->targetX, note->targetY);
        }

        if (PLD_IsSlideType(note->type))
        {
            PLD_PlaySound("slide", 1, 0);
        }
        else if (PLD_IsChainType(note->type))
        {
            PLD_PlaySound("slide_start", 2, -1);
        }
        else if (PLD_IsChainBitType(note->type))
        {
            if (note->nextChainBit == NULL)
            {
                PLD_PlaySound("slide_end", 2, 0);
                PLD_AddScore(PLD_CHAIN_MAX_BONUS);
            }
        }
        else
        {
            PLD_PlaySound("sound", 0, 0);
        }
    }
    else if (!PLD_IsChainBitType(note->type))
    {
        int diff = SDL_fabs(note->time - time);

        if (diff <= PLD_COOL_TIMING)
        {
            evaluate->score += PLD_WRONG_COOL_POINTS;
        }
        else if (diff <= PLD_FINE_TIMING)
        {
            evaluate->score += PLD_WRONG_FINE_POINTS;
        }
        else if (diff <= PLD_SAFE_TIMING)
        {
            evaluate->score += PLD_WRONG_SAFE_POINTS;
        }
        else if (diff <= PLD_SAD_TIMING)
        {
            evaluate->score += PLD_WRONG_SAD_POINTS;
        }

        evaluate->image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_WORST);

        evaluate->worstCount++;
        evaluate->combo = 0;
    }

    if (!PLD_IsChainBitType(note->type))
    {
        evaluate->rect.w = evaluate->image->width;
        evaluate->rect.h = evaluate->image->height;
        evaluate->rect.x = note->targetX;
        evaluate->rect.y = note->targetY - PLD_EVALUATE_POS;

        evaluate->timeOut = time + PLD_EVALUATE_TIMEOUT;
    }

    note->active = false;

    if (evaluate->combo > evaluate->maxCombo)
    {
        evaluate->maxCombo = evaluate->combo;
    }

    return valid;
}

int PLD_FreeEvaluate()
{
    if (evaluate == NULL)
    {
        return PLD_FAILURE;
    }

    for (int i = 0; i < PLD_DIGIT_MAX; i++)
    {
        SDL_free(evaluate->scoreRect[i]);
    }

    SDL_free(evaluate->scoreRect);
    SDL_free(evaluate);
    evaluate = NULL;

    return PLD_SUCCESS;
}


void PLD_GetEvaluates(int* cool, int* fine, int* safe, int* sad, int* worst, int* score, int* combo)
{
    *cool = evaluate->coolCount;
    *fine = evaluate->fineCount;
    *safe = evaluate->safeCount;
    *sad = evaluate->sadCount;
    *worst = evaluate->worstCount;
    *score = evaluate->score;
    *combo = evaluate->maxCombo;
}

void PLD_AddScore(int value)
{
    evaluate->score += value;
}