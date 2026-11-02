#include <PLD/PLD_popup.h>
#include <PLD/PLD_animation.h>
#include <PLD/PLD_text.h>
#include <PLD/PLD_utils.h>

#define PLD_POPUP_MARGIN 20.0f
#define PLD_POPUP_BORDER_SIZE 10

#define PLD_POPUP_APPEAR_ANIMATION_NUM_STATES 4
static const PLD_AnimationState PLD_POPUP_APPEAR_ANIMATION_STATES[PLD_POPUP_APPEAR_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {1.1f, 1.1f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {300, {0.0f, 0.0f}, {0.9f, 0.9f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {400, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

#define PLD_POPUP_CLOSE_ANIMATION_NUM_STATES 2
static const PLD_AnimationState PLD_POPUP_CLOSE_ANIMATION_STATES[PLD_POPUP_CLOSE_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

#define PLD_POPUP_CONTINUE_ANIMATION_NUM_STATES 2
static const PLD_AnimationState PLD_POPUP_CONTINUE_ANIMATION_STATES[PLD_POPUP_CONTINUE_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f,   {1.0f, 1.0f, 1.0f, 1.0f}},
    {2000, {0.0f, 0.0f}, {1.0f, 1.0f}, 360.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

static const char* PLD_POPUP_BACKGROUND_SVG =
"<svg width='%d' height='%d'>"
"<rect x='%d' y='%d' width='%d' height='%d' rx='10' ry='10' fill='black' fill-opacity='0.9' stroke-width='%d' stroke='rgb(255, 222, 33)'/>"
"</svg>";

static const char* PLD_POPUP_CONTINUE_SVG = 
"<svg width='40' height='40'>"
"<rect x='5' y='5' width='30' height='30' fill='none' stroke-width='10' stroke='white'/>"
"</svg>";

typedef struct PLD_Popup
{
    SDL_FRect rect;
    PLD_Image* image;
    PLD_Image* background;
    PLD_Image* continue_image;
    PLD_Animation* appear_animation;
    PLD_Animation* close_animation;
    PLD_Animation* continue_animation;
    PLD_AnimationQueue* window_queue;
    PLD_AnimationQueue* effect_queue;
    SDL_FPoint middle;
    PLD_Text* text;

    PLD_PopupPhase phase;
}PLD_Popup;

void PLD_PopupAppearCallback(void* userdata)
{
    PLD_Popup* popup = userdata;
    popup->phase = PLD_POPUP_OPENED;
}

void PLD_PopupCloseCallback(void* userdata)
{
    PLD_Popup* popup = userdata;
    PLD_ClearAnimationQueue(popup->effect_queue);
    popup->phase = PLD_POPUP_CLOSED;
}

PLD_Image* PLD_GetBackgroundSVG(PLD_Context* context, int width, int height)
{
    char* svg;
    int length = SDL_asprintf(&svg, PLD_POPUP_BACKGROUND_SVG, width, height, 
                                                             PLD_POPUP_BORDER_SIZE / 2, PLD_POPUP_BORDER_SIZE / 2,
                                                             width - PLD_POPUP_BORDER_SIZE, height - PLD_POPUP_BORDER_SIZE,
                                                             PLD_POPUP_BORDER_SIZE);

    if (length)
    {
        PLD_Image* image = PLD_LoadSVG_IO(context, SDL_IOFromConstMem(svg, length), true, width, height);
        SDL_free(svg);
        return image;
    }
    return NULL;
}

PLD_Popup* PLD_CreatePopup(PLD_Context* context, SDL_FPoint position, int width, const char* text)
{
    if (!text) { SDL_Log("Failed to set popup text : text NULL"); return false; }
    PLD_Popup* popup = SDL_calloc(1, sizeof(PLD_Popup));
    if (!popup) { goto error; }

    int text_width, text_height;
    popup->text = PLD_CreateText(context, text, width - PLD_POPUP_MARGIN * 2);
    if (!popup->text) { goto error; }

    PLD_SetTextColor(popup->text, (SDL_Color){255, 255, 255, 255});
    PLD_GetTextSize(context, popup->text, &text_width, &text_height);
    if (text_width % 2 != 0) { text_width--; }
    if (text_height % 2 != 0) { text_height--; }
    popup->rect = (SDL_FRect){position.x, position.y, width, text_height + PLD_POPUP_MARGIN * 2};
    popup->rect.x -= popup->rect.w / 2.0f;
    popup->rect.y -= popup->rect.h / 2.0f;

    popup->image = PLD_CreateImage(context, popup->rect.w, popup->rect.h);
    if (!popup->image) { goto error; }

    popup->background = PLD_GetBackgroundSVG(context, popup->rect.w, popup->rect.h);
    if (!popup->background) { goto error; }

    popup->continue_image = PLD_LoadSVG_IO(context, SDL_IOFromConstMem(PLD_POPUP_CONTINUE_SVG, SDL_strlen(PLD_POPUP_CONTINUE_SVG)), true, 40, 40);
    if (!popup->continue_image) { goto error; }

    popup->window_queue = PLD_CreateAnimationQueue();
    if (!popup->window_queue) { goto error; }
    popup->effect_queue = PLD_CreateAnimationQueue();
    if (!popup->effect_queue) { goto error; }

    popup->appear_animation = PLD_CreateAnimationFromStates(PLD_POPUP_APPEAR_ANIMATION_STATES, PLD_POPUP_APPEAR_ANIMATION_NUM_STATES);
    if (!popup->appear_animation) { goto error; }
    popup->close_animation = PLD_CreateAnimationFromStates(PLD_POPUP_CLOSE_ANIMATION_STATES, PLD_POPUP_CLOSE_ANIMATION_NUM_STATES);
    if (!popup->close_animation) { goto error; }
    popup->continue_animation = PLD_CreateAnimationFromStates(PLD_POPUP_CONTINUE_ANIMATION_STATES, PLD_POPUP_CONTINUE_ANIMATION_NUM_STATES);
    if (!popup->continue_animation) { goto error; }

    popup->middle = (SDL_FPoint){popup->rect.x + popup->rect.w / 2.0f, popup->rect.y + popup->rect.h / 2.0f};
    popup->phase = PLD_POPUP_CLOSED;

    return popup;

error:
    SDL_Log("Failed to create popup : %s", SDL_GetError());
    if (popup) { PLD_DestroyPopup(popup); }
    return NULL;
}

void PLD_DestroyPopup(PLD_Popup* popup)
{
    if (popup)
    {
        if (popup->image) { PLD_DestroyImage(popup->image); }
        if (popup->background) { PLD_DestroyImage(popup->background); }
        if (popup->continue_image) { PLD_DestroyImage(popup->continue_image); }
        if (popup->text) { PLD_DestroyText(popup->text); } 
        if (popup->window_queue) { PLD_DestroyAnimationQueue(popup->window_queue); }
        if (popup->effect_queue) { PLD_DestroyAnimationQueue(popup->effect_queue); }
        if (popup->appear_animation) { PLD_DestroyAnimation(popup->appear_animation); }
        if (popup->close_animation) { PLD_DestroyAnimation(popup->close_animation); }
        if (popup->continue_animation) { PLD_DestroyAnimation(popup->continue_animation); }
        SDL_free(popup);
    }
}

bool PLD_OpenPopup(PLD_Popup* popup, Uint64 time)
{
    if (!popup) { SDL_Log("Failed to open popup : popup NULL"); return false; }
    if (popup->phase != PLD_POPUP_CLOSED) { SDL_Log("Failed to open popup : popup already opened"); return false; }

    popup->phase = PLD_POPUP_OPENING;
    SDL_FPoint position = {popup->rect.w - popup->continue_image->width * 1.2f, popup->rect.h - popup->continue_image->height * 1.2f};
    PLD_PlayAnimation(popup->effect_queue, popup->continue_animation, popup->continue_image, position, true, time);
    PLD_PlayAnimationWithCallback(popup->window_queue, popup->appear_animation, popup->image, popup->middle, false, time, PLD_PopupAppearCallback, popup);

    return true;
}

bool PLD_RenderPopup(PLD_Context* context, PLD_Popup* popup, Uint64 time)
{
    if (!popup) { SDL_Log("Failed to render popup : popup NULL"); return false; }

    if (popup->phase == PLD_POPUP_CLOSED) { return false; }

    PLD_SetRenderTarget(context, popup->image);
    SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 0);
    SDL_RenderClear(context->renderer);
    PLD_RenderImage(context, popup->background, NULL, NULL, false, 0, NULL, 255);
    int text_width, text_height;
    PLD_GetTextSize(context, popup->text, &text_width, &text_height);
    int draw_x = (int)(popup->rect.w / 2.0f - text_width / 2.0f);
    int draw_y = (int)(popup->rect.h / 2.0f);
    PLD_RenderText(context, popup->text, (SDL_Point){draw_x, draw_y}, 0, 255, false, 0);
    PLD_RenderAnimationQueue(context, popup->effect_queue, time);
    PLD_SetRenderTarget(context, NULL);
    PLD_RenderAnimationQueue(context, popup->window_queue, time);

    if (popup->phase == PLD_POPUP_OPENED)
    {
        PLD_RenderImage(context, popup->image, NULL, &popup->rect, false, 0, NULL, 255);
    }

    return true;
}

bool PLD_ClosePopup(PLD_Popup* popup, Uint64 time)
{
    if (!popup) { SDL_Log("Failed to close popup : popup NULL"); return false; }
    if (popup->phase != PLD_POPUP_OPENED) { SDL_Log("Failed to close popup : popup not opened"); return false; }

    popup->phase = PLD_POPUP_CLOSING;
    PLD_PlayAnimationWithCallback(popup->window_queue, popup->close_animation, popup->image, popup->middle, false, time, PLD_PopupCloseCallback, popup);
    return true;
}

PLD_PopupPhase PLD_GetPopupPhase(PLD_Popup* popup)
{
    if (!popup) { SDL_Log("Failed to get popup phase : popup NULL"); return PLD_POPUP_INVALID; }
    return popup->phase;
}