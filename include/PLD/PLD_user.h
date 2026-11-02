#pragma once
#include <SDL3/SDL.h>

#include <PLD/PLD_context.h>

typedef struct PLD_UserData
{
    Uint32 cool_count;
    Uint32 fine_count;
    Uint32 safe_count;
    Uint32 sad_count;
    Uint32 worst_count;
    Uint32 combo;
    Uint32 score;
    Uint32 percentage;
    Uint32 num_tries;
    Uint32 grade;
    Uint32 accuracy;
    Uint32 offset;
}PLD_UserData;

bool PLD_InitUserData(PLD_Context* context);

void PLD_QuitUserData();

bool PLD_GetUserData(const char* chart_path, PLD_UserData* out_data);

bool PLD_SaveUserData(const char* chart_path, const PLD_UserData* in_data);