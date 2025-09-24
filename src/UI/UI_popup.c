#include "UI_popup.h"
#include "UI_animation.h"
#include "UI_utils.h"

#define UI_POPUP_MARGIN 20.0f
#define UI_POPUP_BORDER_SIZE 10

#define UI_POPUP_APPEAR_ANIMATION_NUM_STATES 4
static const UI_AnimationState UI_POPUP_APPEAR_ANIMATION_STATES[UI_POPUP_APPEAR_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {1.1f, 1.1f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {300, {0.0f, 0.0f}, {0.9f, 0.9f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {400, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

#define UI_POPUP_CLOSE_ANIMATION_NUM_STATES 2
static const UI_AnimationState UI_POPUP_CLOSE_ANIMATION_STATES[UI_POPUP_CLOSE_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

#define UI_POPUP_CONTINUE_ANIMATION_NUM_STATES 2
static const UI_AnimationState UI_POPUP_CONTINUE_ANIMATION_STATES[UI_POPUP_CONTINUE_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f,   {1.0f, 1.0f, 1.0f, 1.0f}},
    {2000, {0.0f, 0.0f}, {1.0f, 1.0f}, 360.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

static const char* UI_POPUP_BACKGROUND_SVG =
"<svg width='%d' height='%d'>"
"<rect x='%d' y='%d' width='%d' height='%d' rx='10' ry='10' fill='black' fill-opacity='0.5' stroke-width='%d' stroke='rgb(255, 222, 33)'/>"
"</svg>";

static const char* UI_POPUP_CONTINUE_SVG = 
"<svg width='40' height='40'>"
"<rect x='5' y='5' width='30' height='30' fill='none' stroke-width='10' stroke='white'/>"
"</svg>";

typedef struct UI_Popup
{
    SDL_FRect rect;
    SDL_Texture* texture;
    SDL_Texture* background;
    SDL_Texture* continue_texture;
    UI_Animation* appear_animation;
    UI_Animation* close_animation;
    UI_Animation* continue_animation;
    UI_AnimationQueue* window_queue;
    UI_AnimationQueue* effect_queue;
    SDL_FPoint middle;
    TTF_Text* text;

    UI_PopupPhase phase;
}UI_Popup;

void UI_PopupAppearCallback(void* userdata)
{
    UI_Popup* popup = userdata;
    popup->phase = UI_POPUP_OPENED;
}

void UI_PopupCloseCallback(void* userdata)
{
    UI_Popup* popup = userdata;
    UI_ClearAnimationQueue(popup->effect_queue);
    popup->phase = UI_POPUP_CLOSED;
}

SDL_Texture* UI_GetBackgroundSVG(SDL_Renderer* renderer, int width, int height)
{
    char* svg;
    int length = SDL_asprintf(&svg, UI_POPUP_BACKGROUND_SVG, width, height, 
                                                             UI_POPUP_BORDER_SIZE / 2, UI_POPUP_BORDER_SIZE / 2,
                                                             width - UI_POPUP_BORDER_SIZE, height - UI_POPUP_BORDER_SIZE,
                                                             UI_POPUP_BORDER_SIZE);

    if (length)
    {
        SDL_Texture* texture = UI_LoadSVG_IO(renderer, SDL_IOFromConstMem(svg, length), true, 0, 0);
        SDL_free(svg);
        return texture;
    }
    return NULL;
}

UI_Popup* UI_CreatePopup(SDL_Renderer* renderer, SDL_FPoint position, int width, TTF_TextEngine* engine, TTF_Font* font, const char* text)
{
    if (!renderer) { SDL_Log("Failed to create popup : renderer NULL"); return NULL; }
    if (!engine) { SDL_Log("Failed to set popup text : engine NULL"); return false; }
    if (!font) { SDL_Log("Failed to set popup text : font NULL"); return false; }
    if (!text) { SDL_Log("Failed to set popup text : text NULL"); return false; }
    UI_Popup* popup = SDL_calloc(1, sizeof(UI_Popup));
    if (!popup) { goto error; }

    int text_width, text_height;
    popup->text = TTF_CreateText(engine, font, text, 0);
    if (!popup->text) { goto error; }

    TTF_SetTextWrapWidth(popup->text, width - UI_POPUP_MARGIN * 2);
    TTF_GetTextSize(popup->text, &text_width, &text_height);
    if (text_width % 2 != 0) { text_width--; }
    if (text_height % 2 != 0) { text_height--; }
    popup->rect = (SDL_FRect){position.x, position.y, width, text_height + UI_POPUP_MARGIN * 2};
    popup->rect.x -= popup->rect.w / 2.0f;
    popup->rect.y -= popup->rect.h / 2.0f;

    popup->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, popup->rect.w, popup->rect.h);
    if (!popup->texture) { goto error; }

    popup->background = UI_GetBackgroundSVG(renderer, popup->rect.w, popup->rect.h);
    if (!popup->background) { goto error; }

    popup->continue_texture = UI_LoadSVG_IO(renderer, SDL_IOFromConstMem(UI_POPUP_CONTINUE_SVG, SDL_strlen(UI_POPUP_CONTINUE_SVG)), true, 0, 0);
    if (!popup->continue_texture) { goto error; }

    popup->window_queue = UI_CreateAnimationQueue();
    if (!popup->window_queue) { goto error; }
    popup->effect_queue = UI_CreateAnimationQueue();
    if (!popup->effect_queue) { goto error; }

    popup->appear_animation = UI_CreateAnimationFromStates(UI_POPUP_APPEAR_ANIMATION_STATES, UI_POPUP_APPEAR_ANIMATION_NUM_STATES);
    if (!popup->appear_animation) { goto error; }
    popup->close_animation = UI_CreateAnimationFromStates(UI_POPUP_CLOSE_ANIMATION_STATES, UI_POPUP_CLOSE_ANIMATION_NUM_STATES);
    if (!popup->close_animation) { goto error; }
    popup->continue_animation = UI_CreateAnimationFromStates(UI_POPUP_CONTINUE_ANIMATION_STATES, UI_POPUP_CONTINUE_ANIMATION_NUM_STATES);
    if (!popup->continue_animation) { goto error; }

    popup->middle = (SDL_FPoint){popup->rect.x + popup->rect.w / 2.0f, popup->rect.y + popup->rect.h / 2.0f};
    popup->phase = UI_POPUP_CLOSED;

    return popup;

error:
    SDL_Log("Failed to create popup : %s", SDL_GetError());
    if (popup) { UI_DestroyPopup(popup); }
    return NULL;
}

void UI_DestroyPopup(UI_Popup* popup)
{
    if (popup)
    {
        if (popup->texture) { SDL_DestroyTexture(popup->texture); }
        if (popup->background) { SDL_DestroyTexture(popup->background); }
        if (popup->continue_texture) { SDL_DestroyTexture(popup->continue_texture); }
        if (popup->text) { TTF_DestroyText(popup->text); } 
        if (popup->window_queue) { UI_DestroyAnimationQueue(popup->window_queue); }
        if (popup->effect_queue) { UI_DestroyAnimationQueue(popup->effect_queue); }
        if (popup->appear_animation) { UI_DestroyAnimation(popup->appear_animation); }
        if (popup->close_animation) { UI_DestroyAnimation(popup->close_animation); }
        if (popup->continue_animation) { UI_DestroyAnimation(popup->continue_animation); }
        SDL_free(popup);
    }
}

bool UI_OpenPopup(UI_Popup* popup, Uint64 time)
{
    if (!popup) { SDL_Log("Failed to open popup : popup NULL"); return false; }
    if (popup->phase != UI_POPUP_CLOSED) { SDL_Log("Failed to open popup : popup already opened"); return false; }

    popup->phase = UI_POPUP_OPENING;
    SDL_FPoint position = {popup->rect.w - popup->continue_texture->w * 1.2f, popup->rect.h - popup->continue_texture->h * 1.2f};
    UI_PlayAnimation(popup->effect_queue, popup->continue_animation, popup->continue_texture, position, true, time);
    UI_PlayAnimationWithCallback(popup->window_queue, popup->appear_animation, popup->texture, popup->middle, false, time, UI_PopupAppearCallback, popup);

    return true;
}

bool UI_RenderPopup(SDL_Renderer* renderer, UI_Popup* popup, Uint64 time)
{
    if (!renderer) { SDL_Log("Failed to render popup : renderer NULL"); return false; }
    if (!popup) { SDL_Log("Failed to render popup : popup NULL"); return false; }

    if (popup->phase == UI_POPUP_CLOSED) { return false; }

    SDL_SetRenderTarget(renderer, popup->texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, popup->background, NULL, NULL);
    int text_width, text_height;
    TTF_GetTextSize(popup->text, &text_width, &text_height);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int draw_x = (int)(popup->rect.w / 2.0f - text_width / 2.0f);
    int draw_y = (int)(popup->rect.h / 2.0f - text_height / 2.0f);
    TTF_DrawRendererText(popup->text, draw_x, draw_y);
    UI_RenderAnimationQueue(renderer, popup->effect_queue, time);
    SDL_SetRenderTarget(renderer, NULL);
    UI_RenderAnimationQueue(renderer, popup->window_queue, time);

    if (popup->phase == UI_POPUP_OPENED)
    {
        SDL_RenderTexture(renderer, popup->texture, NULL, &popup->rect);
    }

    return true;
}

bool UI_ClosePopup(UI_Popup* popup, Uint64 time)
{
    if (!popup) { SDL_Log("Failed to close popup : popup NULL"); return false; }
    if (popup->phase != UI_POPUP_OPENED) { SDL_Log("Failed to close popup : popup not opened"); return false; }

    popup->phase = UI_POPUP_CLOSING;
    UI_PlayAnimationWithCallback(popup->window_queue, popup->close_animation, popup->texture, popup->middle, false, time, UI_PopupCloseCallback, popup);
    return true;
}

UI_PopupPhase UI_GetPopupPhase(UI_Popup* popup)
{
    if (!popup) { SDL_Log("Failed to get popup phase : popup NULL"); return UI_POPUP_INVALID; }
    return popup->phase;
}