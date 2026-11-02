#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <PLD/PLD_context.h>

typedef enum PLD_PopupPhase
{
    PLD_POPUP_INVALID = -1,
    PLD_POPUP_OPENING,
    PLD_POPUP_OPENED,
    PLD_POPUP_CLOSING,
    PLD_POPUP_CLOSED
}PLD_PopupPhase;

typedef struct PLD_Popup PLD_Popup;

PLD_Popup* PLD_CreatePopup(PLD_Context* context, SDL_FPoint position, int width, const char* text);

void PLD_DestroyPopup(PLD_Popup* popup);

bool PLD_OpenPopup(PLD_Popup* popup, Uint64 time);

bool PLD_RenderPopup(PLD_Context* context, PLD_Popup* popup, Uint64 time);

bool PLD_ClosePopup(PLD_Popup* popup, Uint64 time);

PLD_PopupPhase PLD_GetPopupPhase(PLD_Popup* popup);