#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <PLD/PLD_array_list.h>
#include <PLD/PLD_utils.h>


#define PLD_LOGICAL_WIDTH 1920
#define PLD_LOGICAL_HEIGHT 1080


typedef enum PLD_GameplayInput
{
    PLD_GAMEPLAY_INPUT_INVALID = -1,
    PLD_GAMEPLAY_INPUT_NORTH,
    PLD_GAMEPLAY_INPUT_WEST,
    PLD_GAMEPLAY_INPUT_SOUTH,
    PLD_GAMEPLAY_INPUT_EAST,
    PLD_GAMEPLAY_INPUT_L,
    PLD_GAMEPLAY_INPUT_R,
    PLD_GAMEPLAY_INPUT_COUNT
}PLD_GameplayInput;


typedef enum PLD_MenuInput
{
    PLD_MENU_INPUT_INVALID = -1,
    PLD_MENU_INPUT_UP,
    PLD_MENU_INPUT_LEFT,
    PLD_MENU_INPUT_DOWN,
    PLD_MENU_INPUT_RIGHT,
    PLD_MENU_INPUT_NORTH,
    PLD_MENU_INPUT_WEST,
    PLD_MENU_INPUT_SOUTH,
    PLD_MENU_INPUT_EAST,
    PLD_MENU_INPUT_L,
    PLD_MENU_INPUT_R,
    PLD_MENU_INPUT_BACK,
    PLD_MENU_INPUT_START,
    PLD_MENU_INPUT_COUNT
}PLD_MenuInput;


typedef enum PLD_Button
{
    PLD_BUTTON_INVALID = -1,
    PLD_BUTTON_SOUTH,
    PLD_BUTTON_EAST,
    PLD_BUTTON_WEST,
    PLD_BUTTON_NORTH,
    PLD_BUTTON_BACK,
    PLD_BUTTON_GUIDE,
    PLD_BUTTON_START,
    PLD_BUTTON_LEFT_STICK,
    PLD_BUTTON_RIGHT_STICK,
    PLD_BUTTON_LEFT_SHOULDER,
    PLD_BUTTON_RIGHT_SHOULDER,
    PLD_BUTTON_DPAD_UP,
    PLD_BUTTON_DPAD_DOWN,
    PLD_BUTTON_DPAD_LEFT,
    PLD_BUTTON_DPAD_RIGHT,
    PLD_BUTTON_MISC1,
    PLD_BUTTON_RIGHT_PADDLE1,
    PLD_BUTTON_LEFT_PADDLE1,
    PLD_BUTTON_RIGHT_PADDLE2,
    PLD_BUTTON_LEFT_PADDLE2,
    PLD_BUTTON_TOUCHPAD,
    PLD_BUTTON_MISC2,
    PLD_BUTTON_MISC3,
    PLD_BUTTON_MISC4,
    PLD_BUTTON_MISC5,
    PLD_BUTTON_MISC6,
    PLD_BUTTON_LEFT_STICK_UP,
    PLD_BUTTON_LEFT_STICK_DOWN,
    PLD_BUTTON_LEFT_STICK_LEFT,
    PLD_BUTTON_LEFT_STICK_RIGHT,
    PLD_BUTTON_RIGHT_STICK_UP,
    PLD_BUTTON_RIGHT_STICK_DOWN,
    PLD_BUTTON_RIGHT_STICK_LEFT,
    PLD_BUTTON_RIGHT_STICK_RIGHT,
    PLD_BUTTON_LEFT_TRIGGER,
    PLD_BUTTON_RIGHT_TRIGGER,
    PLD_BUTTON_COUNT
}PLD_Button;

typedef struct PLD_TouchButton
{
    SDL_FRect rect;
    PLD_Button button;
}PLD_TouchButton;

typedef struct PLD_Config
{
    bool fullscreen;
    bool vsync;
    bool autoplay;
    Sint32 fps;
    Sint32 offset;
    bool load_song_menu_media;
    Sint32 image_width;
    Sint32 image_height;
    float music_volume;
    float sound_effect_volume;
    
    PLD_ArrayList* gameplay_keys[PLD_GAMEPLAY_INPUT_COUNT];
    PLD_ArrayList* gameplay_buttons[PLD_GAMEPLAY_INPUT_COUNT];
    Sint32 menu_keys[PLD_MENU_INPUT_COUNT];
    Sint32 menu_buttons[PLD_MENU_INPUT_COUNT];
    PLD_ArrayList* touch_buttons;

    Sint32 trigger_deadzone;
    Sint32 joystick_deadzone;
}PLD_Config;


typedef struct PLD_Context
{
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Gamepad* gamepad;
    TTF_Font* font;
    TTF_TextEngine* text_engine;

    char* data_path;

    PLD_Config* config;

    bool key_states[SDL_SCANCODE_COUNT];
    bool button_states[PLD_BUTTON_COUNT];
}PLD_Context;

SDL_FPoint PLD_GetLogicalCenter();

PLD_Context* PLD_Init();

void PLD_Quit(PLD_Context* context);

bool PLD_WriteConfig(PLD_Context* context);

bool PLD_ApplyConfig(PLD_Context* context);

bool PLD_ClearWindow(PLD_Context* context);

bool PLD_PresentWindow(PLD_Context* context);

bool PLD_DrawRectangle(PLD_Context* context, SDL_FRect* rect, int r, int g, int b, int a);
