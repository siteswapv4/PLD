#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include "PLD.h"
#include "PLD_context.h"
#include "PLDCore.h"
#include "PLDControl.h"
#include "PLD_image.h"
#include "PLD_effect.h"
#include "PLDSongMenu.h"
#include "PLDUtils.h"
#include "PLDPause.h"
#include "PLDResult.h"


typedef enum PLD_State
{
    PLD_STATE_INVALID = -1,
    PLD_STATE_SONG_MENU,
    PLD_STATE_GAMEPLAY,
    PLD_STATE_RESULT,
    PLD_STATE_COUNT
}PLD_State;


typedef struct PLD_AppState
{
    PLD_Context* context;
    PLD_SongMenu* song_menu;
    PLD_Gameplay* gameplay;
    PLD_Result* result;
    PLD_State state;
}PLD_AppState;


const char* PLD_DIFFICULTY_STRING[] = {"Easy", "Normal", "Hard", "Extreme", "ExtraExtreme", "Base"};


SDL_AppResult SDL_AppInit(void** user_data, int argc, char* argv[])
{
    PLD_AppState* app_state = SDL_calloc(1, sizeof(PLD_AppState));
    *user_data = app_state;

    if ((app_state->context = PLD_Init()) == NULL)
    {
        goto error;
    }

    if (!PLD_LoadEffects(app_state->context))
    {
        goto error;
    }
    
    if (!PLD_OpenAudio()) { goto error; }

    if (!PLD_LoadSounds(app_state->context))
    {
        goto error;
    }
    
    if (!PLD_OpenFont(app_state->context))
    {
        goto error;
    }

    app_state->song_menu = PLD_LoadSongMenu(app_state->context);
    if (app_state->song_menu == NULL)
    {
        goto error;
    }

    app_state->state = PLD_STATE_SONG_MENU;

    return SDL_APP_CONTINUE;

error:
    return SDL_APP_FAILURE;
}


void PLD_ReturnToSongMenu(PLD_AppState* app_state)
{
    app_state->state = PLD_STATE_SONG_MENU;
    app_state->song_menu->state = PLD_SONGMENU_STATE_SONG;
    PLD_QuitGameplay(app_state->context, app_state->gameplay);
    app_state->gameplay = NULL;

    PLD_PlayMusic(app_state->song_menu->music);
    PLD_SetMusicPosition(app_state->song_menu->music, app_state->song_menu->dataIni->thumbTimeStart);
}


SDL_AppResult SDL_AppEvent(void* user_data, SDL_Event* event)
{
    PLD_AppState* app_state = user_data;

    SDL_ConvertEventToRenderCoordinates(app_state->context->renderer, event);
        	
    PLD_EventLoop(app_state->context, event);

    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    else
    {
        switch (app_state->state)
        {
            case PLD_STATE_SONG_MENU:
                PLD_MenuKeyPress(app_state->context, app_state->song_menu, event);
                break;

            case PLD_STATE_GAMEPLAY:
                if (app_state->gameplay->pause)
                {
                    if (PLD_PauseEvent(app_state->context, &app_state->gameplay->pause, &app_state->gameplay->offset, app_state->gameplay->pauseMenu, app_state->gameplay->musicPlaying, event) != PLD_PAUSE_CONTINUE)
                    {
                        PLD_ReturnToSongMenu(app_state);
                    }
                    else
                    {
                        if (app_state->gameplay->pauseMenu->retry)
                        {
                            PLD_RetryGameplay(app_state->context, app_state->gameplay);
                        }
                    }
                }
                else
                {
                    PLD_GameplayEvent(app_state->context, app_state->gameplay, event);
                }
                break;

            case PLD_STATE_RESULT:
                if (PLD_ResultEvent(app_state->context, app_state->result, event) != PLD_RESULT_CONTINUE)
                {
                    PLD_QuitResult(app_state->result);
                    app_state->result = NULL;
                    PLD_ReturnToSongMenu(app_state);
                }
                break;

            default:
                break;
        }
    }

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void* user_data)
{
    PLD_AppState* app_state = user_data;

    PLD_ClearWindow(app_state->context);

    switch (app_state->state)
    {
        case PLD_STATE_SONG_MENU:
            PLD_SongMenuLoop(app_state->context, app_state->song_menu);

            if (app_state->song_menu->state == PLD_SONGMENU_STATE_CORE)
            {
                app_state->gameplay = PLD_InitGameplay(app_state->context, app_state->song_menu->currentSongPath, app_state->song_menu->chartPaths[app_state->song_menu->currentDifficulty], app_state->song_menu->currentDifficulty, app_state->context->config->autoplay, app_state->song_menu->background_image, app_state->song_menu->texts[PLD_SONG_LIST_MAX / 2], app_state->song_menu->dataIni, app_state->song_menu->music, app_state->song_menu->video);
                app_state->state = PLD_STATE_GAMEPLAY;
            }
            break;

        case PLD_STATE_GAMEPLAY:
            if (PLD_GameplayLoop(app_state->context, app_state->gameplay) == PLD_GAMEPLAY_SUCCESS)
            {
                PLD_StopMusic(app_state->song_menu->music);
                app_state->result = PLD_InitResult(app_state->context, app_state->song_menu->backgroundPath, app_state->song_menu->texts[PLD_SONG_LIST_MAX / 2]);
                app_state->state = PLD_STATE_RESULT;
            }
            break;

        case PLD_STATE_RESULT:
            PLD_ResultLoop(app_state->context, app_state->result);
            break;

        default:
            break;
    }

    PLD_PresentWindow(app_state->context);

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void* user_data, SDL_AppResult result)
{
    PLD_AppState* app_state = user_data;

    if (result == SDL_APP_FAILURE)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "Check PLD.log for details", NULL);
    }

    if (app_state->gameplay != NULL)
    {
        PLD_QuitGameplay(app_state->context, app_state->gameplay);
    }

    if (app_state->result != NULL)
    {
        PLD_QuitResult(app_state->result);
    }

    if (app_state->song_menu != NULL)
    {
        PLD_QuitSongMenu(app_state->context, app_state->song_menu);
    }

    PLD_CloseFont();
    PLD_DestroySounds();
    PLD_DestroyEffects();
    
    PLD_Quit(app_state->context);
    SDL_free(app_state);
}
