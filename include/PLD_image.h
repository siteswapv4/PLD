#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <PLD_context.h>


#define PLD_IMAGE_ALPHA_FULL 255


typedef enum PLD_ImageCategory
{
    PLD_IMAGE_CATEGORY_INVALID = -1,
    PLD_IMAGE_CATEGORY_GAMEPLAY,
    PLD_IMAGE_CATEGORY_MENU,
    PLD_IMAGE_CATEGORY_COUNT
}PLD_ImageCategory;


// This order is important, only add new image at the end of each category
typedef enum PLD_ImageName
{
    PLD_GAMEPLAY_IMAGE_INVALID = -1,
    
    PLD_GAMEPLAY_IMAGE_0B,
    PLD_GAMEPLAY_IMAGE_0BM,
    PLD_GAMEPLAY_IMAGE_0T,
    PLD_GAMEPLAY_IMAGE_0TM,
    PLD_GAMEPLAY_IMAGE_1B,
    PLD_GAMEPLAY_IMAGE_1BM,
    PLD_GAMEPLAY_IMAGE_1T,
    PLD_GAMEPLAY_IMAGE_1TM,
    PLD_GAMEPLAY_IMAGE_2B,
    PLD_GAMEPLAY_IMAGE_2BM,
    PLD_GAMEPLAY_IMAGE_2T,
    PLD_GAMEPLAY_IMAGE_2TM,
    PLD_GAMEPLAY_IMAGE_3B,
    PLD_GAMEPLAY_IMAGE_3BM,
    PLD_GAMEPLAY_IMAGE_3T,
    PLD_GAMEPLAY_IMAGE_3TM,
    PLD_GAMEPLAY_IMAGE_12B,
    PLD_GAMEPLAY_IMAGE_12BM,
    PLD_GAMEPLAY_IMAGE_12T,
    PLD_GAMEPLAY_IMAGE_12TM,
    PLD_GAMEPLAY_IMAGE_13B,
    PLD_GAMEPLAY_IMAGE_13BM,
    PLD_GAMEPLAY_IMAGE_13T,
    PLD_GAMEPLAY_IMAGE_13TM,
    PLD_GAMEPLAY_IMAGE_15B,
    PLD_GAMEPLAY_IMAGE_15BM,
    PLD_GAMEPLAY_IMAGE_15T,
    PLD_GAMEPLAY_IMAGE_15TM,
    PLD_GAMEPLAY_IMAGE_16B,
    PLD_GAMEPLAY_IMAGE_16BM,
    PLD_GAMEPLAY_IMAGE_16T,
    PLD_GAMEPLAY_IMAGE_16TM,
    PLD_GAMEPLAY_IMAGE_H,
    PLD_GAMEPLAY_IMAGE_HM,
    PLD_GAMEPLAY_IMAGE_JIKU,
    PLD_GAMEPLAY_IMAGE_WORST,
    PLD_GAMEPLAY_IMAGE_SAD,
    PLD_GAMEPLAY_IMAGE_SAFE,
    PLD_GAMEPLAY_IMAGE_FINE,
    PLD_GAMEPLAY_IMAGE_COOL,
    PLD_GAMEPLAY_IMAGE_APPEAR,
    PLD_GAMEPLAY_IMAGE_HIT,
    PLD_GAMEPLAY_IMAGE_CONNECT,
    PLD_GAMEPLAY_IMAGE_NUM,
    PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP,
    PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM,
    PLD_GAMEPLAY_IMAGE_PAUSE_BACKGROUND,

    PLD_MENU_IMAGE_SONG_MENU_TITLE,
    PLD_MENU_IMAGE_SONG_MENU_BOTTOM,
    PLD_MENU_IMAGE_SONG_BAR,
    PLD_MENU_IMAGE_SONG_INFO,
    PLD_MENU_IMAGE_BACKGROUND,

    PLD_IMAGE_COUNT
}PLD_ImageName;


typedef struct PLD_Image
{
    int width;
    int height;

    // PRIVATE
    SDL_Texture* texture;
}PLD_Image;


bool PLD_LoadImages(PLD_Context* context, PLD_ImageCategory category);

void PLD_DestroyImages(PLD_ImageCategory category);

PLD_Image* PLD_GetImage(PLD_ImageName name);

PLD_Image* PLD_LoadImage(PLD_Context* context, const char* path);

PLD_Image* PLD_CreateImageFromTexture(PLD_Context* context, SDL_Texture* texture);

void PLD_DestroyImage(PLD_Image* image);


bool PLD_RenderImage(PLD_Context* context, PLD_Image* image, SDL_FRect* src_rect, SDL_FRect* dst_rect, bool center, double rotation, SDL_FPoint* rotatio_center, int alpha);

// Fits and scales image properly
bool PLD_RenderImageLetterbox(PLD_Context* context, PLD_Image* image);