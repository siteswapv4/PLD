#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <PLD_image.h>
#include <PLD_context.h>

#define PLD_FONT_SIZE 60


typedef struct PLD_Text
{
    SDL_Texture* texture;
}PLD_Text;


bool PLD_OpenFont(PLD_Context* context);

PLD_Text* PLD_CreateText(PLD_Context* context, const char* text);

bool PLD_SetTextColor(PLD_Text* text, Uint8 r, Uint8 g, Uint8 b);

bool PLD_RenderText(PLD_Context* context, PLD_Text* text, int posx, int posy, int max_width, int alpha, bool center);

void PLD_DestroyText(PLD_Text* text);

void PLD_CloseFont();