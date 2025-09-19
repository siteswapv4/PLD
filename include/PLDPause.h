#pragma once
#include "PLD.h"
#include "PLDControl.h"
#include <PLD_sound.h>
#include "PLDUtils.h"
#include <PLD_text.h>
#include "PLDMenu.h"

#include <PLD_context.h>


typedef enum
{
	PLD_PAUSE_INVALID = -1,
	PLD_PAUSE_CONTINUE,
	PLD_PAUSE_SUCCESS,
	PLD_PAUSE_COUNT
}PLD_PauseResult;


typedef struct
{
	int selected;
	bool retry;
	PLD_Text* resume_text;
	PLD_Text* retry_text;
	PLD_Text* exit_text;
	PLD_Text* LText;
	PLD_Text* RText;
	PLD_Text* offsetText;
	PLD_Music* music;
}PLD_PauseMenu;


PLD_PauseMenu* PLD_InitPauseMenu(PLD_Context* context, int offset);

void PLD_QuitPauseMenu(PLD_PauseMenu* pause_menu);

void PLD_SetPause(PLD_PauseMenu* pause_menu, bool* pause, bool musicPlaying, PLD_Music* music);

void PLD_RenderPauseMenu(PLD_Context* context, PLD_PauseMenu* pauseMenu);

PLD_PauseResult PLD_PauseEvent(PLD_Context* context, bool* pause, int* offset, PLD_PauseMenu* pauseMenu, bool musicPlaying, SDL_Event* event);