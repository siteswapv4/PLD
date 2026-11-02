#include <PLD/PLD_touch_config.h>
#include <PLD/PLD_touch.h>
#include <PLD/PLD_menu.h>
#include <PLD/PLD_text.h>
#include <PLD/PLD_image.h>

// Maybe move this out of here if later use is needed
typedef struct PLD_ClickButton
{
    SDL_FRect rect;
    PLD_Text* text;
}PLD_ClickButton;

PLD_ClickButton* PLD_CreateClickButton(PLD_Context* context, const char* text, const SDL_FRect* rect)
{
    PLD_ClickButton* button = SDL_calloc(1, sizeof(PLD_ClickButton));

    button->rect = *rect;
    button->text = PLD_CreateText(context, text, 0);

    return button;
}

void PLD_DestroyClickButton(PLD_ClickButton* button)
{
    if (!button)
        return;

    PLD_DestroyText(button->text);
    SDL_free(button);
}

bool PLD_RenderClickButton(PLD_Context* context, PLD_ClickButton* button)
{
    PLD_DrawRectangle(context, &button->rect, 0, 0, 0, 100);
    PLD_RenderText(context, button->text, (SDL_Point){button->rect.x + button->rect.w / 2, button->rect.y + button->rect.h / 2}, button->rect.w, 255, true, 0);

    return true;
}

bool PLD_ClickButtonPressed(PLD_ClickButton* button, const SDL_FPoint* position)
{
    return SDL_PointInRectFloat(position, &button->rect);
}

#define PLD_TOUCH_AREA_OFFSET 200.0f
#define PLD_TOUCH_BUTTON_MIN_SIZE 50.0f

static const SDL_FRect PLD_touch_area = {PLD_TOUCH_AREA_OFFSET, 
                                         PLD_TOUCH_AREA_OFFSET, 
                                         PLD_LOGICAL_WIDTH - PLD_TOUCH_AREA_OFFSET * 2.0f, 
                                         PLD_LOGICAL_HEIGHT - PLD_TOUCH_AREA_OFFSET * 2.0f};

typedef enum PLD_TouchConfigPhase
{
    PLD_TOUCH_CONFIG_PHASE_DEFAULT
}PLD_TouchConfigPhase;

typedef struct PLD_TouchConfigState
{
    PLD_TouchConfigPhase phase;
    PLD_TouchButton* selected;
    bool mouse_down;
    SDL_FPoint drag_offset;
    bool resizing;
    bool hide_ui;

    PLD_ClickButton* back_button;
    PLD_ClickButton* add_button;
    PLD_ClickButton* delete_button;
    PLD_ClickButton* hide_button;
}PLD_TouchConfigState;

bool PLD_ScaleTouchConfigDown(PLD_Context* context)
{
    int count = context->config->touch_buttons->len;

    if (count <= 0)
        return false;

    float scale_x = PLD_touch_area.w / PLD_LOGICAL_WIDTH;
    float scale_y = PLD_touch_area.h / PLD_LOGICAL_HEIGHT;

    float offset_x = PLD_touch_area.x;
    float offset_y = PLD_touch_area.y;

    for (int i = 0; i < count; i++)
    {
        PLD_TouchButton* button = context->config->touch_buttons->data[i];

        button->rect.x = offset_x + (button->rect.x / PLD_LOGICAL_WIDTH) * PLD_touch_area.w;
        button->rect.y = offset_y + (button->rect.y / PLD_LOGICAL_HEIGHT) * PLD_touch_area.h;

        button->rect.w = (button->rect.w / PLD_LOGICAL_WIDTH) * PLD_touch_area.w;
        button->rect.h = (button->rect.h / PLD_LOGICAL_HEIGHT) * PLD_touch_area.h;
    }

    return true;
}

bool PLD_ScaleTouchConfigUp(PLD_Context* context)
{
    int count = context->config->touch_buttons->len;

    if (count <= 0)
        return false;

    float scale_x = PLD_touch_area.w / PLD_LOGICAL_WIDTH;
    float scale_y = PLD_touch_area.h / PLD_LOGICAL_HEIGHT;

    float offset_x = PLD_touch_area.x;
    float offset_y = PLD_touch_area.y;

    for (int i = 0; i < count; i++)
    {
        PLD_TouchButton* button = context->config->touch_buttons->data[i];

        button->rect.x = ((button->rect.x - offset_x) / PLD_touch_area.w) * PLD_LOGICAL_WIDTH;
        button->rect.y = ((button->rect.y - offset_y) / PLD_touch_area.h) * PLD_LOGICAL_HEIGHT;

        button->rect.w = (button->rect.w / PLD_touch_area.w) * PLD_LOGICAL_WIDTH;
        button->rect.h = (button->rect.h / PLD_touch_area.h) * PLD_LOGICAL_HEIGHT;
    }

    return true;
}

PLD_TouchConfigState* PLD_InitTouchConfig(PLD_Context* context)
{
    PLD_TouchConfigState* config = SDL_calloc(1, sizeof(PLD_TouchConfigState));

    PLD_ResetTouchState(context);
    PLD_LoadImages(context, PLD_IMAGE_CATEGORY_GAMEPLAY);
    
    config->phase = PLD_TOUCH_CONFIG_PHASE_DEFAULT;

    PLD_ScaleTouchConfigDown(context);

    config->back_button = PLD_CreateClickButton(context, "←", &(SDL_FRect){10, 10, 150, 75});
    config->add_button = PLD_CreateClickButton(context, "+", &(SDL_FRect){200, 10, 75, 75});
    config->delete_button = PLD_CreateClickButton(context, "-", &(SDL_FRect){325, 10, 75, 75});
    config->hide_button = PLD_CreateClickButton(context, "hide UI", &(SDL_FRect){450, 10, 300, 75});

    return config;
}

void PLD_QuitTouchConfig(PLD_Context* context, PLD_TouchConfigState* config)
{
    if (!config) return;

    PLD_DestroyImages(PLD_IMAGE_CATEGORY_GAMEPLAY);

    PLD_ScaleTouchConfigUp(context);

    PLD_DestroyClickButton(config->back_button);
    PLD_DestroyClickButton(config->add_button);
    PLD_DestroyClickButton(config->delete_button);
    PLD_DestroyClickButton(config->hide_button);

    SDL_free(config);
}

SDL_FRect PLD_GetTouchResizeRect(const SDL_FRect* source)
{
    float ref = SDL_min(source->w, source->h);
    float hit_width = ref / 5.0f;
    float hit_height = ref / 5.0f;

    SDL_FRect rect = {source->x + source->w - hit_width,
                      source->y + source->h - hit_height,
                        hit_width,
                        hit_height};

    return rect;
}

PLD_TouchConfigResult PLD_TouchConfigMouseDown(PLD_Context* context, PLD_TouchConfigState* config, SDL_FPoint position)
{
    if (SDL_PointInRectFloat(&position, &PLD_touch_area))
    {
        bool was_selected = config->selected;
        bool found_selection = false;
        for (int i = 0; i < context->config->touch_buttons->len; i++)
        {
            PLD_TouchButton* button = context->config->touch_buttons->data[i];

            if (SDL_PointInRectFloat(&position, &button->rect))
            {
                config->selected = button;
                found_selection = true;
                config->drag_offset.x = position.x - button->rect.x;
                config->drag_offset.y = position.y - button->rect.y;
            }
        }
        if (!found_selection)
            config->selected = NULL;

        if (was_selected && config->selected)
        {
            SDL_FRect rect = PLD_GetTouchResizeRect(&config->selected->rect);

            if (SDL_PointInRectFloat(&position, &rect))
                config->resizing = true;
        }
    }
    else
    {
        if (PLD_ClickButtonPressed(config->back_button, &position))
            return PLD_TOUCH_CONFIG_SUCCESS;

        if (config->selected)
        {
            if (PLD_ClickButtonPressed(config->delete_button, &position))
            {
                PLD_ArrayListRemove(context->config->touch_buttons, config->selected);
                SDL_free(config->selected);
                config->selected = NULL;
            }
        }

        if (PLD_ClickButtonPressed(config->hide_button, &position))
            config->hide_ui = !config->hide_ui;
    }

    config->mouse_down = true;

    return PLD_TOUCH_CONFIG_CONTINUE;
}

bool PLD_TouchConfigMouseUp(PLD_Context* context, PLD_TouchConfigState* config, SDL_FPoint position)
{
    config->mouse_down = false;
    config->resizing = false;

    return true;
}

bool PLD_TouchConfigMouseMotion(PLD_Context* context, PLD_TouchConfigState* config, SDL_FPoint position)
{
    if (SDL_PointInRectFloat(&position, &PLD_touch_area) && config->mouse_down && config->selected)
    {
        if (config->resizing)
        {
            config->selected->rect.w = position.x - config->selected->rect.x;
            config->selected->rect.h = position.y - config->selected->rect.y;

            if (config->selected->rect.w < PLD_TOUCH_BUTTON_MIN_SIZE)
                config->selected->rect.w = PLD_TOUCH_BUTTON_MIN_SIZE;
            if (config->selected->rect.h < PLD_TOUCH_BUTTON_MIN_SIZE)
                config->selected->rect.h = PLD_TOUCH_BUTTON_MIN_SIZE;
        }
        else
        {
            config->selected->rect.x = position.x - config->drag_offset.x;
            config->selected->rect.y = position.y - config->drag_offset.y;
        }
    }

    return true;
}

PLD_TouchConfigResult PLD_TouchConfigEvent(PLD_Context* context, PLD_TouchConfigState* config, SDL_Event* event)
{
    PLD_MenuInput input = PLD_GetMenuPressedInput(context, event);

    if (input == PLD_MENU_INPUT_SOUTH)
        return PLD_TOUCH_CONFIG_SUCCESS;

    if ((event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) || (event->type == SDL_EVENT_MOUSE_BUTTON_UP))
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            SDL_FPoint position = {event->button.x, event->button.y};
            PLD_ScalePointBack(&position);

            if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                return PLD_TouchConfigMouseDown(context, config, position);
            }
            else
            {
                PLD_TouchConfigMouseUp(context, config, position);
            }
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        SDL_FPoint position = {event->motion.x, event->motion.y};
        PLD_ScalePointBack(&position);

        PLD_TouchConfigMouseMotion(context, config, position);
    }

    return PLD_TOUCH_CONFIG_CONTINUE;
}

bool PLD_RenderTouchConfigForeground(PLD_Context* context)
{
    PLD_Image* foreground_top = PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP);
    PLD_Image* foreground_bottom = PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM);

    float coeff = PLD_touch_area.w / PLD_LOGICAL_WIDTH;

    SDL_FRect rect = {PLD_touch_area.x, PLD_touch_area.y, foreground_top->width * coeff, foreground_top->height * coeff};
    PLD_RenderImage(context, foreground_top, NULL, &rect, false, 0, NULL, 255);

    rect.w = foreground_bottom->width * coeff;
    rect.h = foreground_bottom->height * coeff;
    rect.y = PLD_touch_area.y + PLD_touch_area.h - rect.h;
    PLD_RenderImage(context, foreground_bottom, NULL, &rect, false, 0, NULL, 255);

    return true;
}

PLD_TouchConfigResult PLD_TouchConfigIterate(PLD_Context* context, PLD_TouchConfigState* config)
{
    SDL_FRect window_rect = {0.0f, 0.0f, PLD_LOGICAL_WIDTH, PLD_LOGICAL_HEIGHT};
    PLD_DrawRectangle(context, &window_rect, 230, 230, 230, 255);

    PLD_SetClipRect(context, &PLD_touch_area);
    PLD_DrawRectangle(context, &window_rect, 255, 255, 255, 255);

    if (!config->hide_ui)
        PLD_RenderTouchConfigForeground(context);

    PLD_RenderTouch(context, true);

    if (config->selected)
    {
        PLD_DrawBorder(context, &config->selected->rect, &(SDL_Color){0, 0, 255, 255}, 5);
        SDL_FRect rect = PLD_GetTouchResizeRect(&config->selected->rect);
        PLD_DrawRectangle(context, &rect, 255, 100, 100, 255);
    }

    PLD_SetClipRect(context, NULL);

    PLD_DrawBorder(context, &PLD_touch_area, &(SDL_Color){0, 0, 0, 255}, 5);

    PLD_RenderClickButton(context, config->back_button);
    PLD_RenderClickButton(context, config->add_button);
    if (config->selected)
        PLD_RenderClickButton(context, config->delete_button);
    PLD_RenderClickButton(context, config->hide_button);

    return PLD_TOUCH_CONFIG_CONTINUE;
}