#pragma once
#include <PLD/PLD_context.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_utils.h>


typedef enum PLD_EffectName
{
    PLD_EFFECT_INVALID = -1,
    PLD_EFFECT_APPEAR,
    PLD_EFFECT_HIT,
    PLD_EFFECT_COUNT
}PLD_EffectName;


bool PLD_LoadEffects(PLD_Context* context);

bool PLD_PlayEffect(PLD_EffectName effect_name, PLD_ImageName image_name, int posx, int posy);

bool PLD_RenderEffects(PLD_Context* context, int time);

bool PLD_ClearPlayingEffects();

void PLD_DestroyEffects();