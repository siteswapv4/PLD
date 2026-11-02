#include <PLD/PLD_start_menu.h>
#include <PLD/PLD_menu.h>
#include <PLD/PLD_popup.h>
#include <PLD/PLD_choice.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_text.h>

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
    PLD_SETTINGS_CHOICE_CONTROLS,
    PLD_SETTINGS_CHOICE_TOUCHSCREEN,
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
    PLD_GENERAL_SETTINGS_CHOICE_ENABLE_VIDEO,
    PLD_GENERAL_SETTINGS_CHOICE_COUNT
}PLD_GeneralSettingsChoice;

static const char* PLD_START_TEXT =
"Welcome to PLD!\n"
"PLD stands for Project Light Dxxx, it's a fangame inspired by PPD.\n"
"PLD's goal is to let anyone play PPD charts on any device!\n"
"Have fun!";

static const char* PLD_START_CHOICES[PLD_START_CHOICE_COUNT] = {"Start Game", "Settings", "Quit"};
static const char* PLD_SETTINGS_CHOICES[PLD_SETTINGS_CHOICE_COUNT] = {"General", "Controls", "Touchscreen"};
static const char* PLD_GENERAL_SETTINGS_CHOICES[PLD_GENERAL_SETTINGS_CHOICE_COUNT] = {
    "Window",
    "Vsync",
    "FPS cap",
    "Auto load song media",
    "Music volume",
    "Effect volume",
    "Enable video"
};

static const char* option_set1[] = { "Fullscreen", "Windowed" };
static const char* option_set2[] = { "On", "Off" };
static const char* option_set3[] = { "30", "60", "120" };
static const char* option_set4[] = { "On", "Off" };
static const char* option_set5[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
static const char* option_set6[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
static const char* option_set7[] = { "On", "Off" };

static const char** PLD_GENERAL_SETTINGS_OPTIONS[] = {
    option_set1,
    option_set2,
    option_set3,
    option_set4,
    option_set5,
    option_set6,
    option_set7
};

static int PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_COUNT] = {
    0,
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
    11,
    2
};

typedef enum PLD_StartMenuPhase
{
    PLD_START_MENU_PHASE_FIRST_POPUP,
    PLD_START_MENU_PHASE_START_CHOICE,
    PLD_START_MENU_PHASE_SETTINGS,
    PLD_START_MENU_PHASE_GENERAL_SETTINGS
}PLD_StartMenuPhase;

typedef struct PLD_StartMenuState
{
    PLD_Popup* start_popup;
    PLD_Choice* start_choice;
    PLD_Choice* settings_choice;
    PLD_Choice* general_settings_choice;
    PLD_StartMenuPhase phase;
    PLD_Text* author_text;
    PLD_Text* change_data_text;
}PLD_StartMenuState;

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
    PLD_general_settings_selected_options[PLD_GENERAL_SETTINGS_CHOICE_ENABLE_VIDEO] = !context->config->enable_video;
}

PLD_StartMenuState* PLD_LoadStartMenu(PLD_Context* context, bool first)
{
    PLD_StartMenuState* menu = SDL_calloc(1, sizeof(PLD_StartMenuState));

    PLD_InitDefaultOptions(context);

    SDL_FPoint center = PLD_GetLogicalCenter();
    menu->start_popup = PLD_CreatePopup(context, center, PLD_LOGICAL_WIDTH * 0.7f, PLD_START_TEXT);
    if (!menu->start_popup) { goto error; }
    menu->start_choice = PLD_CreateChoice(context, (SDL_FPoint){center.x, center.y + 250.0f}, PLD_LOGICAL_WIDTH * 0.3f, PLD_START_CHOICES, PLD_START_CHOICE_COUNT, PLD_CHOICE_VERTICAL);
    if (!menu->start_choice) { goto error; }
    menu->settings_choice = PLD_CreateChoice(context, (SDL_FPoint){PLD_LOGICAL_WIDTH / 2.0f, PLD_LOGICAL_HEIGHT * 0.08f}, PLD_LOGICAL_WIDTH * 0.3f, PLD_SETTINGS_CHOICES, PLD_SETTINGS_CHOICE_COUNT, PLD_CHOICE_HORIZONTAL);
    if (!menu->settings_choice) { goto error; }
    menu->general_settings_choice = PLD_CreateChoice(context, (SDL_FPoint){PLD_LOGICAL_WIDTH / 2.0f, PLD_LOGICAL_HEIGHT * 0.55f}, PLD_LOGICAL_WIDTH * 0.92f, PLD_GENERAL_SETTINGS_CHOICES, PLD_GENERAL_SETTINGS_CHOICE_COUNT, PLD_CHOICE_VERTICAL);
    if (!menu->general_settings_choice) { goto error; }
    if (!PLD_SetChoiceOptions(context, menu->general_settings_choice, (const char* const* const*)PLD_GENERAL_SETTINGS_OPTIONS, PLD_general_settings_selected_options, PLD_GENERAL_SETTINGS_NUM_OPTIONS)) { goto error; }

    if (SDL_rand(2))
        menu->author_text = PLD_CreateText(context, "by siteswap", 0);
    else
        menu->author_text = PLD_CreateText(context, "by siteswapv4", 0);

    if (!menu->author_text) { goto error; }
    PLD_SetTextColor(menu->author_text, (SDL_Color){0, 0, 0, 255});
    
    if (first)
    {
        menu->change_data_text = PLD_CreateText(context, "△ to change data path", 0);
        if (!menu->change_data_text) goto error;
        PLD_SetTextColor(menu->change_data_text, (SDL_Color){0, 0, 0, 255});

        if (!PLD_OpenPopup(menu->start_popup, SDL_GetTicks())) { goto error; }
        menu->phase = PLD_START_MENU_PHASE_FIRST_POPUP;
    }
    else
    {
        if (!PLD_OpenChoice(menu->start_choice, SDL_GetTicks())) { goto error; }
        menu->phase = PLD_START_MENU_PHASE_START_CHOICE;
    }

    return menu;

error:
    SDL_Log("Failed to load start menu");
    if (menu) { PLD_QuitStartMenu(menu); }
    return NULL;
}

void PLD_QuitStartMenu(PLD_StartMenuState* menu)
{
    if (menu)
    {
        if (menu->start_popup) { PLD_DestroyPopup(menu->start_popup); }
        if (menu->start_choice) { PLD_DestroyChoice(menu->start_choice); }
        if (menu->settings_choice) { PLD_DestroyChoice(menu->settings_choice); }
        if (menu->general_settings_choice) { PLD_DestroyChoice(menu->general_settings_choice); }
        if (menu->author_text) { PLD_DestroyText(menu->author_text); }
        if (menu->change_data_text) PLD_DestroyText(menu->change_data_text);
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

        case PLD_GENERAL_SETTINGS_CHOICE_ENABLE_VIDEO:
            context->config->enable_video = !option;
            break;

        default:
            break;
    }

    PLD_ApplyConfig(context);
}

void PLD_ChangeDataPathCallback(void* userdata, const char* path)
{
    if (path == NULL)
        return;
    
    if (!PLD_ChangeDataPath(userdata, path))
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "Directory is not a valid data directory", NULL);
}

PLD_StartMenuResult PLD_StartMenuEvent(PLD_Context* context, PLD_StartMenuState* menu, SDL_Event* event)
{
    PLD_MenuInput input = PLD_GetMenuPressedInput(context, event);

    switch (menu->phase)
    {
        case PLD_START_MENU_PHASE_FIRST_POPUP:
            if (PLD_GetPopupPhase(menu->start_popup) == PLD_POPUP_OPENED)
            {
                if (input == PLD_MENU_INPUT_EAST)
                {
                    if (!PLD_ClosePopup(menu->start_popup, SDL_GetTicks())) { goto error; }
                }
            }
            break;

        case PLD_START_MENU_PHASE_START_CHOICE:
            if (PLD_GetChoicePhase(menu->start_choice) == PLD_CHOICE_OPENED)
            {
                if (input == PLD_MENU_INPUT_UP)
                {
                    if (!PLD_SetSelectedChoice(menu->start_choice, PLD_GetSelectedChoice(menu->start_choice) - 1, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_DOWN)
                {
                    if (!PLD_SetSelectedChoice(menu->start_choice, PLD_GetSelectedChoice(menu->start_choice) + 1, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_EAST)
                {
                    if (!PLD_CloseChoice(menu->start_choice, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_NORTH)
                {
                    if (menu->change_data_text)
                        PLD_ShowDirectoryPicker(PLD_ChangeDataPathCallback, context);
                }
            }
            break;

        case PLD_START_MENU_PHASE_SETTINGS:
            if (PLD_GetChoicePhase(menu->settings_choice) == PLD_CHOICE_OPENED)
            {
                if (input == PLD_MENU_INPUT_LEFT)
                {
                    if (!PLD_SetSelectedChoice(menu->settings_choice, PLD_GetSelectedChoice(menu->settings_choice) - 1, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_RIGHT)
                {
                    if (!PLD_SetSelectedChoice(menu->settings_choice, PLD_GetSelectedChoice(menu->settings_choice) + 1, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_EAST)
                {
                    switch (PLD_GetSelectedChoice(menu->settings_choice))
                    {
                        case PLD_SETTINGS_CHOICE_GENERAL:
                            if (!PLD_OpenChoice(menu->general_settings_choice, SDL_GetTicks())) { goto error; }
                            break;

                        case PLD_SETTINGS_CHOICE_TOUCHSCREEN:
                            return PLD_START_MENU_TOUCH_CONFIG;
                    }
                }
                else if (input == PLD_MENU_INPUT_SOUTH)
                {
                    if (PLD_GetChoicePhase(menu->general_settings_choice) != PLD_CHOICE_OPENING)
                    {
                        if (!PLD_CloseChoice(menu->settings_choice, SDL_GetTicks())) { goto error; }
                    }
                }
            }
            break;

        case PLD_START_MENU_PHASE_GENERAL_SETTINGS:
            if (PLD_GetChoicePhase(menu->general_settings_choice) == PLD_CHOICE_OPENED)
            {
                if (input == PLD_MENU_INPUT_UP)
                {
                    if (!PLD_SetSelectedChoice(menu->general_settings_choice, PLD_GetSelectedChoice(menu->general_settings_choice) - 1, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_DOWN)
                {
                    if (!PLD_SetSelectedChoice(menu->general_settings_choice, PLD_GetSelectedChoice(menu->general_settings_choice) + 1, SDL_GetTicks())) { goto error; }
                }
                else if (input == PLD_MENU_INPUT_LEFT)
                {
                    if (!PLD_SetSelectedOption(menu->general_settings_choice, PLD_GetSelectedOption(menu->general_settings_choice) - 1, SDL_GetTicks())) { goto error; }
                    PLD_UpdateConfig(context, PLD_GetSelectedChoice(menu->general_settings_choice), PLD_GetSelectedOption(menu->general_settings_choice));
                }
                else if (input == PLD_MENU_INPUT_RIGHT)
                {
                    if (!PLD_SetSelectedOption(menu->general_settings_choice, PLD_GetSelectedOption(menu->general_settings_choice) + 1, SDL_GetTicks())) { goto error; }
                    PLD_UpdateConfig(context, PLD_GetSelectedChoice(menu->general_settings_choice), PLD_GetSelectedOption(menu->general_settings_choice));
                }
                else if (input == PLD_MENU_INPUT_SOUTH)
                {
                    if (!PLD_CloseChoice(menu->general_settings_choice, SDL_GetTicks())) { goto error; }
                    if (!PLD_WriteConfig(context)) { goto error; }
                }
            }
            break;

        default:
            break;
    }

    return PLD_START_MENU_CONTINUE;

error:
    return PLD_START_MENU_FAILURE;
}

PLD_StartMenuResult PLD_StartMenuIterate(PLD_Context* context, PLD_StartMenuState* menu)
{
    switch (menu->phase)
    {
        case PLD_START_MENU_PHASE_FIRST_POPUP:
            if (PLD_GetPopupPhase(menu->start_popup) == PLD_POPUP_CLOSED)
            {
                if (!PLD_OpenChoice(menu->start_choice, SDL_GetTicks())) { goto error; }
                menu->phase = PLD_START_MENU_PHASE_START_CHOICE;
            }
            break;

        case PLD_START_MENU_PHASE_START_CHOICE:
            if (PLD_GetChoicePhase(menu->start_choice) == PLD_CHOICE_CLOSED)
            {
                switch (PLD_GetSelectedChoice(menu->start_choice))
                {
                    case PLD_START_CHOICE_START:
                        return PLD_START_MENU_NEXT;

                    case PLD_START_CHOICE_SETTINGS:
                        if (!PLD_OpenChoice(menu->settings_choice, SDL_GetTicks())) { goto error; }
                        menu->phase = PLD_START_MENU_PHASE_SETTINGS;
                        break;

                    case PLD_START_CHOICE_QUIT:
                        return PLD_START_MENU_PREVIOUS;
                    
                    default:
                        break;
                }
            }
            break;

        case PLD_START_MENU_PHASE_SETTINGS:
            if (PLD_GetChoicePhase(menu->settings_choice) == PLD_CHOICE_CLOSED)
            {
                if (!PLD_OpenChoice(menu->start_choice, SDL_GetTicks())) { goto error; }
                menu->phase = PLD_START_MENU_PHASE_START_CHOICE;
            }
            else
            {
                if (PLD_GetChoicePhase(menu->general_settings_choice) == PLD_CHOICE_OPENED)
                {
                    menu->phase = PLD_START_MENU_PHASE_GENERAL_SETTINGS;
                }
            }
            break;

        case PLD_START_MENU_PHASE_GENERAL_SETTINGS:
            if (PLD_GetChoicePhase(menu->general_settings_choice) == PLD_CHOICE_CLOSED)
            {
                menu->phase = PLD_START_MENU_PHASE_SETTINGS;
            }
            break;

        default:
            break;
    }

    PLD_RenderPopup(context, menu->start_popup, SDL_GetTicks());
    PLD_RenderChoice(context, menu->start_choice, SDL_GetTicks());
    PLD_RenderChoice(context, menu->settings_choice, SDL_GetTicks());
    PLD_RenderChoice(context, menu->general_settings_choice, SDL_GetTicks());

    if (menu->phase == PLD_START_MENU_PHASE_START_CHOICE)
    {
        PLD_Image* logo = PLD_GetImage(PLD_MENU_IMAGE_LOGO);
        SDL_FRect rect = {PLD_LOGICAL_WIDTH / 2, 300, logo->width, logo->height};
        PLD_RenderImage(context, logo, NULL, &rect, true, 0, NULL, 255);

        int text_width, text_height;
        PLD_GetTextSize(context, menu->author_text, &text_width, &text_height);
        PLD_RenderText(context, menu->author_text, (SDL_Point){0, PLD_LOGICAL_HEIGHT - text_height / 2}, 0, 255, false, 0);
        
        if (menu->change_data_text)
            PLD_RenderText(context, menu->change_data_text, (SDL_Point){PLD_LOGICAL_WIDTH / 2, text_height / 2}, 0, 255, true, 0);
    }

    return PLD_START_MENU_CONTINUE;

error:
    return PLD_START_MENU_FAILURE;
}
