#include <PLD/PLD_context.h>
#include <PLD/PLD_sound.h>
#include <PLD/PLD_music.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_effect.h>
#include <PLD/PLD_touch.h>
#include <PLD/PLD_user.h>
#include <PLD/PLD_text.h>

#define PLD_CONFIG_NAME "PLD.config"
#define PLD_LOG_NAME "PLD.log"

#define PLD_SDL_INIT_FLAGS SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO

#define PLD_WINDOW_NAME "PLD"
#define PLD_WINDOW_FLAGS SDL_WINDOW_RESIZABLE

#if defined(SDL_PLATFORM_PSP)
int PLD_window_width = 480;
int PLD_window_height = 270;
#elif defined(SDL_PLATFORM_N3DS)
int PLD_window_width = 800;
int PLD_window_height = 240;
#else
int PLD_window_width = 960;
int PLD_window_height = 540;
#endif

static float PLD_window_scale_x = 0;
static float PLD_window_scale_y = 0;
static float PLD_window_scale = 0;
static int PLD_window_scaled_width = 0;
static int PLD_window_scaled_height = 0;
static int PLD_window_offset_x = 0;
static int PLD_window_offset_y = 0;

static const Uint32 PLD_CONFIG_VERSION = 0x01010101;

const bool PLD_DEFAULT_FULLSCREEN = false;
const bool PLD_DEFAULT_VSYNC = false;
const bool PLD_DEFAULT_AUTOPLAY = false;
const Sint32 PLD_DEFAULT_FPS = 60;
const Sint32 PLD_DEFAULT_OFFSET = 0;
const bool PLD_DEFAULT_LOAD_SONG_MENU_MEDIA = true;
const Sint32 PLD_DEFAULT_IMAGE_WIDTH = PLD_LOGICAL_WIDTH;
const Sint32 PLD_DEFAULT_IMAGE_HEIGHT = PLD_LOGICAL_HEIGHT;
const float PLD_DEFAULT_MUSIC_VOLUME = 1.0f;
const float PLD_DEFAULT_SOUND_EFFECT_VOLUME = 1.0f;
const bool PLD_DEFAULT_ENABLE_VIDEO = true;
const bool PLD_DEFAULT_BUTTON_FOR_SLIDER = false;

const Sint32 PLD_CONFIG_STOP_FLAG = -1;

const int PLD_NUM_DEFAULT_GAMEPLAY_KEYS[PLD_GAMEPLAY_INPUT_COUNT] = {
    2,
    2,
    2,
    2,
    2,
    2
};

const Sint32 PLD_DEFAULT_GAMEPLAY_KEYS[PLD_GAMEPLAY_INPUT_COUNT][2] = {
    {SDL_SCANCODE_W, SDL_SCANCODE_I},
    {SDL_SCANCODE_A, SDL_SCANCODE_J},
    {SDL_SCANCODE_S, SDL_SCANCODE_K},
    {SDL_SCANCODE_D, SDL_SCANCODE_L},
    {SDL_SCANCODE_Q, SDL_SCANCODE_U},
    {SDL_SCANCODE_E, SDL_SCANCODE_O}
};

const int PLD_NUM_DEFAULT_GAMEPLAY_BUTTONS[PLD_GAMEPLAY_INPUT_COUNT] = {
    5,
    5,
    5,
    5,
    2,
    2
};

const Sint32 PLD_DEFAULT_GAMEPLAY_BUTTONS[PLD_GAMEPLAY_INPUT_COUNT][5] = {
    {PLD_BUTTON_NORTH, PLD_BUTTON_DPAD_UP, PLD_BUTTON_LEFT_SHOULDER, PLD_BUTTON_LEFT_TRIGGER, PLD_BUTTON_RIGHT_TRIGGER},
    {PLD_BUTTON_WEST, PLD_BUTTON_DPAD_LEFT, PLD_BUTTON_RIGHT_SHOULDER, PLD_BUTTON_LEFT_TRIGGER, PLD_BUTTON_RIGHT_TRIGGER},
    {PLD_BUTTON_SOUTH, PLD_BUTTON_DPAD_DOWN, PLD_BUTTON_LEFT_SHOULDER, PLD_BUTTON_LEFT_TRIGGER, PLD_BUTTON_RIGHT_TRIGGER},
    {PLD_BUTTON_EAST, PLD_BUTTON_DPAD_RIGHT, PLD_BUTTON_RIGHT_SHOULDER,  PLD_BUTTON_LEFT_TRIGGER, PLD_BUTTON_RIGHT_TRIGGER},
    {PLD_BUTTON_LEFT_STICK_LEFT, PLD_BUTTON_RIGHT_STICK_LEFT, -1, -1, -1},
    {PLD_BUTTON_LEFT_STICK_RIGHT, PLD_BUTTON_RIGHT_STICK_RIGHT, -1, -1, -1}
};

const Sint32 PLD_DEFAULT_MENU_KEYS[PLD_MENU_INPUT_COUNT] = {
    SDL_SCANCODE_W,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_I,
    SDL_SCANCODE_J,
    SDL_SCANCODE_K,
    SDL_SCANCODE_L,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_E,
    SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_RETURN
};

const Sint32 PLD_DEFAULT_MENU_BUTTONS[PLD_MENU_INPUT_COUNT] = {
    PLD_BUTTON_DPAD_UP,
    PLD_BUTTON_DPAD_LEFT,
    PLD_BUTTON_DPAD_DOWN,
    PLD_BUTTON_DPAD_RIGHT,
    PLD_BUTTON_NORTH,
    PLD_BUTTON_WEST,
    PLD_BUTTON_SOUTH,
    PLD_BUTTON_EAST,
    PLD_BUTTON_LEFT_SHOULDER,
    PLD_BUTTON_RIGHT_SHOULDER,
    PLD_BUTTON_BACK,
    PLD_BUTTON_START
};

const Sint32 PLD_DEFAULT_TRIGGER_DEADZONE = 26000;
const Sint32 PLD_DEFAULT_JOYSTICK_DEADZONE = 26000;

#define PLD_DEFAULT_TOUCH_BUTTONS_COUNT 9

const PLD_TouchButton PLD_DEFAULT_TOUCH_BUTTONS[PLD_DEFAULT_TOUCH_BUTTONS_COUNT] = {
    {{710, 0, 100, 100}, PLD_BUTTON_DPAD_UP},
    {{910, 0, 100, 100}, PLD_BUTTON_START},
    {{1110, 0, 100, 100}, PLD_BUTTON_DPAD_DOWN},
    {{510, 0, 100, 100}, PLD_BUTTON_DPAD_LEFT},
    {{328, 802, 278, 278}, PLD_BUTTON_NORTH},
    {{656, 802, 278, 278}, PLD_BUTTON_WEST},
    {{984, 802, 278, 278}, PLD_BUTTON_SOUTH},
    {{1312, 802, 278, 278}, PLD_BUTTON_EAST},
    {{1310, 0, 100, 100}, PLD_BUTTON_DPAD_RIGHT}
};

SDL_IOStream* log_file = NULL;

void PLD_Log(void* user_data, int category, SDL_LogPriority priority, const char* message)
{
    if (log_file != NULL)
    {
        SDL_WriteIO(log_file, message, SDL_strlen(message));
        SDL_WriteIO(log_file, "\n", 1);
        SDL_FlushIO(log_file);
    }
}

bool PLD_OpenLog(PLD_Context* context)
{
    if (log_file)
        SDL_CloseIO(log_file);

    char* log_path;
    SDL_asprintf(&log_path, "%s%s", context->data_path, PLD_LOG_NAME);
    log_file = SDL_IOFromFile(log_path, "wb");
    SDL_free(log_path);

    return log_file != NULL;
}

bool PLD_GetDataPath(PLD_Context* context)
{
#if defined(SDL_PLATFORM_ANDROID)

    int state = SDL_GetAndroidExternalStorageState();
    if (!(state & SDL_ANDROID_EXTERNAL_STORAGE_READ))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to read from external storage");
        return false;
    }

    if (!(state & SDL_ANDROID_EXTERNAL_STORAGE_WRITE))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to write to external storage");
        return false;
    }

    const char* path = SDL_GetAndroidExternalStoragePath();
    if (path == NULL)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }

    SDL_asprintf(&context->data_path, "%s/", path);
    return true;

#elif defined(SDL_PLATFORM_VITA)

    context->data_path =  SDL_strdup("ux0:data/PLD/");
    return true;

#elif defined(SDL_PLATFORM_EMSCRIPTEN)

	context->data_path = SDL_strdup("/data/");
	return true;

#else

    const char* path = SDL_GetBasePath();
    if (path == NULL)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }

    SDL_asprintf(&context->data_path, "%sdata/", path);

    return true;

#endif
}


void PLD_DestroyConfig(PLD_Config* config)
{
    if (config != NULL)
    {
        for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
        {
            if (config->gameplay_keys[i] != NULL)
            {
                PLD_DestroyArrayList(config->gameplay_keys[i], SDL_free);
            }

            if (config->gameplay_buttons[i] != NULL)
            {
                PLD_DestroyArrayList(config->gameplay_buttons[i], SDL_free);
            }
        }

        if (config->touch_buttons) { PLD_DestroyArrayList(config->touch_buttons, SDL_free); }

        SDL_free(config);
    }
}


PLD_Config* PLD_CreateDefaultConfig()
{
    PLD_Config* config = SDL_calloc(1, sizeof(PLD_Config));

    config->fullscreen = PLD_DEFAULT_FULLSCREEN;
    config->vsync = PLD_DEFAULT_VSYNC;
    config->autoplay = PLD_DEFAULT_AUTOPLAY;
    config->fps = PLD_DEFAULT_FPS;
    config->offset = PLD_DEFAULT_OFFSET;
    config->load_song_menu_media = PLD_DEFAULT_LOAD_SONG_MENU_MEDIA;
    config->image_width = PLD_DEFAULT_IMAGE_WIDTH;
    config->image_height = PLD_DEFAULT_IMAGE_HEIGHT;
    config->music_volume = PLD_DEFAULT_MUSIC_VOLUME;
    config->sound_effect_volume = PLD_DEFAULT_SOUND_EFFECT_VOLUME;
    config->enable_video = PLD_DEFAULT_ENABLE_VIDEO;
    config->button_for_slider = PLD_DEFAULT_BUTTON_FOR_SLIDER;

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        config->gameplay_keys[i] = PLD_CreateArrayList();
        config->gameplay_buttons[i] = PLD_CreateArrayList();

        for (int j = 0; j < PLD_NUM_DEFAULT_GAMEPLAY_KEYS[i]; j++)
        {
            Sint32* scancode = SDL_malloc(sizeof(Sint32));
            *scancode = PLD_DEFAULT_GAMEPLAY_KEYS[i][j];
            PLD_ArrayListAdd(config->gameplay_keys[i], scancode);
        }

        for (int j = 0; j < PLD_NUM_DEFAULT_GAMEPLAY_BUTTONS[i]; j++)
        {
            Sint32* button = SDL_malloc(sizeof(Sint32));
            *button = PLD_DEFAULT_GAMEPLAY_BUTTONS[i][j];
            PLD_ArrayListAdd(config->gameplay_buttons[i], button);
        }
    }

    for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
    {
        config->menu_keys[i] = PLD_DEFAULT_MENU_KEYS[i];
        config->menu_buttons[i] = PLD_DEFAULT_MENU_BUTTONS[i];
    }

    config->trigger_deadzone = PLD_DEFAULT_TRIGGER_DEADZONE;
    config->joystick_deadzone = PLD_DEFAULT_JOYSTICK_DEADZONE;

    config->touch_buttons = PLD_CreateArrayList();
    for (int i = 0; i < PLD_DEFAULT_TOUCH_BUTTONS_COUNT; i++)
    {
        PLD_TouchButton* touch_button = SDL_malloc(sizeof(PLD_TouchButton));
        touch_button->rect = PLD_DEFAULT_TOUCH_BUTTONS[i].rect;
        touch_button->button = PLD_DEFAULT_TOUCH_BUTTONS[i].button;
        PLD_ArrayListAdd(config->touch_buttons, touch_button);
    }

    return config;
}


bool PLD_ReadConfig(PLD_Context* context)
{
    context->config = SDL_calloc(1, sizeof(PLD_Config));

    char* config_path = NULL;
    SDL_IOStream* config_file = NULL;

    SDL_asprintf(&config_path, "%s%s", context->data_path, PLD_CONFIG_NAME);
    config_file = SDL_IOFromFile(config_path, "rb");
    if (config_file == NULL)
    {
        goto error;
    }

    Uint32 version;

    PLD_READ_CHECK(config_file, &version, sizeof(Uint32));
    if (version != PLD_CONFIG_VERSION)
        goto error;

    PLD_READ_CHECK(config_file, &context->config->fullscreen, sizeof(bool));
    PLD_READ_CHECK(config_file, &context->config->vsync, sizeof(bool));
    PLD_READ_CHECK(config_file, &context->config->autoplay, sizeof(bool));
    PLD_READ_CHECK(config_file, &context->config->fps, sizeof(Sint32));
    PLD_READ_CHECK(config_file, &context->config->offset, sizeof(Sint32));
    PLD_READ_CHECK(config_file, &context->config->load_song_menu_media, sizeof(bool));
    PLD_READ_CHECK(config_file, &context->config->image_width, sizeof(Sint32));
    PLD_READ_CHECK(config_file, &context->config->image_height, sizeof(Sint32));
    PLD_READ_CHECK(config_file, &context->config->music_volume, sizeof(float));
    PLD_READ_CHECK(config_file, &context->config->sound_effect_volume, sizeof(float));
    PLD_READ_CHECK(config_file, &context->config->enable_video, sizeof(bool));
    PLD_READ_CHECK(config_file, &context->config->button_for_slider, sizeof(bool));

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        context->config->gameplay_keys[i] = PLD_CreateArrayList();
        Sint32 flag = !PLD_CONFIG_STOP_FLAG;

        while (flag != PLD_CONFIG_STOP_FLAG)
        {
            PLD_READ_CHECK(config_file, &flag, sizeof(Sint32));

            if (flag != PLD_CONFIG_STOP_FLAG)
            {
                Sint32* scancode = SDL_malloc(sizeof(Sint32));
                *scancode = flag;
                PLD_ArrayListAdd(context->config->gameplay_keys[i], scancode);
            }
        }
    }

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        context->config->gameplay_buttons[i] = PLD_CreateArrayList();
        Sint32 flag = !PLD_CONFIG_STOP_FLAG;

        while (flag != PLD_CONFIG_STOP_FLAG)
        {
            PLD_READ_CHECK(config_file, &flag, sizeof(Sint32));

            if (flag != PLD_CONFIG_STOP_FLAG)
            {
                Sint32* button = SDL_malloc(sizeof(Sint32));
                *button = flag;
                PLD_ArrayListAdd(context->config->gameplay_buttons[i], button);
            }
        }
    }

    for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
    {
        PLD_READ_CHECK(config_file, &context->config->menu_keys[i], sizeof(Sint32));
    }

    for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
    {
        PLD_READ_CHECK(config_file, &context->config->menu_buttons[i], sizeof(Sint32));
    }

    PLD_READ_CHECK(config_file, &context->config->trigger_deadzone, sizeof(Sint32));
    PLD_READ_CHECK(config_file, &context->config->joystick_deadzone, sizeof(Sint32));

    context->config->touch_buttons = PLD_CreateArrayList();
    Sint32 flag = !PLD_CONFIG_STOP_FLAG;
    while (flag != PLD_CONFIG_STOP_FLAG)
    {
        PLD_READ_CHECK(config_file, &flag, sizeof(Sint32));
        if (flag != PLD_CONFIG_STOP_FLAG)
        {
            PLD_TouchButton* touch_button = SDL_malloc(sizeof(PLD_TouchButton));
            touch_button->button = flag;
            PLD_READ_CHECK(config_file, &touch_button->rect, sizeof(float) * 4);
            PLD_ArrayListAdd(context->config->touch_buttons, touch_button);
        }
    }

    SDL_CloseIO(config_file);
    SDL_free(config_path);

    return true;

error:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());

    if (config_file != NULL)
    {
        SDL_CloseIO(config_file);
    }

    if (config_path != NULL)
    {
        SDL_free(config_path);
    }

    if (context->config != NULL)
    {
        PLD_DestroyConfig(context->config);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Defaulting config");
    PLD_ShowMessage("Old / corrupted config. Your config file be regenerated now and your settings will be reset. Close the game now to prevent this");

    context->config = PLD_CreateDefaultConfig();
    PLD_WriteConfig(context);

    return false;
}


void PLD_QuitSDL(PLD_Context* context)
{
    if (context != NULL)
    {
        if (context->renderer != NULL)
        {
            SDL_DestroyRenderer(context->renderer);
            context->renderer = NULL;
        }

        if (context->window != NULL)
        {
            SDL_DestroyWindow(context->window);
            context->window = NULL;
        }
    }

    TTF_Quit();
    SDL_Quit();
}


bool PLD_InitSDL(PLD_Context* context)
{
    if (!SDL_Init(PLD_SDL_INIT_FLAGS))
    {
        goto error;
    }

    if (!SDL_CreateWindowAndRenderer(PLD_WINDOW_NAME, PLD_window_width, PLD_window_height, PLD_WINDOW_FLAGS, &context->window, &context->renderer))
    {
        goto error;
    }

    if (!SDL_SetRenderDrawBlendMode(context->renderer, SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    }

    if (!TTF_Init())
    {
        goto error;
    }

    SDL_srand(0);

    return true;

error:
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());

    PLD_QuitSDL(context);

    return false;
}


bool PLD_WriteConfig(PLD_Context* context)
{
    Sint32 stop_flag = PLD_CONFIG_STOP_FLAG;
    char* config_path = NULL;
    SDL_IOStream* config_file = NULL;

    SDL_asprintf(&config_path, "%s%s", context->data_path, PLD_CONFIG_NAME);
    config_file = SDL_IOFromFile(config_path, "wb");
    if (config_file == NULL)
    {
        goto error;
    }

    PLD_WRITE_CHECK(config_file, &PLD_CONFIG_VERSION, sizeof(Uint32));

    PLD_WRITE_CHECK(config_file, &context->config->fullscreen, sizeof(bool));
    PLD_WRITE_CHECK(config_file, &context->config->vsync, sizeof(bool));
    PLD_WRITE_CHECK(config_file, &context->config->autoplay, sizeof(bool));
    PLD_WRITE_CHECK(config_file, &context->config->fps, sizeof(Sint32));
    PLD_WRITE_CHECK(config_file, &context->config->offset, sizeof(Sint32));
    PLD_WRITE_CHECK(config_file, &context->config->load_song_menu_media, sizeof(bool));
    PLD_WRITE_CHECK(config_file, &context->config->image_width, sizeof(Sint32));
    PLD_WRITE_CHECK(config_file, &context->config->image_height, sizeof(Sint32));
    PLD_WRITE_CHECK(config_file, &context->config->music_volume, sizeof(float));
    PLD_WRITE_CHECK(config_file, &context->config->sound_effect_volume, sizeof(float));
    PLD_WRITE_CHECK(config_file, &context->config->enable_video, sizeof(bool));
    PLD_WRITE_CHECK(config_file, &context->config->button_for_slider, sizeof(bool));

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        for (int j = 0; j < context->config->gameplay_keys[i]->len; j++)
        {
            PLD_WRITE_CHECK(config_file, context->config->gameplay_keys[i]->data[j], sizeof(Sint32));
        }
        PLD_WRITE_CHECK(config_file, &stop_flag, sizeof(Sint32));
    }

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        for (int j = 0; j < context->config->gameplay_buttons[i]->len; j++)
        {
            PLD_WRITE_CHECK(config_file, context->config->gameplay_buttons[i]->data[j], sizeof(Sint32));
        }
        PLD_WRITE_CHECK(config_file, &stop_flag, sizeof(Sint32));
    }

    for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
    {
        PLD_WRITE_CHECK(config_file, &context->config->menu_keys[i], sizeof(Sint32));
    }

    for (int i = 0; i < PLD_MENU_INPUT_COUNT; i++)
    {
        PLD_WRITE_CHECK(config_file, &context->config->menu_buttons[i], sizeof(Sint32));
    }

    PLD_WRITE_CHECK(config_file, &context->config->trigger_deadzone, sizeof(Sint32));
    PLD_WRITE_CHECK(config_file, &context->config->joystick_deadzone, sizeof(Sint32));

    for (int i = 0; i < context->config->touch_buttons->len; i++)
    {
        PLD_TouchButton* touch_button = context->config->touch_buttons->data[i];
        PLD_WRITE_CHECK(config_file, &touch_button->button, sizeof(Sint32));
        PLD_WRITE_CHECK(config_file, &touch_button->rect, sizeof(float) * 4);
    }
    PLD_WRITE_CHECK(config_file, &stop_flag, sizeof(Sint32));

    SDL_CloseIO(config_file);
    SDL_free(config_path);

    return true;

error:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to write config : %s", SDL_GetError());

    if (config_file != NULL)
    {
        SDL_CloseIO(config_file);
    }

    if (config_path != NULL)
    {
        SDL_free(config_path);
    }

    return false;
}

bool PLD_ApplyConfig(PLD_Context* context)
{
    bool success = true;

    char* fps;
    SDL_asprintf(&fps, "%d", context->config->fps);
//#ifndef SDL_PLATFORM_EMSCRIPTEN
    if (!context->config->vsync)
    {
        SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, fps);
    }
    else
    {
        SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "0");
    }
//#endif
    SDL_free(fps);

    if (!SDL_SetWindowFullscreen(context->window, context->config->fullscreen))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        success = false;
    }

    if (!SDL_SetRenderVSync(context->renderer, context->config->vsync))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        success = false;
    }
    
    PLD_SetSoundEffectVolume(context->config->sound_effect_volume);
    PLD_SetMusicVolume(context->config->music_volume);

    return success;
}

SDL_FPoint PLD_GetLogicalCenter()
{
    return (SDL_FPoint){PLD_LOGICAL_WIDTH / 2.0f, PLD_LOGICAL_HEIGHT / 2.0f};
}

void PLD_Quit(PLD_Context* context)
{
    if (context != NULL)
    {
        PLD_QuitMusic();
        PLD_CloseAudio(context);
        PLD_QuitSDL(context);
        
        if (context->config != NULL)
        {
            PLD_DestroyConfig(context->config);
        }

        if (context->data_path != NULL)
        {
            SDL_free(context->data_path);
        }

        SDL_free(context);
    }
}

bool PLD_CopyAndroidData(PLD_Context* context)
{
    return PLD_CopyDirectory("assets://data", context->data_path, false);
}

PLD_Context* PLD_Init()
{
    PLD_Context* context = SDL_calloc(1, sizeof(PLD_Context));

    SDL_SetLogOutputFunction(PLD_Log, NULL);

    if (!PLD_GetDataPath(context))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "Failed to get a readable/writable path for data", NULL);
        goto error;
    }
    
    PLD_OpenLog(context);
    PLD_ReadConfig(context);

    if (!PLD_InitSDL(context))
    {
        goto error;
    }

    if (!PLD_OpenAudio(context)) { goto error; }
    if (!PLD_InitMusic(context)) { goto error; }

#ifdef SDL_PLATFORM_ANDROID
    PLD_CopyAndroidData(context);
    PLD_ReadConfig(context);
#endif

    PLD_ApplyConfig(context);

    return context;

error:
    if (context != NULL)
    {
        PLD_Quit(context);
    }

    return NULL;
}

bool PLD_ClearWindow(PLD_Context* context)
{
    SDL_GetRenderOutputSize(context->renderer, &PLD_window_width, &PLD_window_height);
    PLD_window_scale_x = (float)PLD_window_width  / (float)PLD_LOGICAL_WIDTH;
    PLD_window_scale_y = (float)PLD_window_height / (float)PLD_LOGICAL_HEIGHT;

    PLD_window_scale = (PLD_window_scale_x < PLD_window_scale_y) ? PLD_window_scale_x : PLD_window_scale_y;

    PLD_window_scaled_width  = SDL_roundf(PLD_LOGICAL_WIDTH  * PLD_window_scale);
    PLD_window_scaled_height = SDL_roundf(PLD_LOGICAL_HEIGHT * PLD_window_scale);

    PLD_window_offset_x = SDL_roundf((PLD_window_width  - PLD_window_scaled_width)  * 0.5f);
    PLD_window_offset_y = SDL_roundf((PLD_window_height - PLD_window_scaled_height) * 0.5f);

    SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 255);
    return SDL_RenderClear(context->renderer);
}

bool PLD_PresentWindow(PLD_Context* context)
{
    SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 255);

    SDL_FRect rect = {0.0f, 0.0f, PLD_window_width, PLD_window_offset_y};
    SDL_RenderFillRect(context->renderer, &rect);

    rect.y = PLD_window_scaled_height + PLD_window_offset_y;
    SDL_RenderFillRect(context->renderer, &rect);

    rect = (SDL_FRect){0.0f, 0.0f, PLD_window_offset_x, PLD_window_height};
    SDL_RenderFillRect(context->renderer, &rect);

    rect.x = PLD_window_scaled_width + PLD_window_offset_x;
    SDL_RenderFillRect(context->renderer, &rect);

    return SDL_RenderPresent(context->renderer);
}

bool PLD_DrawRectangle(PLD_Context* context, SDL_FRect* rect, int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(context->renderer, r, g, b, a);

    if (!rect)
    {
        return SDL_RenderFillRect(context->renderer, rect);
    }
    SDL_FRect copy = *rect;
    PLD_ScaleRect(&copy);
    return SDL_RenderFillRect(context->renderer, &copy);
}

bool PLD_ScalePoint(SDL_FPoint* point)
{
    if (!point)
        return false;

    point->x = point->x * PLD_window_scale + PLD_window_offset_x;
    point->y = point->y * PLD_window_scale + PLD_window_offset_y;

    // floating point does weird things
    point->x = (int)point->x;
    point->y = (int)point->y;

    return true;
}

bool PLD_ScalePointBack(SDL_FPoint* point)
{
    if (!point)
        return false;

    point->x = (point->x - PLD_window_offset_x) / PLD_window_scale;
    point->y = (point->y - PLD_window_offset_y) / PLD_window_scale;

    point->x = (int)point->x;
    point->y = (int)point->y;

    return true;
}

bool PLD_ScaleCenterPoint(SDL_FPoint* point)
{
    if (!point) return false;

    point->x *= PLD_window_scale;
    point->y *= PLD_window_scale;

    return true;
}

bool PLD_ScaleRect(SDL_FRect* rect)
{
    if (!rect)
        return false;

    rect->x = rect->x * PLD_window_scale + PLD_window_offset_x;
    rect->y = rect->y * PLD_window_scale + PLD_window_offset_y;

    rect->w *= PLD_window_scale;
    rect->h *= PLD_window_scale;

    // floating point does weird things
    rect->x = (int)rect->x;
    rect->y = (int)rect->y;
    rect->w = (int)rect->w;
    rect->h = (int)rect->h;

    return true;
}

bool PLD_DrawBorder(PLD_Context* context, const SDL_FRect* rect, const SDL_Color* color, float thickness)
{
    SDL_FRect new_rect = {rect->x - thickness,
                          rect->y - thickness,
                          rect->w + thickness * 2,
                          thickness};

    PLD_DrawRectangle(context, &new_rect, color->r, color->g, color->b, color->a);
    new_rect.y = rect->y + rect->h;
    PLD_DrawRectangle(context, &new_rect, color->r, color->g, color->b, color->a);

    new_rect = (SDL_FRect){rect->x - thickness,
                           rect->y - thickness,
                           thickness,
                           rect->h + thickness * 2};

    PLD_DrawRectangle(context, &new_rect, color->r, color->g, color->b, color->a);
    new_rect.x = rect->x + rect->w;
    PLD_DrawRectangle(context, &new_rect, color->r, color->g, color->b, color->a);

    return true;
}

bool PLD_SetClipRect(PLD_Context* context, const SDL_FRect* rect)
{
    if (rect == NULL)
        return SDL_SetRenderClipRect(context->renderer, NULL);

    SDL_FRect copy = *rect;
    PLD_ScaleRect(&copy);
    SDL_Rect final = {copy.x, copy.y, copy.w, copy.h};

    return SDL_SetRenderClipRect(context->renderer, &final);
}

bool PLD_ChangeDataPath(PLD_Context* context, const char* path)
{
    char* temp_font_path;
    SDL_asprintf(&temp_font_path, "%s/font.otf", path);
    if (!SDL_GetPathInfo(temp_font_path, NULL))
    {
        SDL_free(temp_font_path);
        return false;
    }
    SDL_free(temp_font_path);
    
    PLD_QuitTouch();
    PLD_QuitUserData();
    PLD_DestroyImages(PLD_IMAGE_CATEGORY_MENU);
    PLD_DestroySounds();
    PLD_DestroyEffects();
    PLD_CloseFont(context);
    PLD_DestroyConfig(context->config);
    
    SDL_free(context->data_path);
    SDL_asprintf(&context->data_path, "%s/", path);
    
    PLD_OpenLog(context);
    PLD_ReadConfig(context);
    PLD_ApplyConfig(context);
    
    PLD_OpenFont(context);
    PLD_LoadEffects(context);
    PLD_LoadSounds(context);
    PLD_LoadImages(context, PLD_IMAGE_CATEGORY_MENU);
    PLD_InitUserData(context);
    PLD_InitTouch(context);

    return true;
}

void PLD_ShowMessage(const char* message)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "MESSAGE", message, NULL);
}
