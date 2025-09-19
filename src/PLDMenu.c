#include "PLDMenu.h"


PLD_MenuInput PLD_GetMenuPressedInput(PLD_Context* context, SDL_Event* event)
{
    if ((event->type == SDL_EVENT_KEY_DOWN) && (event->key.repeat == 0))
    {
        for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
        {
            if (context->config->menu_keys[i] == event->key.scancode)
            {
                return i;
            }
        }
    }
    else if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
    {
        for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
        {
            if (context->config->menu_buttons[i] == event->gbutton.button)
            {
                return i;
            }
        }
    }

    return PLD_MENU_INPUT_INVALID;
}
