#include <PLD/PLD_effect.h>
#include <PLD/PLD_animation.h>

#define PLD_EFFECT_DIRECTORY "effects/"
#define PLD_EFFECT_EXTENSION ".etd"

const char* PLD_EFFECTS_STRINGS[PLD_EFFECT_COUNT] = {"appear", "hit", "max_slide_text", "max_slide_glow"};

PLD_Animation* PLD_effects[PLD_EFFECT_COUNT] = {0};
PLD_AnimationQueue* PLD_effect_queue = NULL;

PLD_Animation* PLD_LoadEffect(PLD_Context* context, const char* path)
{
    return PLD_LoadAnimation(path);
}

bool PLD_LoadEffects(PLD_Context* context)
{
    PLD_effect_queue = PLD_CreateAnimationQueue();
    for (int i = 0; i < PLD_EFFECT_COUNT; i++)
    {
        char* path;
        SDL_asprintf(&path, "%s%s%s%s", context->data_path, PLD_EFFECT_DIRECTORY, PLD_EFFECTS_STRINGS[i], PLD_EFFECT_EXTENSION);
        PLD_effects[i] = PLD_LoadEffect(context, path);
        SDL_free(path);

        if (PLD_effects[i] == NULL)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            PLD_DestroyEffects();
            return false;
        }
    }

    return true;
}

bool PLD_PlayEffect(PLD_EffectName effect_name, PLD_ImageName image_name, int posx, int posy, Uint64 time)
{
    PLD_Animation* effect = PLD_effects[effect_name];
    if (!effect) { return false; }
    PLD_Image* image = PLD_GetImage(image_name);
    if (!image) { return false; }

    return PLD_PlayAnimation(PLD_effect_queue, effect, image, (SDL_FPoint){posx, posy}, false, time);
}

bool PLD_RenderEffects(PLD_Context* context, Uint64 time)
{
    return PLD_RenderAnimationQueue(context, PLD_effect_queue, time);    
}

void PLD_ClearPlayingEffects()
{
    PLD_ClearAnimationQueue(PLD_effect_queue);
}

void PLD_DestroyEffects()
{
    for (int i = 0; i < PLD_EFFECT_COUNT; i++)
    {
        if (PLD_effects[i] != NULL)
        {
            PLD_DestroyAnimation(PLD_effects[i]);
            PLD_effects[i] = NULL;
        }
    }
    if (PLD_effect_queue)
    {
        PLD_DestroyAnimationQueue(PLD_effect_queue);
        PLD_effect_queue = NULL;
    }
}