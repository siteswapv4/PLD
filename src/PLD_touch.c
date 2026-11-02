#include <PLD/PLD_touch.h>

#define PLD_SHOW_TOUCH_DELAY 10000
#define PLD_TOUCH_SLIDE_OFFSET 100.0f // pixels

typedef struct PLD_HeldTouch
{
    SDL_FingerID id;
    PLD_Button button;
    int index;
    int sliding; // 0 no, 1 left, 2 right
    SDL_FPoint start_position;
}PLD_HeldTouch;

typedef struct PLD_TouchImage
{
    PLD_Image* image;
    int num;
}PLD_TouchImage;

static bool PLD_touch_init = false;
static int PLD_num_sliding = false;

static Uint64 PLD_last_touch = 0;
static bool PLD_screen_touched = false;

static bool* PLD_touch_is_held = NULL;
static PLD_ArrayList* PLD_held_touches = NULL;

PLD_ArrayList* PLD_touch_images = NULL;

bool PLD_InitTouch(PLD_Context* context)
{
    if (PLD_touch_init)
        return false;

    PLD_num_sliding = 0;
    PLD_last_touch = SDL_GetTicks();
    PLD_screen_touched = false;
    PLD_held_touches = PLD_CreateArrayList();
    PLD_touch_images = PLD_CreateArrayList();
    PLD_touch_is_held = SDL_calloc(context->config->touch_buttons->len, sizeof(bool));

    char* path;
    SDL_asprintf(&path, "%s/images/touch/", context->data_path);
    PLD_ArrayList* files = PLD_GetDirectoryContent(path, true, false, false);
    SDL_free(path);
    if (!files) return true;

    for (int i = 0; i < files->len; i++)
    {
        int num = 0;
        SDL_sscanf((char*)files->data[i], "%d", &num);
        SDL_asprintf(&path, "%s/images/touch/%s", context->data_path, (char*)files->data[i]);
        PLD_TouchImage* image = SDL_malloc(sizeof(PLD_TouchImage));
        image->image = PLD_LoadImage(context, path);
        image->num = num;
        SDL_free(path);

        PLD_ArrayListAdd(PLD_touch_images, image);
    }
    PLD_DestroyArrayList(files, SDL_free);
    
    PLD_touch_init = true;
    
    return true;
}

void PLD_RenderTouch(PLD_Context* context, bool force)
{
    if (!PLD_touch_init)
        return;

    if (force || ((PLD_screen_touched) && (SDL_GetTicks() - PLD_last_touch < PLD_SHOW_TOUCH_DELAY)))
    {
        for (int i = 0; i < context->config->touch_buttons->len; i++)
        {
            SDL_FRect rect = ((PLD_TouchButton*)context->config->touch_buttons->data[i])->rect;
            bool has_image = false;
            for (int j = 0; j < PLD_touch_images->len; j++)
            {
                if (((PLD_TouchImage*)PLD_touch_images->data[j])->num == i)
                {
                    PLD_RenderImage(context, ((PLD_TouchImage*)PLD_touch_images->data[j])->image, NULL, &rect, false, 0.0, NULL, PLD_touch_is_held[i] ? 100 : 255);
                    has_image = true;
                    break;
                }
            }
            if (!has_image)
            {
                if (PLD_touch_is_held[i])
                {
                    PLD_DrawRectangle(context, &rect, 100, 100, 100, 100);
                }
                else
                {
                    PLD_DrawRectangle(context, &rect, 0, 0, 0, 100);
                }
            }
        }
    }
}

void PLD_ResetTouchState(PLD_Context* context)
{
    if (!PLD_touch_init)
        return;
        
    SDL_memset(PLD_touch_is_held, 0, context->config->touch_buttons->len * sizeof(bool));
    while (PLD_held_touches->len > 0)
        SDL_free(PLD_ArrayListRemoveAt(PLD_held_touches, 0));
}

void PLD_TouchEvent(PLD_Context* context, SDL_Event* event)
{
    if (!PLD_touch_init)
        return;
        
    PLD_screen_touched = true;
    PLD_last_touch = SDL_GetTicks();
    
    SDL_FPoint finger_position = {event->tfinger.x, event->tfinger.y};
    PLD_ScalePointBack(&finger_position);
    
    if (event->type == SDL_EVENT_FINGER_DOWN)
    {
        PLD_Button button = PLD_BUTTON_INVALID;
        int touch_index = -1;

        for (int i = 0; i < context->config->touch_buttons->len; i++)
        {
            PLD_TouchButton* touch_button = context->config->touch_buttons->data[i];

            if (SDL_PointInRectFloat(&finger_position, &touch_button->rect))
            {
                button = touch_button->button;
                touch_index = i;
                PLD_touch_is_held[i] = true;

                break;
            }
        }

        PLD_HeldTouch* held_touch = SDL_malloc(sizeof(PLD_HeldTouch));
        held_touch->id = event->tfinger.fingerID;
        held_touch->button = button;
        held_touch->index = touch_index;
        held_touch->sliding = 0;
        held_touch->start_position = finger_position;
        PLD_ArrayListAdd(PLD_held_touches, held_touch);

        SDL_Event new_event = {0};
        new_event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        new_event.gbutton.button = button;
        SDL_PushEvent(&new_event);
    }
    else if (event->type == SDL_EVENT_FINGER_MOTION)
    {
        for (int i = 0; i < PLD_held_touches->len; i++)
        {
            PLD_HeldTouch* held_touch = PLD_held_touches->data[i];

            if (held_touch->id == event->tfinger.fingerID)
            {
                if ((held_touch->start_position.x - finger_position.x > PLD_TOUCH_SLIDE_OFFSET) && (!held_touch->sliding || (held_touch->sliding == 2)))
                {
                    held_touch->start_position = finger_position;
                    SDL_Event new_event = {0};
                    new_event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                    new_event.gbutton.button = (PLD_num_sliding == 0) ? PLD_BUTTON_LEFT_STICK_LEFT : PLD_BUTTON_RIGHT_STICK_LEFT;
                    SDL_PushEvent(&new_event);

                    if (!held_touch->sliding)
                        PLD_num_sliding++;

                    held_touch->sliding = 1;
                }
                else if ((held_touch->start_position.x - finger_position.x < -PLD_TOUCH_SLIDE_OFFSET) && (!held_touch->sliding || (held_touch->sliding == 1)))
                {
                    held_touch->start_position = finger_position;
                    SDL_Event new_event = {0};
                    new_event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                    new_event.gbutton.button = (PLD_num_sliding == 0) ? PLD_BUTTON_LEFT_STICK_RIGHT : PLD_BUTTON_RIGHT_STICK_RIGHT;
                    SDL_PushEvent(&new_event);

                    if (!held_touch->sliding)
                        PLD_num_sliding++;

                    held_touch->sliding = 2;
                }
            }
        }
    }
    else if ((event->type == SDL_EVENT_FINGER_UP) || (event->type == SDL_EVENT_FINGER_CANCELED))
    {
        for (int i = PLD_held_touches->len - 1; i >= 0; i--) // backwards to delete stuff
        {
            PLD_HeldTouch* held_touch = PLD_held_touches->data[i];
            if (held_touch->id == event->tfinger.fingerID)
            {
                PLD_touch_is_held[held_touch->index] = false;
                SDL_Event new_event = {0};
                new_event.type = SDL_EVENT_GAMEPAD_BUTTON_UP;
                new_event.gbutton.button = held_touch->button;
                SDL_PushEvent(&new_event);

                if (held_touch->sliding)
                {
                    if (held_touch->sliding == 1)
                    {
                        new_event.gbutton.button = (PLD_num_sliding == 2) ? PLD_BUTTON_RIGHT_STICK_LEFT : PLD_BUTTON_LEFT_STICK_LEFT;
                    }
                    else
                    {
                        new_event.gbutton.button = (PLD_num_sliding == 2) ? PLD_BUTTON_RIGHT_STICK_RIGHT : PLD_BUTTON_LEFT_STICK_RIGHT;
                    }

                    SDL_PushEvent(&new_event);
                    PLD_num_sliding--;
                }

                SDL_free(PLD_ArrayListRemoveAt(PLD_held_touches, i));
            }
        }
    }
}

void PLD_QuitTouch()
{
    if (!PLD_touch_init)
        return;

    PLD_DestroyArrayList(PLD_held_touches, SDL_free);
    PLD_held_touches = NULL;

    if (PLD_touch_images)
    {
        for (int i = 0; i < PLD_touch_images->len; i++)
        {
            PLD_TouchImage* img = (PLD_TouchImage*)PLD_touch_images->data[i];
            if (img)
            {
                PLD_DestroyImage(img->image);
            }
        }
        PLD_DestroyArrayList(PLD_touch_images, SDL_free);
        PLD_touch_images = NULL;
    }

    SDL_free(PLD_touch_is_held);
    PLD_touch_is_held = NULL;

    PLD_touch_init = false;
}
