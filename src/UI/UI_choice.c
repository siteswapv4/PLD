#include "UI_choice.h"
#include "UI_animation.h"
#include "UI_utils.h"

#define UI_CHOICE_PADDING 20.0f
#define UI_CHOICE_MARGIN 10.0f
#define UI_CHOICE_BOX_BORDER_SIZE 6

#define UI_CHOICE_OUTLINE_BORDER_SIZE 10

static const char* UI_CHOICE_BOX_SVG =
"<svg width='%d' height='%d'>"
"<rect x='%d' y='%d' width='%d' height='%d' rx='%d' fill='black' fill-opacity='0.9' stroke='white' stroke-width='%d'/>"
"</svg>";

static const char* UI_CHOICE_OUTLINE_SVG =
"<svg width='%d' height='%d'>"
"<rect x='%d' y='%d' width='%d' height='%d' rx='%d' fill='none' stroke='rgb(255, 222, 33)' stroke-width='%d'/>"
"</svg>";

#define UI_CHOICE_OPEN_ANIMATION_NUM_STATES 4
static const UI_AnimationState UI_CHOICE_OPEN_ANIMATION_STATES[UI_CHOICE_OPEN_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {1.1f, 1.1f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {300, {0.0f, 0.0f}, {0.9f, 0.9f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {400, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

#define UI_CHOICE_CLOSE_ANIMATION_NUM_STATES 2
static const UI_AnimationState UI_CHOICE_CLOSE_ANIMATION_STATES[UI_CHOICE_CLOSE_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

typedef struct UI_Choice
{
    SDL_FRect box_rect;
    SDL_Texture* background;
    SDL_Texture* outline;
    SDL_Texture** box_textures;
    TTF_Text** texts;
    UI_AnimationQueue* box_queue;
    UI_AnimationQueue* effect_queue;
    UI_Animation* open_animation;
    UI_Animation* close_animation;
    UI_Animation* switch_up_animation;
    UI_Animation* switch_down_animation;
    UI_Animation* switch_left_animation;
    UI_Animation* switch_right_animation;
    int num_choices;
    int selected;
    UI_ChoicePhase phase;
    UI_ChoiceDirection direction;

    TTF_Text*** options;
    int* num_options;
    int* selected_options;
}UI_Choice;

void UI_OpenChoiceCallback(void* userdata)
{
    UI_Choice* choice = userdata;
    choice->phase = UI_CHOICE_OPENED;   
}

void UI_CloseChoiceCallback(void* userdata)
{
    UI_Choice* choice = userdata;
    choice->phase = UI_CHOICE_CLOSED;
}

void UI_SwitchChoiceCallback(void* userdata)
{
    UI_Choice* choice = userdata;
    choice->phase = UI_CHOICE_OPENED;
}

SDL_Texture* UI_GetBoxSVG(SDL_Renderer* renderer, const char* source, int width, int height, int border)
{
    char* svg;
    int length = SDL_asprintf(&svg, source, width, height, 
                                    border / 2, border / 2,
                                    width - border, height - border,
                                    height / 2, border);

    if (length)
    {
        SDL_Texture* texture = UI_LoadSVG_IO(renderer, SDL_IOFromConstMem(svg, length), true, 0, 0);
        SDL_free(svg);
        return texture;
    }
    return NULL;
}

UI_Animation* UI_GetSwitchAnimation(int width, int height, int direction)
{
    float end_x = width + UI_CHOICE_MARGIN * 2.0f;
    float end_y = height + UI_CHOICE_MARGIN * 2.0f;

    if ((direction == 0) || (direction == 1))
    {
        const UI_AnimationState switch_states[2] = {
            {0,   {0.0f, 0.0f},                              {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
            {200, {0.0f, direction == 0 ? -end_y : end_y}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
        };

        return UI_CreateAnimationFromStates(switch_states, 2);
    }
    else
    {
        const UI_AnimationState switch_states[2] = {
            {0,   {0.0f, 0.0f},                              {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
            {200, {direction == 2 ? -end_x : end_x, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
        };

        return UI_CreateAnimationFromStates(switch_states, 2);
    }
}

UI_Choice* UI_CreateChoice(SDL_Renderer* renderer, SDL_FPoint position, int width, TTF_TextEngine* engine, TTF_Font* font, const char* const* choices, int num_choices, UI_ChoiceDirection direction)
{
    if (!renderer) { SDL_Log("Failed to create choice : renderer NULL"); return NULL; }
    if (!engine) { SDL_Log("Failed to create choice : engine NULL"); return NULL; }
    if (!font) { SDL_Log("Failed to create choice : font NULL"); return NULL; }
    if (!choices) { SDL_Log("Failed to create choice : choices NULL"); return NULL; }
    if (num_choices < 2) { SDL_Log("Failed to create choice : num_choices < 2"); return NULL; }

    UI_Choice* choice = SDL_calloc(1, sizeof(UI_Choice));
    if (!choice) { goto error; }

    choice->box_textures = SDL_calloc(num_choices, sizeof(SDL_Texture*));
    if (!choice->box_textures) { goto error; }
    choice->texts = SDL_calloc(num_choices, sizeof(TTF_Text*));
    if (!choice->texts) { goto error; }

    int max_height = 0;
    for (int i = 0; i < num_choices; i++)
    {
        int text_width, text_height;
        choice->texts[i] = TTF_CreateText(engine, font, choices[i], 0);
        if (!choice->texts[i]) { goto error; }
        TTF_SetTextColor(choice->texts[i], 255, 255, 255, 255);
        TTF_SetTextWrapWidth(choice->texts[i], width - UI_CHOICE_PADDING * 2.0f);
        TTF_GetTextSize(choice->texts[i], &text_width, &text_height);
        if (text_height > max_height) { max_height = text_height; }
    }

    if (direction == UI_CHOICE_VERTICAL)
    {
        choice->box_rect = (SDL_FRect){position.x, 0.0f, width, max_height + UI_CHOICE_PADDING * 2.0f};
        choice->box_rect.x -= choice->box_rect.w / 2.0f;
        choice->box_rect.y = position.y - (choice->box_rect.h * num_choices + UI_CHOICE_MARGIN * 2.0f * (num_choices - 1)) / 2.0f;
    }
    else
    {
        choice->box_rect = (SDL_FRect){0.0f, position.y, width, max_height + UI_CHOICE_PADDING * 2.0f};
        choice->box_rect.x = position.x - (choice->box_rect.w * num_choices + UI_CHOICE_MARGIN * 2.0f * (num_choices - 1)) / 2.0f;
        choice->box_rect.y -= choice->box_rect.h / 2.0f;
    }
    for (int i = 0; i < num_choices; i++)
    {
        choice->box_textures[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, choice->box_rect.w, choice->box_rect.h);
        if (!choice->box_textures[i]) { goto error; }
    }

    choice->box_queue = UI_CreateAnimationQueue();
    if (!choice->box_queue) { goto error; }
    choice->effect_queue = UI_CreateAnimationQueue();
    if (!choice->effect_queue) { goto error; }

    choice->open_animation = UI_CreateAnimationFromStates(UI_CHOICE_OPEN_ANIMATION_STATES, UI_CHOICE_OPEN_ANIMATION_NUM_STATES);
    if (!choice->open_animation) { goto error; }
    choice->close_animation = UI_CreateAnimationFromStates(UI_CHOICE_CLOSE_ANIMATION_STATES, UI_CHOICE_CLOSE_ANIMATION_NUM_STATES);
    if (!choice->close_animation) { goto error; }
    choice->switch_up_animation = UI_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 0);
    if (!choice->switch_up_animation) { goto error; }
    choice->switch_down_animation = UI_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 1);
    if (!choice->switch_down_animation) { goto error; }
    choice->switch_left_animation = UI_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 2);
    if (!choice->switch_left_animation) { goto error; }
    choice->switch_right_animation = UI_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 3);
    if (!choice->switch_right_animation) { goto error; }

    choice->background = UI_GetBoxSVG(renderer, UI_CHOICE_BOX_SVG, choice->box_rect.w, choice->box_rect.h, UI_CHOICE_BOX_BORDER_SIZE);
    if (!choice->background) { goto error; }
    choice->outline = UI_GetBoxSVG(renderer, UI_CHOICE_OUTLINE_SVG, choice->box_rect.w, choice->box_rect.h, UI_CHOICE_OUTLINE_BORDER_SIZE);
    if (!choice->outline) { goto error; }

    choice->phase = UI_CHOICE_CLOSED;
    choice->num_choices = num_choices;
    choice->direction = direction;

    return choice;

error:
    SDL_Log("Failed to create choice : %s", SDL_GetError());
    if (choice) { UI_DestroyChoice(choice); }
    return NULL;
}

void UI_DestroyChoice(UI_Choice* choice)
{
    if (choice)
    {
        for (int i = 0; i < choice->num_choices; i++)
        {
            if ((choice->box_textures) && (choice->box_textures[i])) { SDL_DestroyTexture(choice->box_textures[i]); }
            if ((choice->texts) && (choice->texts[i])) { TTF_DestroyText(choice->texts[i]); }
        }
        if (choice->box_queue) { UI_DestroyAnimationQueue(choice->box_queue); }
        if (choice->effect_queue) { UI_DestroyAnimationQueue(choice->effect_queue); }
        if (choice->open_animation) { UI_DestroyAnimation(choice->open_animation); }
        if (choice->close_animation) { UI_DestroyAnimation(choice->close_animation); }
        if (choice->switch_up_animation) { UI_DestroyAnimation(choice->switch_up_animation); }
        if (choice->switch_down_animation) { UI_DestroyAnimation(choice->switch_down_animation); }
        if (choice->switch_left_animation) { UI_DestroyAnimation(choice->switch_left_animation); }
        if (choice->switch_right_animation) { UI_DestroyAnimation(choice->switch_right_animation); }
        if (choice->background) { SDL_DestroyTexture(choice->background); }
        if (choice->outline) { SDL_DestroyTexture(choice->outline); }
        if (choice->options)
        {
            for (int i = 0; i < choice->num_choices; i++)
            {
                for (int j = 0; j < choice->num_options[i]; j++)
                {
                    if (choice->options[i][j]) { TTF_DestroyText(choice->options[i][j]); }
                }
                if (choice->options[i]) { SDL_free(choice->options[i]); }
            }
            SDL_free(choice->options);
        }
        if (choice->num_options) { SDL_free(choice->num_options); }
        if (choice->selected_options) { SDL_free(choice->selected_options); }
        SDL_free(choice);
    }
}

bool UI_SetChoiceOptions(UI_Choice* choice, TTF_TextEngine* engine, TTF_Font* font, const char* const* const* options, const int* selected_options, const int* num_options)
{
    if (!choice) { SDL_Log("Failed to set choice options : choice NULL"); return false; }
    if (!options) { SDL_Log("Failed to set choice options : options NULL"); return false; }
    if (!num_options) { SDL_Log("Failed to set choice options : num_options NULL"); return false; }

    choice->options = SDL_calloc(choice->num_choices, sizeof(TTF_Text**));
    if (!choice->options) { goto error; }
    for (int i = 0; i < choice->num_choices; i++)
    {
        choice->options[i] = SDL_calloc(num_options[i], sizeof(TTF_Text*));
        if (!choice->options[i]) { goto error; }
        for (int j = 0; j < num_options[i]; j++)
        {
            choice->options[i][j] = TTF_CreateText(engine, font, options[i][j], 0);
            if (!choice->options[i][j]) { goto error; }
            TTF_SetTextColor(choice->options[i][j], 255, 255, 255, 255);
        }
    }
    choice->num_options = SDL_calloc(choice->num_choices, sizeof(int));
    if (!choice->num_options) { goto error; }
    SDL_memcpy(choice->num_options, num_options, choice->num_choices * sizeof(int));

    choice->selected_options = SDL_calloc(choice->num_choices, sizeof(int));
    if (!choice->selected_options) { goto error; }
    SDL_memcpy(choice->selected_options, selected_options, choice->num_choices * sizeof(int));

    return true;

error:
    SDL_Log("Failed to set choice options : %s", SDL_GetError());
    if (choice) { UI_DestroyChoice(choice); }
    return false;
}

SDL_FPoint UI_GetChoicePosition(UI_Choice* choice, int i, bool center)
{
    SDL_FPoint position = {choice->box_rect.x, choice->box_rect.y};

    if (choice->direction == UI_CHOICE_VERTICAL)
    {
        position.y += i * (choice->box_rect.h + UI_CHOICE_MARGIN * 2.0f);
    }
    else
    {
        position.x += i * (choice->box_rect.w + UI_CHOICE_MARGIN * 2.0f);
    }

    if (center)
    {
        position.x += choice->box_rect.w / 2.0f;
        position.y += choice->box_rect.h / 2.0f;
    }

    return position;
}

bool UI_OpenChoice(UI_Choice* choice, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to open choice : choice NULL"); return false; }
    if (choice->phase != UI_CHOICE_CLOSED) { SDL_Log("Failed to open choice : choice not closed"); return false; }

    for (int i = 0; i < choice->num_choices; i++)
    {
        SDL_FPoint position = UI_GetChoicePosition(choice, i, true);
        UI_PlayAnimationWithCallback(choice->box_queue, choice->open_animation, choice->box_textures[i], position, false, time, UI_OpenChoiceCallback, choice);
    }

    choice->selected = 0;
    choice->phase = UI_CHOICE_OPENING;

    return true;
}

bool UI_CloseChoice(UI_Choice* choice, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to close choice : choice NULL"); return false; }
    if (choice->phase != UI_CHOICE_OPENED) { SDL_Log("Failed to close choice : choice not opened"); return false; }

    for (int i = 0; i < choice->num_choices; i++)
    {
        SDL_FPoint position = UI_GetChoicePosition(choice, i, true);
        UI_PlayAnimationWithCallback(choice->box_queue, choice->close_animation, choice->box_textures[i], position, false, time, UI_CloseChoiceCallback, choice);
    }

    choice->phase = UI_CHOICE_CLOSING;

    return true;
}

bool UI_RenderChoice(SDL_Renderer* renderer, UI_Choice* choice, Uint64 time)
{
    for (int i = 0; i < choice->num_choices; i++)
    {
        SDL_SetRenderTarget(renderer, choice->box_textures[i]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_FRect rect = {0.0f, 0.0f, choice->background->w, choice->background->h};
        SDL_RenderTexture(renderer, choice->background, NULL, &rect);
        int text_width, text_height;
        if (choice->options)
        {
            TTF_GetTextSize(choice->texts[i], &text_width, &text_height);
            TTF_DrawRendererText(choice->texts[i], (int)(UI_CHOICE_MARGIN * 5), (int)(choice->box_rect.h / 2.0f - text_height / 2.0f));

            TTF_GetTextSize(choice->options[i][choice->selected_options[i]], &text_width, &text_height);
            TTF_DrawRendererText(choice->options[i][choice->selected_options[i]], (int)(choice->box_rect.w - UI_CHOICE_MARGIN * 5 - text_width), (int)(choice->box_rect.h / 2.0f - text_height / 2.0f));
        }
        else
        {
            TTF_GetTextSize(choice->texts[i], &text_width, &text_height);
            TTF_DrawRendererText(choice->texts[i], (int)(choice->box_rect.w / 2.0f - text_width / 2.0f), (int)(choice->box_rect.h / 2.0f - text_height / 2.0f));
        }
    }
    SDL_SetRenderTarget(renderer, NULL);

    UI_RenderAnimationQueue(renderer, choice->box_queue, time);
    if (choice->phase == UI_CHOICE_OPENED || choice->phase == UI_CHOICE_SWITCHING)
    {
        for (int i = 0; i < choice->num_choices; i++)
        {
            SDL_FRect rect = choice->box_rect;
            SDL_FPoint position = UI_GetChoicePosition(choice, i, false);
            rect.x = position.x;
            rect.y = position.y;
            SDL_RenderTexture(renderer, choice->box_textures[i], NULL, &rect);
            if ((choice->phase != UI_CHOICE_SWITCHING) && (i == choice->selected))
            {
                SDL_RenderTexture(renderer, choice->outline, NULL, &rect);
            }
        }
    }
    UI_RenderAnimationQueue(renderer, choice->effect_queue, time);

    return true;
}

UI_ChoicePhase UI_GetChoicePhase(UI_Choice* choice)
{
    if (!choice) { return UI_CHOICE_INVALID; }
    return choice->phase;
}

int UI_GetSelectedChoice(UI_Choice* choice)
{
    if (!choice) { return -1; }
    return choice->selected;
}

bool UI_SetSelectedChoice(UI_Choice* choice, int index, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to switch choice : choice NULL"); return false; }
    if (choice->phase != UI_CHOICE_OPENED) { SDL_Log("Failed to switch choice : choice not opened"); return false; }

    int old_choice = choice->selected;
    choice->selected = index;
    choice->selected = SDL_clamp(choice->selected, 0, choice->num_choices - 1);

    SDL_FPoint position = {choice->box_rect.x, choice->box_rect.y};
    if (choice->direction == UI_CHOICE_VERTICAL)
    {
        position.y += old_choice * (choice->box_rect.h + UI_CHOICE_MARGIN * 2.0f);
    }
    else
    {
        position.x += old_choice * (choice->box_rect.w + UI_CHOICE_MARGIN * 2.0f);
    }
    position.x += choice->box_rect.w / 2.0f;
    position.y += choice->box_rect.h / 2.0f;

    if (old_choice - choice->selected > 0)
    {
        UI_PlayAnimationWithCallback(choice->effect_queue, choice->direction == UI_CHOICE_VERTICAL ? choice->switch_up_animation : choice->switch_left_animation, choice->outline, position, false, time, UI_SwitchChoiceCallback, choice);
        choice->phase = UI_CHOICE_SWITCHING;
    }
    else if (old_choice - choice->selected < 0)
    {
        UI_PlayAnimationWithCallback(choice->effect_queue, choice->direction == UI_CHOICE_VERTICAL ? choice->switch_down_animation : choice->switch_right_animation, choice->outline, position, false, time, UI_SwitchChoiceCallback, choice);
        choice->phase = UI_CHOICE_SWITCHING;
    }

    return true;
}

int UI_GetSelectedOption(UI_Choice* choice)
{
    if (!choice) { return -1; }
    return choice->selected_options[choice->selected];
}

bool UI_SetSelectedOption(UI_Choice* choice, int index, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to switch choice : choice NULL"); return false; }
    if (choice->phase != UI_CHOICE_OPENED) { SDL_Log("Failed to switch choice : choice not opened"); return false; }

    int old_option = choice->selected_options[choice->selected];
    choice->selected_options[choice->selected] = index;
    choice->selected_options[choice->selected] = SDL_clamp(choice->selected_options[choice->selected], 0, choice->num_options[choice->selected] - 1);

    return true;
}