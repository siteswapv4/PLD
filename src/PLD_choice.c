#include <PLD/PLD_choice.h>
#include <PLD/PLD_animation.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_text.h>

#define PLD_CHOICE_PADDING 20.0f
#define PLD_CHOICE_MARGIN 10.0f
#define PLD_CHOICE_BOX_BORDER_SIZE 6

#define PLD_CHOICE_OUTLINE_BORDER_SIZE 10

static const char* PLD_CHOICE_BOX_SVG =
"<svg width='%d' height='%d'>"
"<rect x='%d' y='%d' width='%d' height='%d' rx='%d' fill='black' fill-opacity='0.9' stroke='white' stroke-width='%d'/>"
"</svg>";

static const char* PLD_CHOICE_OUTLINE_SVG =
"<svg width='%d' height='%d'>"
"<rect x='%d' y='%d' width='%d' height='%d' rx='%d' fill='none' stroke='rgb(255, 222, 33)' stroke-width='%d'/>"
"</svg>";

static const char* PLD_CHOICE_LEFT_ARROW_SVG = 
"<svg width='100' height='100'>"
"<path d='M 90 0 L 0 50 L 90 90' fill='none' stroke='white' stroke-width='10'/>"
"</svg>";

#define PLD_CHOICE_OPEN_ANIMATION_NUM_STATES 4
static const PLD_AnimationState PLD_CHOICE_OPEN_ANIMATION_STATES[PLD_CHOICE_OPEN_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {1.1f, 1.1f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {300, {0.0f, 0.0f}, {0.9f, 0.9f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {400, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

#define PLD_CHOICE_CLOSE_ANIMATION_NUM_STATES 2
static const PLD_AnimationState PLD_CHOICE_CLOSE_ANIMATION_STATES[PLD_CHOICE_CLOSE_ANIMATION_NUM_STATES] = {
    {0,   {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    {200, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
};

typedef struct PLD_Choice
{
    SDL_FRect box_rect;
    PLD_Image* left_arrow;
    PLD_Image* background;
    PLD_Image* outline;
    PLD_Image** box_images;
    PLD_Text** texts;
    PLD_AnimationQueue* box_queue;
    PLD_AnimationQueue* effect_queue;
    PLD_Animation* open_animation;
    PLD_Animation* close_animation;
    PLD_Animation* switch_up_animation;
    PLD_Animation* switch_down_animation;
    PLD_Animation* switch_left_animation;
    PLD_Animation* switch_right_animation;
    int num_choices;
    int selected;
    PLD_ChoicePhase phase;
    PLD_ChoiceDirection direction;

    PLD_Text*** options;
    int* num_options;
    int* selected_options;
}PLD_Choice;

void PLD_OpenChoiceCallback(void* userdata)
{
    PLD_Choice* choice = userdata;
    choice->phase = PLD_CHOICE_OPENED;   
}

void PLD_CloseChoiceCallback(void* userdata)
{
    PLD_Choice* choice = userdata;
    choice->phase = PLD_CHOICE_CLOSED;
}

void PLD_SwitchChoiceCallback(void* userdata)
{
    PLD_Choice* choice = userdata;
    choice->phase = PLD_CHOICE_OPENED;
}

PLD_Image* PLD_GetBoxSVG(PLD_Context* context, const char* source, int width, int height, int border)
{
    char* svg;
    int length = SDL_asprintf(&svg, source, width, height, 
                                    border / 2, border / 2,
                                    width - border, height - border,
                                    height / 2, border);

    if (length)
    {
        PLD_Image* image = PLD_LoadSVG_IO(context, SDL_IOFromConstMem(svg, length), true, width, height);
        SDL_free(svg);
        return image;
    }
    return NULL;
}

PLD_Animation* PLD_GetSwitchAnimation(int width, int height, int direction)
{
    float end_x = width + PLD_CHOICE_MARGIN * 2.0f;
    float end_y = height + PLD_CHOICE_MARGIN * 2.0f;

    if ((direction == 0) || (direction == 1))
    {
        const PLD_AnimationState switch_states[2] = {
            {0,   {0.0f, 0.0f},                              {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
            {200, {0.0f, direction == 0 ? -end_y : end_y}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
        };

        return PLD_CreateAnimationFromStates(switch_states, 2);
    }
    else
    {
        const PLD_AnimationState switch_states[2] = {
            {0,   {0.0f, 0.0f},                              {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
            {200, {direction == 2 ? -end_x : end_x, 0.0f}, {1.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}
        };

        return PLD_CreateAnimationFromStates(switch_states, 2);
    }
}

PLD_Choice* PLD_CreateChoice(PLD_Context* context, SDL_FPoint position, int width, const char* const* choices, int num_choices, PLD_ChoiceDirection direction)
{
    if (!choices) { SDL_Log("Failed to create choice : choices NULL"); return NULL; }
    if (num_choices < 2) { SDL_Log("Failed to create choice : num_choices < 2"); return NULL; }

    PLD_Choice* choice = SDL_calloc(1, sizeof(PLD_Choice));
    if (!choice) { goto error; }

    choice->box_images = SDL_calloc(num_choices, sizeof(PLD_Image*));
    if (!choice->box_images) { goto error; }
    choice->texts = SDL_calloc(num_choices, sizeof(PLD_Text*));
    if (!choice->texts) { goto error; }

    int max_height = 0;
    for (int i = 0; i < num_choices; i++)
    {
        int text_width, text_height;
        choice->texts[i] = PLD_CreateText(context, choices[i], width - PLD_CHOICE_PADDING * 2.0f);
        if (!choice->texts[i]) { goto error; }
        PLD_SetTextColor(choice->texts[i], (SDL_Color){255, 255, 255, 255});
        PLD_GetTextSize(context, choice->texts[i], &text_width, &text_height);
        if (text_height > max_height) { max_height = text_height; }
    }

    if (direction == PLD_CHOICE_VERTICAL)
    {
        choice->box_rect = (SDL_FRect){position.x, 0.0f, width, max_height + PLD_CHOICE_PADDING * 2.0f};
        choice->box_rect.x -= choice->box_rect.w / 2.0f;
        choice->box_rect.y = position.y - (choice->box_rect.h * num_choices + PLD_CHOICE_MARGIN * 2.0f * (num_choices - 1)) / 2.0f;
    }
    else
    {
        choice->box_rect = (SDL_FRect){0.0f, position.y, width, max_height + PLD_CHOICE_PADDING * 2.0f};
        choice->box_rect.x = position.x - (choice->box_rect.w * num_choices + PLD_CHOICE_MARGIN * 2.0f * (num_choices - 1)) / 2.0f;
        choice->box_rect.y -= choice->box_rect.h / 2.0f;
    }
    for (int i = 0; i < num_choices; i++)
    {
        choice->box_images[i] =  PLD_CreateImage(context, choice->box_rect.w, choice->box_rect.h);
        if (!choice->box_images[i]) { goto error; }
    }

    choice->box_queue = PLD_CreateAnimationQueue();
    if (!choice->box_queue) { goto error; }
    choice->effect_queue = PLD_CreateAnimationQueue();
    if (!choice->effect_queue) { goto error; }

    choice->open_animation = PLD_CreateAnimationFromStates(PLD_CHOICE_OPEN_ANIMATION_STATES, PLD_CHOICE_OPEN_ANIMATION_NUM_STATES);
    if (!choice->open_animation) { goto error; }
    choice->close_animation = PLD_CreateAnimationFromStates(PLD_CHOICE_CLOSE_ANIMATION_STATES, PLD_CHOICE_CLOSE_ANIMATION_NUM_STATES);
    if (!choice->close_animation) { goto error; }
    choice->switch_up_animation = PLD_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 0);
    if (!choice->switch_up_animation) { goto error; }
    choice->switch_down_animation = PLD_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 1);
    if (!choice->switch_down_animation) { goto error; }
    choice->switch_left_animation = PLD_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 2);
    if (!choice->switch_left_animation) { goto error; }
    choice->switch_right_animation = PLD_GetSwitchAnimation(choice->box_rect.w, choice->box_rect.h, 3);
    if (!choice->switch_right_animation) { goto error; }
    choice->left_arrow = PLD_LoadSVG_IO(context, SDL_IOFromConstMem(PLD_CHOICE_LEFT_ARROW_SVG, SDL_strlen(PLD_CHOICE_LEFT_ARROW_SVG)), true, 40, 40);
    if (!choice->left_arrow) { goto error; }

    choice->background = PLD_GetBoxSVG(context, PLD_CHOICE_BOX_SVG, choice->box_rect.w, choice->box_rect.h, PLD_CHOICE_BOX_BORDER_SIZE);
    if (!choice->background) { goto error; }
    choice->outline = PLD_GetBoxSVG(context, PLD_CHOICE_OUTLINE_SVG, choice->box_rect.w, choice->box_rect.h, PLD_CHOICE_OUTLINE_BORDER_SIZE);
    if (!choice->outline) { goto error; }

    choice->phase = PLD_CHOICE_CLOSED;
    choice->num_choices = num_choices;
    choice->direction = direction;

    return choice;

error:
    SDL_Log("Failed to create choice : %s", SDL_GetError());
    if (choice) { PLD_DestroyChoice(choice); }
    return NULL;
}

void PLD_DestroyChoice(PLD_Choice* choice)
{
    if (choice)
    {
        for (int i = 0; i < choice->num_choices; i++)
        {
            if ((choice->box_images) && (choice->box_images[i])) { PLD_DestroyImage(choice->box_images[i]); }
            if ((choice->texts) && (choice->texts[i])) { PLD_DestroyText(choice->texts[i]); }
        }
        if (choice->box_images) SDL_free(choice->box_images);
        if (choice->texts) SDL_free(choice->texts);
        if (choice->box_queue) { PLD_DestroyAnimationQueue(choice->box_queue); }
        if (choice->effect_queue) { PLD_DestroyAnimationQueue(choice->effect_queue); }
        if (choice->open_animation) { PLD_DestroyAnimation(choice->open_animation); }
        if (choice->close_animation) { PLD_DestroyAnimation(choice->close_animation); }
        if (choice->switch_up_animation) { PLD_DestroyAnimation(choice->switch_up_animation); }
        if (choice->switch_down_animation) { PLD_DestroyAnimation(choice->switch_down_animation); }
        if (choice->switch_left_animation) { PLD_DestroyAnimation(choice->switch_left_animation); }
        if (choice->switch_right_animation) { PLD_DestroyAnimation(choice->switch_right_animation); }
        if (choice->background) { PLD_DestroyImage(choice->background); }
        if (choice->outline) { PLD_DestroyImage(choice->outline); }
        if (choice->left_arrow) { PLD_DestroyImage(choice->left_arrow); }
        if (choice->options)
        {
            for (int i = 0; i < choice->num_choices; i++)
            {
                for (int j = 0; j < choice->num_options[i]; j++)
                {
                    if (choice->options[i][j]) { PLD_DestroyText(choice->options[i][j]); }
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

bool PLD_SetChoiceOptions(PLD_Context* context, PLD_Choice* choice, const char* const* const* options, const int* selected_options, const int* num_options)
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
            choice->options[i][j] = PLD_CreateText(context, options[i][j], 0);
            if (!choice->options[i][j]) { goto error; }
            PLD_SetTextColor(choice->options[i][j], (SDL_Color){255, 255, 255, 255});
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
    if (choice) { PLD_DestroyChoice(choice); }
    return false;
}

SDL_FPoint PLD_GetChoicePosition(PLD_Choice* choice, int i, bool center)
{
    SDL_FPoint position = {choice->box_rect.x, choice->box_rect.y};

    if (choice->direction == PLD_CHOICE_VERTICAL)
    {
        position.y += i * (choice->box_rect.h + PLD_CHOICE_MARGIN * 2.0f);
    }
    else
    {
        position.x += i * (choice->box_rect.w + PLD_CHOICE_MARGIN * 2.0f);
    }

    if (center)
    {
        position.x += choice->box_rect.w / 2.0f;
        position.y += choice->box_rect.h / 2.0f;
    }

    return position;
}

bool PLD_OpenChoice(PLD_Choice* choice, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to open choice : choice NULL"); return false; }
    if (choice->phase != PLD_CHOICE_CLOSED) { SDL_Log("Failed to open choice : choice not closed"); return false; }

    for (int i = 0; i < choice->num_choices; i++)
    {
        SDL_FPoint position = PLD_GetChoicePosition(choice, i, true);
        PLD_PlayAnimationWithCallback(choice->box_queue, choice->open_animation, choice->box_images[i], position, false, time, PLD_OpenChoiceCallback, choice);
    }

    choice->selected = 0;
    choice->phase = PLD_CHOICE_OPENING;

    return true;
}

bool PLD_CloseChoice(PLD_Choice* choice, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to close choice : choice NULL"); return false; }
    if (choice->phase != PLD_CHOICE_OPENED) { SDL_Log("Failed to close choice : choice not opened"); return false; }

    for (int i = 0; i < choice->num_choices; i++)
    {
        SDL_FPoint position = PLD_GetChoicePosition(choice, i, true);
        PLD_PlayAnimationWithCallback(choice->box_queue, choice->close_animation, choice->box_images[i], position, false, time, PLD_CloseChoiceCallback, choice);
    }

    choice->phase = PLD_CHOICE_CLOSING;

    return true;
}

bool PLD_RenderChoice(PLD_Context* context, PLD_Choice* choice, Uint64 time)
{
    for (int i = 0; i < choice->num_choices; i++)
    {
        PLD_SetRenderTarget(context, choice->box_images[i]);
        SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 0);
        SDL_RenderClear(context->renderer);
        SDL_FRect rect = {0.0f, 0.0f, choice->background->width, choice->background->height};
        PLD_RenderImage(context, choice->background, NULL, &rect, false, 0, NULL, 255);
        int text_width, text_height;
        if (choice->options)
        {
            PLD_GetTextSize(context, choice->texts[i], &text_width, &text_height);
            PLD_RenderText(context, choice->texts[i], (SDL_Point){PLD_CHOICE_MARGIN * 5, choice->box_rect.h / 2.0f}, 0, 255, false, 0);

            PLD_GetTextSize(context, choice->options[i][choice->selected_options[i]], &text_width, &text_height);
            PLD_RenderText(context, choice->options[i][choice->selected_options[i]], (SDL_Point){choice->box_rect.w - PLD_CHOICE_MARGIN * 8 - text_width, choice->box_rect.h / 2.0f}, 0, 255, false, 0);

            rect = (SDL_FRect){choice->box_rect.w - PLD_CHOICE_MARGIN * 11 - text_width, choice->box_rect.h / 2.0f + 5, choice->left_arrow->width, choice->left_arrow->height};
            if (choice->selected_options[i] != 0)
                PLD_RenderImage(context, choice->left_arrow, NULL, &rect, true, 0, NULL, 255);

            rect.x = choice->box_rect.w - PLD_CHOICE_MARGIN * 5;
            if (choice->selected_options[i] != choice->num_options[i] - 1)
                PLD_RenderImage(context, choice->left_arrow, NULL, &rect, true, 180, NULL, 255);
        }
        else
        {
            PLD_GetTextSize(context, choice->texts[i], &text_width, &text_height);
            PLD_RenderText(context, choice->texts[i], (SDL_Point){choice->box_rect.w / 2.0f - text_width / 2.0f, choice->box_rect.h / 2.0f}, 0, 255, false, 0);
        }
    }
    PLD_SetRenderTarget(context, NULL);

    PLD_RenderAnimationQueue(context, choice->box_queue, time);
    if (choice->phase == PLD_CHOICE_OPENED || choice->phase == PLD_CHOICE_SWITCHING)
    {
        for (int i = 0; i < choice->num_choices; i++)
        {
            SDL_FRect rect = choice->box_rect;
            SDL_FPoint position = PLD_GetChoicePosition(choice, i, false);
            rect.x = position.x;
            rect.y = position.y;
            PLD_RenderImage(context, choice->box_images[i], NULL, &rect, false, 0, NULL, 255);
            if ((choice->phase != PLD_CHOICE_SWITCHING) && (i == choice->selected))
            {
                PLD_RenderImage(context, choice->outline, NULL, &rect, false, 0, NULL, 255);
            }
        }
    }
    PLD_RenderAnimationQueue(context, choice->effect_queue, time);

    return true;
}

PLD_ChoicePhase PLD_GetChoicePhase(PLD_Choice* choice)
{
    if (!choice) { return PLD_CHOICE_INVALID; }
    return choice->phase;
}

int PLD_GetSelectedChoice(PLD_Choice* choice)
{
    if (!choice) { return -1; }
    return choice->selected;
}

bool PLD_SetSelectedChoice(PLD_Choice* choice, int index, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to switch choice : choice NULL"); return false; }
    if (choice->phase != PLD_CHOICE_OPENED) { SDL_Log("Failed to switch choice : choice not opened"); return false; }

    int old_choice = choice->selected;
    choice->selected = index;
    choice->selected = SDL_clamp(choice->selected, 0, choice->num_choices - 1);

    SDL_FPoint position = {choice->box_rect.x, choice->box_rect.y};
    if (choice->direction == PLD_CHOICE_VERTICAL)
    {
        position.y += old_choice * (choice->box_rect.h + PLD_CHOICE_MARGIN * 2.0f);
    }
    else
    {
        position.x += old_choice * (choice->box_rect.w + PLD_CHOICE_MARGIN * 2.0f);
    }
    position.x += choice->box_rect.w / 2.0f;
    position.y += choice->box_rect.h / 2.0f;

    if (old_choice - choice->selected > 0)
    {
        PLD_PlayAnimationWithCallback(choice->effect_queue, choice->direction == PLD_CHOICE_VERTICAL ? choice->switch_up_animation : choice->switch_left_animation, choice->outline, position, false, time, PLD_SwitchChoiceCallback, choice);
        choice->phase = PLD_CHOICE_SWITCHING;
    }
    else if (old_choice - choice->selected < 0)
    {
        PLD_PlayAnimationWithCallback(choice->effect_queue, choice->direction == PLD_CHOICE_VERTICAL ? choice->switch_down_animation : choice->switch_right_animation, choice->outline, position, false, time, PLD_SwitchChoiceCallback, choice);
        choice->phase = PLD_CHOICE_SWITCHING;
    }

    return true;
}

int PLD_GetSelectedOption(PLD_Choice* choice)
{
    if (!choice) { return -1; }
    return choice->selected_options[choice->selected];
}

bool PLD_SetSelectedOption(PLD_Choice* choice, int index, Uint64 time)
{
    if (!choice) { SDL_Log("Failed to switch choice : choice NULL"); return false; }
    if (choice->phase != PLD_CHOICE_OPENED) { SDL_Log("Failed to switch choice : choice not opened"); return false; }

    choice->selected_options[choice->selected] = index;
    choice->selected_options[choice->selected] = SDL_clamp(choice->selected_options[choice->selected], 0, choice->num_options[choice->selected] - 1);

    return true;
}