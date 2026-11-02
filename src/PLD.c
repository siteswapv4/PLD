#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <PLD/PLD.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_start_menu.h>
#include <PLD/PLD_core.h>
#include <PLD/PLD_control.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_effect.h>
#include <PLD/PLD_song_menu.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_pause.h>
#include <PLD/PLD_result.h>
#include <PLD/PLD_sound.h>
#include <PLD/PLD_music.h>
#include <PLD/PLD_touch.h>
#include <PLD/PLD_user.h>
#include <PLD/PLD_touch_config.h>

typedef enum PLD_AppPhase
{
    PLD_APP_PHASE_START_MENU,
    PLD_APP_PHASE_TOUCH_CONFIG,
    PLD_APP_PHASE_SONG_MENU,
    PLD_APP_PHASE_GAMEPLAY,
    PLD_APP_PHASE_RESULT,
    PLD_APP_PHASE_COUNT
}PLD_AppPhase;

typedef struct PLD_AppState
{
    PLD_Context* context;
    PLD_StartMenuState* start_menu;
    PLD_SongMenu* song_menu;
    PLD_Gameplay* gameplay;
    PLD_Result* result;
    PLD_AppPhase phase;
    PLD_TouchConfigState* touch_config;
}PLD_AppState;

const char* PLD_DIFFICULTY_STRING[] = {"Easy", "Normal", "Hard", "Extreme", "ExtraExtreme", "Base"};

int PLD_EventLoop(PLD_Context* context, SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_EVENT_KEY_DOWN:
            if (event->key.repeat == 0)
            {
            	context->key_states[event->key.scancode] = true;
            	
                switch (event->key.scancode)
                {
                    case SDL_SCANCODE_F11:
                        if (!SDL_SetWindowFullscreen(context->window, !(SDL_GetWindowFlags(context->window) & SDL_WINDOW_FULLSCREEN)))
                        {
                            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
                        }
                        break;
                    
                    default:
                        break;
                }
            }
            break;
            
        case SDL_EVENT_KEY_UP:
			context->key_states[event->key.scancode] = false;	
        	break;
        	
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        	context->button_states[event->gbutton.button] = true;
        	break;
        	
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        	context->button_states[event->gbutton.button] = false;
        	break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            if ((context->gamepad != NULL) && (!SDL_GamepadConnected(context->gamepad)))
            {
                SDL_CloseGamepad(context->gamepad);
                context->gamepad = NULL;
            }
            break;

        case SDL_EVENT_GAMEPAD_ADDED:
        	if (!context->gamepad)
        	{
        		 context->gamepad = SDL_OpenGamepad(event->gdevice.which);
        	}
            break;
    }

    return 0;
}

SDL_AppResult SDL_AppInit(void** user_data, int argc, char* argv[])
{
    PLD_AppState* app = SDL_calloc(1, sizeof(PLD_AppState));
    *user_data = app;

    if ((app->context = PLD_Init()) == NULL)
    {
        goto error;
    }

    if (!PLD_LoadEffects(app->context))
    {
        goto error;
    }

    if (!PLD_LoadSounds(app->context))
    {
        goto error;
    }
    
    if (!PLD_OpenFont(app->context))
    {
        goto error;
    }

    app->start_menu = PLD_LoadStartMenu(app->context, true);
    if (app->start_menu == NULL)
    {
        goto error;
    }

    app->phase = PLD_APP_PHASE_START_MENU;

    PLD_InitTouch(app->context);
    PLD_LoadImages(app->context, PLD_IMAGE_CATEGORY_MENU);

    if (!PLD_InitUserData(app->context)) goto error;

    return SDL_APP_CONTINUE;

error:
    return SDL_APP_FAILURE;
}

void PLD_ReturnToSongMenu(PLD_AppState* app)
{
    app->phase = PLD_APP_PHASE_SONG_MENU;
    app->song_menu->state = PLD_SONGMENU_STATE_SONG;
    PLD_QuitGameplay(app->context, app->gameplay);
    if (!PLD_SaveUserData(app->song_menu->chartPaths[app->song_menu->currentDifficulty], &app->song_menu->new_user_data))
        SDL_Log("Failed to save user data");

    app->gameplay = NULL;

    PLD_PlayMusic(app->context, app->song_menu->music);
    PLD_SetMusicPosition(app->song_menu->music, app->song_menu->dataIni->thumbTimeStart);
}

void PLD_SwitchToSongMenu(PLD_AppState* app)
{
    PLD_QuitStartMenu(app->start_menu);
    app->start_menu = NULL;
    if (!app->song_menu)
    {
        app->song_menu = PLD_LoadSongMenu(app->context);
    }
    app->phase = PLD_APP_PHASE_SONG_MENU;
}

void PLD_SwitchToTouchConfig(PLD_AppState* app)
{
    app->touch_config = PLD_InitTouchConfig(app->context);
    app->phase = PLD_APP_PHASE_TOUCH_CONFIG;
}

void PLD_SwitchToStartMenuFromTouchConfig(PLD_AppState* app)
{
    PLD_QuitTouchConfig(app->context, app->touch_config);
    app->phase = PLD_APP_PHASE_START_MENU;
}

SDL_AppResult SDL_AppEvent(void* user_data, SDL_Event* event)
{
    PLD_AppState* app = user_data;

    SDL_ConvertEventToRenderCoordinates(app->context->renderer, event);
    PLD_EventLoop(app->context, event);

    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    
    if (app->phase != PLD_APP_PHASE_TOUCH_CONFIG)
    {
        if ((event->type == SDL_EVENT_FINGER_DOWN) || (event->type == SDL_EVENT_FINGER_UP) || (event->type == SDL_EVENT_FINGER_MOTION) || (event->type == SDL_EVENT_FINGER_CANCELED))
            PLD_TouchEvent(app->context, event);
    }

    switch (app->phase)
    {
        case PLD_APP_PHASE_START_MENU:
            switch (PLD_StartMenuEvent(app->context, app->start_menu, event))
            {
                case PLD_START_MENU_NEXT:
                    PLD_SwitchToSongMenu(app);
                    break;

                case PLD_START_MENU_PREVIOUS:
                    return SDL_APP_SUCCESS;

                case PLD_START_MENU_FAILURE:
                    return SDL_APP_FAILURE;

                case PLD_START_MENU_TOUCH_CONFIG:
                    PLD_SwitchToTouchConfig(app);
                    break;

                default:
                    break;
            }
            break;

        case PLD_APP_PHASE_SONG_MENU:
            if (!PLD_SongMenuKeyPress(app->context, app->song_menu, event))
            {
                app->start_menu = PLD_LoadStartMenu(app->context, false);
                app->phase = PLD_APP_PHASE_START_MENU;
            }
            break;

        case PLD_APP_PHASE_GAMEPLAY:
            if (app->gameplay->pause)
            {
                if (PLD_PauseEvent(app->context, &app->gameplay->pause, &app->gameplay->offset, app->gameplay->pauseMenu, app->gameplay->musicPlaying, event) != PLD_PAUSE_CONTINUE)
                {
                    PLD_ReturnToSongMenu(app);
                }
                else
                {
                    if (app->gameplay->pauseMenu->retry)
                    {
                        PLD_RetryGameplay(app->context, app->gameplay);
                    }
                }
            }
            else
            {
                PLD_GameplayEvent(app->context, app->gameplay, event);
            }
            break;

        case PLD_APP_PHASE_RESULT:
            if (PLD_ResultEvent(app->context, app->result, event) != PLD_RESULT_CONTINUE)
            {
                PLD_QuitResult(app->result);
                app->result = NULL;
                PLD_ReturnToSongMenu(app);
            }
            break;

        case PLD_APP_PHASE_TOUCH_CONFIG:
            switch (PLD_TouchConfigEvent(app->context, app->touch_config, event))
            {
                case PLD_TOUCH_CONFIG_SUCCESS:
                    PLD_SwitchToStartMenuFromTouchConfig(app);
                    break;

                case PLD_TOUCH_CONFIG_FAILURE:
                    return SDL_APP_FAILURE;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* user_data)
{
    PLD_AppState* app = user_data;

    PLD_ClearWindow(app->context);

    PLD_UpdateMusic();

    switch (app->phase)
    {
        case PLD_APP_PHASE_START_MENU:
            switch (PLD_StartMenuIterate(app->context, app->start_menu))
            {
                case PLD_START_MENU_NEXT:
                    PLD_SwitchToSongMenu(app);
                    break;

                case PLD_START_MENU_PREVIOUS:
                    return SDL_APP_SUCCESS;

                case PLD_START_MENU_FAILURE:
                    return SDL_APP_FAILURE;
                
                default:
                    break;
            }
            break;

        case PLD_APP_PHASE_SONG_MENU:
            PLD_SongMenuLoop(app->context, app->song_menu);

            if (app->song_menu->state == PLD_SONGMENU_STATE_CORE)
            {
                app->gameplay = PLD_InitGameplay(app->context, &app->song_menu->new_user_data, app->song_menu->currentSongPath, app->song_menu->chartPaths[app->song_menu->currentDifficulty], app->song_menu->currentDifficulty, app->context->config->autoplay, app->song_menu->background_image, app->song_menu->texts[PLD_SONG_LIST_MAX / 2], app->song_menu->dataIni, app->song_menu->music, app->song_menu->video);
                app->phase = PLD_APP_PHASE_GAMEPLAY;
            }
            break;

        case PLD_APP_PHASE_GAMEPLAY:
            if (PLD_GameplayLoop(app->context, app->gameplay) == PLD_GAMEPLAY_SUCCESS)
            {
                PLD_StopMusic(app->song_menu->music);
                app->result = PLD_InitResult(app->context, app->song_menu->texts[PLD_SONG_LIST_MAX / 2], &app->song_menu->new_user_data, app->song_menu->new_user_data.score > app->song_menu->user_datas[app->song_menu->currentDifficulty].score);
                app->phase = PLD_APP_PHASE_RESULT;
            }
            break;

        case PLD_APP_PHASE_RESULT:
            PLD_ResultLoop(app->context, app->result);
            break;

        case PLD_APP_PHASE_TOUCH_CONFIG:
            PLD_TouchConfigIterate(app->context, app->touch_config);
            break;

        default:
            break;
    }

    if (app->phase != PLD_APP_PHASE_TOUCH_CONFIG)
        PLD_RenderTouch(app->context, false);
    
    PLD_PresentWindow(app->context);

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void* user_data, SDL_AppResult result)
{
    PLD_AppState* app = user_data;

    if (result == SDL_APP_FAILURE)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "Check PLD.log for details", NULL);
    }

    if (app->gameplay != NULL)
    {
        PLD_QuitGameplay(app->context, app->gameplay);
    }

    if (app->result != NULL)
    {
        PLD_QuitResult(app->result);
    }

    if (app->song_menu != NULL)
    {
        PLD_QuitSongMenu(app->context, app->song_menu);
    }

    PLD_DestroyImages(PLD_IMAGE_CATEGORY_MENU);

    if (app->start_menu) { PLD_QuitStartMenu(app->start_menu); }

    PLD_CloseFont(app->context);
    PLD_DestroySounds();
    PLD_DestroyEffects();

    PLD_QuitUserData();
    
    PLD_QuitTouch();

    PLD_Quit(app->context);
    SDL_free(app);
}
