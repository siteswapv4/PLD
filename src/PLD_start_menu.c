#include <PLD/PLD_start_menu.h>
#include <PLD/PLD_menu.h>
#include <UI/UI_popup.h>
#include <UI/UI_choice.h>

typedef enum PLD_StartChoice
{
    PLD_START_CHOICE_INVALID = -1,
    PLD_START_CHOICE_START,
    PLD_START_CHOICE_SETTINGS,
    PLD_START_CHOICE_QUIT,
    PLD_START_CHOICE_COUNT
}PLD_StartChoice;

typedef enum PLD_SettingsChoice
{
    PLD_SETTINGS_CHOICE_INVALID = -1,
    PLD_SETTINGS_CHOICE_GENERAL,
    PLD_SETTINGS_CHOICE_KEYBOARD,
    PLD_SETTINGS_CHOICE_GAMEPAD,
    PLD_SETTINGS_CHOICE_COUNT
}PLD_SettingsChoice;

typedef enum PLD_GeneralSettingsChoice
{
    PLD_GENERAL_SETTINGS_CHOICE_INVALID = -1,
    PLD_GENERAL_SETTINGS_CHOICE_WINDOW,
    PLD_GENERAL_SETTINGS_CHOICE_VSYNC,
    PLD_GENERAL_SETTINGS_CHOICE_FPS,
    PLD_GENERAL_SETTINGS_CHOICE_MEDIA,
    PLD_GENERAL_SETTINGS_CHOICE_MUSIC_VOLUME,
    PLD_GENERAL_SETTINGS_CHOICE_EFFECT_VOLUME,
    PLD_GENERAL_SETTINGS_CHOICE_COUNT
}PLD_GeneralSettingsChoice;

static const char* PLD_START_TEXT =
"Welcome to PLD!\n"
"PLD stands for Project Light Dxxx, it's a fangame inspired by PPD.\n"
"PLD's goal is to let anyone play PPD charts on any device!\n"
"Have fun!";

static const char* PLD_START_CHOICES[PLD_START_CHOICE_COUNT] = {"Start Game", "Settings", "Quit"};
static const char* PLD_SETTINGS_CHOICES[PLD_SETTINGS_CHOICE_COUNT] = {"General", "Keyboard", "Gamepad"};
static const char* PLD_GENERAL_SETTINGS_CHOICES[PLD_GENERAL_SETTINGS_CHOICE_COUNT] = {
    "Window",
    "Vsync",
    "FPS cap",
    "Auto load song media",
    "Music volume",
    "Effect volume"
};

static const char* option_set1[] = { "Fullscreen", "Windowed" };
static const char* option_set2[] = { "On", "Off" };
static const char* option_set3[] = { "30", "60", "120" };
static const char* option_set4[] = { "On", "Off" };
static const char* option_set5[] = { "0", "1", "2", "4", "4", "5", "6", "7", "8", "9", "10" };
static const char* option_set6[] = { "0", "1", "2", "4", "4", "5", "6", "7", "8", "9", "10" };

static const char** all_options[] = { option_set1, option_set2 };

static const char** PLD_GENERAL_SETTINGS_OPTIONS[] = {
    option_set1,
    option_set2,
    option_set3,
    option_set4,
    option_set5,
    option_set6
};

static int PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_COUNT] = {
    0,
    0,
    0,
    0,
    0,
    0
};

static const int PLD_GENERAL_SETTINGS_NUM_OPTIONS[PLD_GENERAL_SETTINGS_CHOICE_COUNT] = {
    2,
    2,
    3,
    2,
    11,
    11
};

typedef struct PLD_StartMenu
{
    UI_Popup* start_popup;
    UI_Choice* choice;
    UI_Choice* settings_choice;
    UI_Choice* general_settings_choice;
}PLD_StartMenu;

int PLD_FPSToOption(int fps)
{
    if (fps == 30)
    {
        return 0;
    }
    if (fps == 60)
    {
        return 1;
    }
    if (fps == 120)
    {
        return 2;
    }

    return 1;
}

void PLD_InitDefaultOptions(PLD_Context* context)
{
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_WINDOW] = !context->config->fullscreen;
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_VSYNC] = !context->config->vsync;
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_FPS] = PLD_FPSToOption(context->config->fps);
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_MEDIA] = !context->config->load_song_menu_media;
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_MUSIC_VOLUME] = context->config->music_volume * 10;
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_EFFECT_VOLUME] = context->config->sound_effect_volume * 10;
}

PLD_StartMenu* PLD_LoadStartMenu(PLD_Context* context, bool first)
{
    PLD_StartMenu* menu = SDL_calloc(1, sizeof(PLD_StartMenu));

    PLD_InitDefaultOptions(context);

    menu->start_popup = UI_CreatePopup(context->renderer, PLD_GetLogicalCenter(), PLD_LOGICAL_WIDTH * 0.7f, context->text_engine, context->font, PLD_START_TEXT);
    menu->choice = UI_CreateChoice(context->renderer, PLD_GetLogicalCenter(), PLD_LOGICAL_WIDTH * 0.3f, context->text_engine, context->font, PLD_START_CHOICES, PLD_START_CHOICE_COUNT, UI_CHOICE_VERTICAL);
    menu->settings_choice = UI_CreateChoice(context->renderer, (SDL_FPoint){PLD_LOGICAL_WIDTH / 2.0f, PLD_LOGICAL_HEIGHT * 0.08f}, PLD_LOGICAL_WIDTH * 0.3f, context->text_engine, context->font, PLD_SETTINGS_CHOICES, PLD_SETTINGS_CHOICE_COUNT, UI_CHOICE_HORIZONTAL);
    menu->general_settings_choice = UI_CreateChoice(context->renderer, (SDL_FPoint){PLD_LOGICAL_WIDTH / 2.0f, PLD_LOGICAL_HEIGHT * 0.55f}, PLD_LOGICAL_WIDTH * 0.92f, context->text_engine, context->font, PLD_GENERAL_SETTINGS_CHOICES, PLD_GENERAL_SETTINGS_CHOICE_COUNT, UI_CHOICE_VERTICAL);
    UI_SetChoiceOptions(menu->general_settings_choice, context->text_engine, context->font, (const char* const* const*)PLD_GENERAL_SETTINGS_OPTIONS, PLD_general_settings_selected_options, PLD_GENERAL_SETTINGS_NUM_OPTIONS);

    if (first)
    {
        UI_OpenPopup(menu->start_popup, SDL_GetTicks());
    }
    else
    {
        UI_OpenChoice(menu->choice, SDL_GetTicks());
    }

    return menu;
}

void PLD_QuitStartMenu(PLD_StartMenu* menu)
{
    if (menu)
    {
        if (menu->start_popup) { UI_DestroyPopup(menu->start_popup); }
        if (menu->choice) { UI_DestroyChoice(menu->choice); }
        if (menu->settings_choice) { UI_DestroyChoice(menu->settings_choice); }
        if (menu->general_settings_choice) { UI_DestroyChoice(menu->general_settings_choice); }
        SDL_free(menu);
    }
}

void PLD_UpdateConfig(PLD_Context* context, int choice, int option)
{
    switch (choice)
    {
        case PLD_GENERAL_SETTINGS_CHOICE_WINDOW:
            context->config->fullscreen = !option;
            break;

        case PLD_GENERAL_SETTINGS_CHOICE_VSYNC:
            context->config->vsync = !option;
            break;

        case PLD_GENERAL_SETTINGS_CHOICE_FPS:
            context->config->fps = SDL_atoi(PLD_GENERAL_SETTINGS_OPTIONS[choice][option]);
            break;

        case PLD_GENERAL_SETTINGS_CHOICE_MEDIA:
            context->config->load_song_menu_media = !option;
            break;
        
        case PLD_GENERAL_SETTINGS_CHOICE_MUSIC_VOLUME:
            context->config->music_volume = SDL_atoi(PLD_GENERAL_SETTINGS_OPTIONS[choice][option]) / 10.0f;
            break;
        
        case PLD_GENERAL_SETTINGS_CHOICE_EFFECT_VOLUME:
            context->config->sound_effect_volume = SDL_atoi(PLD_GENERAL_SETTINGS_OPTIONS[choice][option]) / 10.0f;
            break;

        default:
            break;
    }

    PLD_ApplyConfig(context);
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
            else if (UI_GetChoicePhase(menu->choice) == UI_CHOICE_OPENED)
            {
                UI_CloseChoice(menu->choice, SDL_GetTicks());
                switch (UI_GetSelectedChoice(menu->choice))
                {
                    case PLD_START_CHOICE_START:
                        return PLD_START_MENU_NEXT;
                
                    case PLD_START_CHOICE_SETTINGS:
                        if (UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_CLOSED)
                        {
                            UI_OpenChoice(menu->settings_choice, SDL_GetTicks());
                        }
                        break;
                
                    case PLD_START_CHOICE_QUIT:
                        return PLD_START_MENU_PREVIOUS;

                    default:
                        break;
                }
            } 
            else if ((UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_OPENED) && (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_CLOSED))
            {
                if (UI_GetSelectedChoice(menu->settings_choice) == PLD_SETTINGS_CHOICE_GENERAL)
                {
                    UI_OpenChoice(menu->general_settings_choice, SDL_GetTicks());
                }
            }
            break;

        case PLD_MENU_INPUT_SOUTH:
            if (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_OPENED)
            {
                PLD_WriteConfig(context);
                UI_CloseChoice(menu->general_settings_choice, SDL_GetTicks());
            }
            else if ((UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_CLOSED) && (UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_OPENED))
            {
                UI_CloseChoice(menu->settings_choice, SDL_GetTicks());
                UI_OpenChoice(menu->choice, SDL_GetTicks());
            }
            break;

        case PLD_MENU_INPUT_UP:
            if (UI_GetChoicePhase(menu->choice) == UI_CHOICE_OPENED)
            {
                UI_SetSelectedChoice(menu->choice, UI_GetSelectedChoice(menu->choice) - 1, SDL_GetTicks());
            }
            else if ((UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_OPENED) && (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_OPENED))
            {
                UI_SetSelectedChoice(menu->general_settings_choice, UI_GetSelectedChoice(menu->general_settings_choice) - 1, SDL_GetTicks());
            }
            break;

        case PLD_MENU_INPUT_DOWN:
            if (UI_GetChoicePhase(menu->choice) == UI_CHOICE_OPENED)
            {
                UI_SetSelectedChoice(menu->choice, UI_GetSelectedChoice(menu->choice) + 1, SDL_GetTicks());
            }
            else if ((UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_OPENED) && (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_OPENED))
            {
                UI_SetSelectedChoice(menu->general_settings_choice, UI_GetSelectedChoice(menu->general_settings_choice) + 1, SDL_GetTicks());
            }
            break;

        case PLD_MENU_INPUT_LEFT:
            if (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_OPENED)
            {
                UI_SetSelectedOption(menu->general_settings_choice, UI_GetSelectedOption(menu->general_settings_choice) - 1, SDL_GetTicks());
                PLD_UpdateConfig(context, UI_GetSelectedChoice(menu->general_settings_choice), UI_GetSelectedOption(menu->general_settings_choice));
            }
            else if ((UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_OPENED) && (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_CLOSED))
            {
                UI_SetSelectedChoice(menu->settings_choice, UI_GetSelectedChoice(menu->settings_choice) - 1, SDL_GetTicks());
            }
            break;

        case PLD_MENU_INPUT_RIGHT:
            if (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_OPENED)
            {
                UI_SetSelectedOption(menu->general_settings_choice, UI_GetSelectedOption(menu->general_settings_choice) + 1, SDL_GetTicks());
                PLD_UpdateConfig(context, UI_GetSelectedChoice(menu->general_settings_choice), UI_GetSelectedOption(menu->general_settings_choice));
            }
            else if ((UI_GetChoicePhase(menu->settings_choice) == UI_CHOICE_OPENED) && (UI_GetChoicePhase(menu->general_settings_choice) == UI_CHOICE_CLOSED))
            {
                UI_SetSelectedChoice(menu->settings_choice, UI_GetSelectedChoice(menu->settings_choice) + 1, SDL_GetTicks());
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
    UI_RenderChoice(context->renderer, menu->settings_choice, SDL_GetTicks());
    UI_RenderChoice(context->renderer, menu->general_settings_choice, SDL_GetTicks());

    return PLD_START_MENU_CONTINUE;
}