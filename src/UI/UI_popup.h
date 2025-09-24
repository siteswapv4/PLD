#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef enum UI_PopupPhase
{
    UI_POPUP_INVALID = -1,
    UI_POPUP_OPENING,
    UI_POPUP_OPENED,
    UI_POPUP_CLOSING,
    UI_POPUP_CLOSED
}UI_PopupPhase;

typedef struct UI_Popup UI_Popup;

UI_Popup* UI_CreatePopup(SDL_Renderer* renderer, SDL_FPoint position, int width, TTF_TextEngine* engine, TTF_Font* font, const char* text);

void UI_DestroyPopup(UI_Popup* popup);

bool UI_OpenPopup(UI_Popup* popup, Uint64 time);

bool UI_RenderPopup(SDL_Renderer* renderer, UI_Popup* popup, Uint64 time);

bool UI_ClosePopup(UI_Popup* popup, Uint64 time);

UI_PopupPhase UI_GetPopupPhase(UI_Popup* popup);