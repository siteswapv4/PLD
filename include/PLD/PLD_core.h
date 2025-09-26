#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_effect.h>
#include <PLD/PLD_note.h>
#include <PLD/PLD_hold.h>
#include <PLD/PLD_video.h>
#include <PLD/PLD_evaluate.h>
#include <PLD/PLD_pause.h>
#include <PLD/PLD_dsc.h>
#include <PLD/PLD_ppd.h>
#include <PLD/PLD_note.h>
#include <PLD/PLD_sound.h>
#include <PLD/PLD_text.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_menu.h>
#include <PLD/PLD_context.h>

typedef enum 
{
    PLD_GAMEPLAY_INVALID,
    PLD_GAMEPLAY_CONTINUE,
    PLD_GAMEPLAY_QUIT,
    PLD_GAMEPLAY_SUCCESS,
    PLD_GAMEPLAY_COUNT
}PLD_GameplayResult;


typedef struct PLD_Gameplay
{
    int index;
    int time;
    int startTime;
    bool pause;
    bool musicPlaying;
    int offset;
    PLD_Song songInfo;
    PLD_PauseMenu* pauseMenu;
    PLD_Text* text;
    char* songPath;
    PLD_Video* video;
    PLD_Image* background;
    PLD_Music* music;
}PLD_Gameplay;


PLD_Gameplay* PLD_InitGameplay(PLD_Context* context, char* songPath, char* chartPath, PLD_DifficultyType difficulty, bool autoplay, PLD_Image* background, PLD_Text* text, PLD_DataIni* dataIni, PLD_Music* music, PLD_Video* video);

int PLD_QuitGameplay(PLD_Context* context, PLD_Gameplay* gameplay);

int PLD_RetryGameplay(PLD_Context* context, PLD_Gameplay* gameplay);

PLD_GameplayResult PLD_GameplayLoop(PLD_Context* context, PLD_Gameplay* gameplay);

int PLD_GameplayEvent(PLD_Context* context, PLD_Gameplay* gameplay, SDL_Event* event);