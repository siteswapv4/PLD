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

typedef enum PLD_AppPhase
{
    PLD_APP_PHASE_START_MENU,
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
    
    if (!PLD_OpenAudio()) { goto error; }

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

    return SDL_APP_CONTINUE;

error:
    return SDL_APP_FAILURE;
}

void PLD_ReturnToSongMenu(PLD_AppState* app)
{
    app->phase = PLD_APP_PHASE_SONG_MENU;
    app->song_menu->state = PLD_SONGMENU_STATE_SONG;
    PLD_QuitGameplay(app->context, app->gameplay);
    app->gameplay = NULL;

    PLD_PlayMusic(app->song_menu->music);
    PLD_SetMusicPosition(app->song_menu->music, app->song_menu->dataIni->thumbTimeStart);
}

#define PLD_SHOW_TOUCH_DELAY 10000
static Uint64 PLD_last_touch = 0;
static bool PLD_screen_touched = false;
typedef struct PLD_HeldTouch
{
    SDL_FingerID id;
    PLD_Button button;
}PLD_HeldTouch;
static PLD_ArrayList* PLD_held_touches = NULL;
void PLD_TouchEvent(PLD_Context* context, SDL_Event* event)
{
    PLD_screen_touched = true;
    PLD_last_touch = SDL_GetTicks();
    if (!PLD_held_touches) { PLD_held_touches = PLD_CreateArrayList(); }
    if (event->type == SDL_EVENT_FINGER_DOWN)
    {
        for (int i = 0; i < context->config->touch_buttons->len; i++)
        {
            PLD_TouchButton* touch_button = context->config->touch_buttons->data[i];
            SDL_FPoint finger_position = {event->tfinger.x, event->tfinger.y};
            if (SDL_PointInRectFloat(&finger_position, &touch_button->rect))
            {
                PLD_HeldTouch* held_touch = SDL_malloc(sizeof(PLD_HeldTouch));
                held_touch->id = event->tfinger.fingerID;
                held_touch->button = touch_button->button;
                PLD_ArrayListAdd(PLD_held_touches, held_touch);

                SDL_Event new_event = {0};
                new_event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                new_event.gbutton.button = touch_button->button;
                SDL_PushEvent(&new_event);
            }
        }
    }
    else if (event->type == SDL_EVENT_FINGER_UP)
    {
        for (int i = PLD_held_touches->len - 1; i >= 0; i--)
        {
            PLD_HeldTouch* held_touch = PLD_held_touches->data[i];
            if (held_touch->id == event->tfinger.fingerID)
            {
                SDL_Event new_event = {0};
                new_event.type = SDL_EVENT_GAMEPAD_BUTTON_UP;
                new_event.gbutton.button = held_touch->button;
                SDL_PushEvent(&new_event);
                SDL_free(PLD_ArrayListRemoveAt(PLD_held_touches, i));
            }
        }
    }
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

SDL_AppResult SDL_AppEvent(void* user_data, SDL_Event* event)
{
    PLD_AppState* app = user_data;

    SDL_ConvertEventToRenderCoordinates(app->context->renderer, event);
        	
    PLD_EventLoop(app->context, event);

    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    
    if ((event->type == SDL_EVENT_FINGER_DOWN) || (event->type == SDL_EVENT_FINGER_UP)) { PLD_TouchEvent(app->context, event); }
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

        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

void PLD_RenderTouch(PLD_Context* context)
{
    if ((PLD_screen_touched) && (SDL_GetTicks() - PLD_last_touch < PLD_SHOW_TOUCH_DELAY))
    {
        for (int i = 0; i < context->config->touch_buttons->len; i++)
        {
            SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 100);
            SDL_RenderFillRect(context->renderer, &((PLD_TouchButton*)context->config->touch_buttons->data[i])->rect);
        }
    }
}

SDL_AppResult SDL_AppIterate(void* user_data)
{
    PLD_AppState* app = user_data;

    PLD_ClearWindow(app->context);

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
                app->gameplay = PLD_InitGameplay(app->context, app->song_menu->currentSongPath, app->song_menu->chartPaths[app->song_menu->currentDifficulty], app->song_menu->currentDifficulty, app->context->config->autoplay, app->song_menu->background_image, app->song_menu->texts[PLD_SONG_LIST_MAX / 2], app->song_menu->dataIni, app->song_menu->music, app->song_menu->video);
                app->phase = PLD_APP_PHASE_GAMEPLAY;
            }
            break;

        case PLD_APP_PHASE_GAMEPLAY:
            if (PLD_GameplayLoop(app->context, app->gameplay) == PLD_GAMEPLAY_SUCCESS)
            {
                PLD_StopMusic(app->song_menu->music);
                app->result = PLD_InitResult(app->context, app->song_menu->backgroundPath, app->song_menu->texts[PLD_SONG_LIST_MAX / 2]);
                app->phase = PLD_APP_PHASE_RESULT;
            }
            break;

        case PLD_APP_PHASE_RESULT:
            PLD_ResultLoop(app->context, app->result);
            break;

        default:
            break;
    }

    PLD_RenderTouch(app->context);
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

    if (app->start_menu) { PLD_QuitStartMenu(app->start_menu); }

    PLD_CloseFont(app->context);
    PLD_DestroySounds();
    PLD_DestroyEffects();
    
    PLD_Quit(app->context);
    SDL_free(app);
}
