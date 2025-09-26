#include <PLD/PLD_effect.h>

#define PLD_EFFECT_DIRECTORY "effects/"
#define PLD_EFFECT_EXTENSION ".etd"


const char* PLD_EFFECTS_STRINGS[PLD_EFFECT_COUNT] = {"appear", "hit"};


typedef struct PLD_Effect
{
    bool playing;
    Sint32 start;
    Sint32 end;
    
    Sint32 start_x;
    Sint32 start_y;
    Sint32 start_alpha;
    Sint32 start_rotation;
    float start_scale_x;
    float start_scale_y;
    
    Sint32 end_x;
    Sint32 end_y;
    Sint32 end_alpha;
    Sint32 end_rotation;
    float end_scale_x;
    float end_scale_y;

    PLD_Image* image;
    SDL_FRect rect;
}PLD_Effect;


PLD_Effect* PLD_effects[PLD_EFFECT_COUNT] = {0};

PLD_ArrayList* PLD_playing_effects = NULL;


PLD_Effect* PLD_LoadEffect(PLD_Context* context, const char* path)
{
    PLD_Effect* effect = SDL_calloc(1, sizeof(PLD_Effect));
    SDL_IOStream* etd_file = NULL;

    etd_file = SDL_IOFromFile(path, "rb");
    if (etd_file == NULL)
    {
        goto error;
    }

    PLD_READ_CHECK(etd_file, &effect->start_x, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->start_y, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->start_alpha, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->start_rotation, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->start_scale_x, sizeof(float));
    PLD_READ_CHECK(etd_file, &effect->start_scale_y, sizeof(float));
    PLD_READ_CHECK(etd_file, &effect->start, sizeof(Sint32));

    PLD_READ_CHECK(etd_file, &effect->end_x, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->end_y, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->end_alpha, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->end_rotation, sizeof(Sint32));
    PLD_READ_CHECK(etd_file, &effect->end_scale_x, sizeof(float));
    PLD_READ_CHECK(etd_file, &effect->end_scale_y, sizeof(float));
    PLD_READ_CHECK(etd_file, &effect->end, sizeof(Sint32));
    
    SDL_CloseIO(etd_file);

    return effect;

error:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());

    if (etd_file != NULL)
    {
        SDL_CloseIO(etd_file);
    }

    if (effect != NULL)
    {
        SDL_free(effect);
    }

    return NULL;
}


bool PLD_LoadEffects(PLD_Context* context)
{
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


bool PLD_PlayEffect(PLD_EffectName effect_name, PLD_ImageName image_name, int posx, int posy)
{
    if (PLD_effects[effect_name] == NULL)
    {
        return false;
    }

    if (PLD_playing_effects == NULL)
    {
        PLD_playing_effects = PLD_CreateArrayList();
    }

    PLD_Effect* effect = SDL_calloc(1, sizeof(PLD_Effect));
    *effect = *PLD_effects[effect_name];

    effect->image = PLD_GetImage(image_name);
    effect->rect.w = effect->image->width;
    effect->rect.h = effect->image->height;

    effect->start_x += posx;
    effect->end_x += posx;
    effect->start_y += posy;
    effect->end_y += posy;

    PLD_ArrayListAdd(PLD_playing_effects, effect);

    return true;
}


bool PLD_RenderEffect(PLD_Context* context, PLD_Effect* effect, int time)
{
    int rotation;
    int alpha;
    float coeff = 0;

    if (!effect->playing)
    {
        effect->playing = true;
        effect->start += time;
        effect->end += time;
    }
    else if (effect->end < time)
    {
        return false;
    }

    SDL_FRect rectangle = effect->rect;

    coeff = 1 - ((float)(effect->end - time)/(effect->end - effect->start));

    rectangle.x = effect->start_x + ((effect->end_x - effect->start_x) * coeff);
    rectangle.y = effect->start_y + ((effect->end_y - effect->start_y) * coeff);
    rectangle.w = effect->rect.w * (effect->start_scale_x + ((effect->end_scale_x - effect->start_scale_x) * coeff));
    rectangle.h = effect->rect.h * (effect->start_scale_y + ((effect->end_scale_y - effect->start_scale_y) * coeff));

    rotation = effect->start_rotation + ((effect->end_rotation - effect->start_rotation) * coeff);
    alpha = effect->start_alpha + ((effect->end_alpha - effect->start_alpha) * coeff);

    return PLD_RenderImage(context, effect->image, NULL, &rectangle, true, rotation, NULL, alpha);
}


bool PLD_RenderEffects(PLD_Context* context, int time)
{
    if (PLD_playing_effects != NULL)
    {
        for (int i = 0; i < PLD_playing_effects->len; i++)
        {
            if (!PLD_RenderEffect(context, PLD_playing_effects->data[i], time))
            {
                SDL_free(PLD_ArrayListRemoveAt(PLD_playing_effects, i));
            }
        }
    }

    return true;
}


bool PLD_ClearPlayingEffects()
{
    if (PLD_playing_effects != NULL)
    {
        PLD_DestroyArrayList(PLD_playing_effects, SDL_free);
        PLD_playing_effects = NULL;
    }

    return true;
}

void PLD_DestroyEffects()
{
    for (int i = 0; i < PLD_EFFECT_COUNT; i++)
    {
        if (PLD_effects[i] != NULL)
        {
            SDL_free(PLD_effects[i]);
            PLD_effects[i] = NULL;
        }
    }
}