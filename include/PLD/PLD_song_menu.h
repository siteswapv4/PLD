#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_text.h>
#include <PLD/PLD_menu.h>
#include <PLD/PLD_sound.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_video.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_ppd.h>
#include <UI/UI_popup.h>

#define PLD_SONGMENU_TEXTBACKGROUND_ALPHA 100
#define PLD_SONGMENU_TEXTBACKGROUND_HEIGHT 200
#define PLD_SONGMENU_DISPLAYED_SONGS 9

#define PLD_SONG_LIST_MAX 9

typedef enum
{
    PLD_SONGMENU_STATE_SONG,
    PLD_SONGMENU_STATE_DIFFICULTY,
    PLD_SONGMENU_STATE_CORE
}PLD_SongMenuStateType;

typedef struct PLD_SongMenu
{
    int textPos;
    char* current_directory;
    char* currentSongPath;
    char* backgroundPath;
    PLD_ArrayList* songNames;
    PLD_Text* texts[PLD_SONG_LIST_MAX];
    PLD_Text* no_song_text;
    PLD_Image* background_image;
    SDL_FRect backgroundRect;
    int currentSong;
    PLD_DifficultyType currentDifficulty;
    bool difficulties[PLD_DIFFICULTY_MAX - 1];
    PLD_Text* difficultiesText[PLD_DIFFICULTY_MAX - 1];
    char* chartPaths[PLD_DIFFICULTY_MAX - 1];
    PLD_SongMenuStateType state;
    PLD_Music* music;
    PLD_Video* video;
    PLD_DataIni* dataIni;
    PLD_ArrayList* directory_content;
    char* song_directory;

    UI_Popup* instruction_popup;
}PLD_SongMenu;


bool PLD_SongMenuKeyPress(PLD_Context* context, PLD_SongMenu* menu, SDL_Event* event);

PLD_SongMenu* PLD_LoadSongMenu(PLD_Context* context);

int PLD_SongMenuLoop(PLD_Context* context, PLD_SongMenu* song_menu);

void PLD_QuitSongMenu(PLD_Context* context, PLD_SongMenu* song_menu);