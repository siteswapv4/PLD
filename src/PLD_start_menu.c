#include "PLD_start_menu.h"
#include "PLDMenu.h"
#include "UI/UI_popup.h"
#include "UI/UI_choice.h"

static const char* PLD_START_TEXT =
"Welcome to PLD!\n"
"PLD stands for Project Light Dxxx, it's a fangame inspired by PPD.\n"
"PLD's goal is to let anyone play PPD charts on any device!\n"
"Have fun!";

#define PLD_START_NUM_CHOICES 2
const char* PLD_START_CHOICES[PLD_START_NUM_CHOICES] = {"Start Game", "Quit"};

typedef struct PLD_StartMenu
{
    UI_Popup* start_popup;
    UI_Choice* choice;
    int selected;
}PLD_StartMenu;

PLD_StartMenu* PLD_LoadStartMenu(PLD_Context* context)
{
    PLD_StartMenu* menu = SDL_calloc(1, sizeof(PLD_StartMenu));

    menu->start_popup = UI_CreatePopup(context->renderer, PLD_GetLogicalCenter(), PLD_LOGICAL_WIDTH * 0.7f, context->text_engine, context->font, PLD_START_TEXT);
    UI_OpenPopup(menu->start_popup, SDL_GetTicks());

    menu->choice = UI_CreateChoice(context->renderer, PLD_GetLogicalCenter(), PLD_LOGICAL_WIDTH * 0.3f, context->text_engine, context->font, PLD_START_CHOICES, PLD_START_NUM_CHOICES);
    menu->selected = -1;

    return menu;
}

void PLD_QuitStartMenu(PLD_StartMenu* menu)
{
    if (menu)
    {
        if (menu->start_popup) { UI_DestroyPopup(menu->start_popup); }
        if (menu->choice) { UI_DestroyChoice(menu->choice); }
        SDL_free(menu);
    }
}

PLD_StartMenuResult PLD_StartMenuKeyPress(PLD_Context* context, PLD_StartMenu* menu, SDL_Event* event)
{
    switch (PLD_GetMenuPressedInput(context, event))
    {
        case PLD_MENU_INPUT_EAST:
            if (UI_GetPopupPhase(menu->start_popup) == UI_POPUP_OPENED)
            {
                UI_ClosePopup(menu->start_popup, SDL_GetTicks());
                UI_OpenChoice(menu->choice, SDL_GetTicks());
            }
            if (UI_GetChoicePhase(menu->choice) == UI_CHOICE_OPENED)
            {
                menu->selected = UI_GetSelectedChoice(menu->choice);
                UI_CloseChoice(menu->choice, SDL_GetTicks());
            }
            break;

        case PLD_MENU_INPUT_UP:
            if (UI_GetChoicePhase(menu->choice) == UI_CHOICE_OPENED)
            {
                UI_SetSelectedChoice(menu->choice, UI_GetSelectedChoice(menu->choice) - 1, SDL_GetTicks());
            }
            break;

        case PLD_MENU_INPUT_DOWN:
            if (UI_GetChoicePhase(menu->choice) == UI_CHOICE_OPENED)
            {
                UI_SetSelectedChoice(menu->choice, UI_GetSelectedChoice(menu->choice) + 1, SDL_GetTicks());
            }
            break;

        default:
            break;
    }

    return PLD_START_MENU_CONTINUE;
}

PLD_StartMenuResult PLD_StartMenuLoop(PLD_Context* context, PLD_StartMenu* menu)
{
    UI_RenderPopup(context->renderer, menu->start_popup, SDL_GetTicks());
    UI_RenderChoice(context->renderer, menu->choice, SDL_GetTicks());

    if ((UI_GetPopupPhase(menu->start_popup) == UI_POPUP_CLOSED) && (UI_GetChoicePhase(menu->choice) == UI_CHOICE_CLOSED))
    {
        if (menu->selected == 0)
        {
            return PLD_START_MENU_NEXT;
        }
        else
        {
            return PLD_START_MENU_PREVIOUS;
        }
    }

    return PLD_START_MENU_CONTINUE;
}