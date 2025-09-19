#include "PLDPause.h"

int PLD_PAUSE_TIME = 0;


PLD_PauseMenu* PLD_InitPauseMenu(PLD_Context* context, int offset)
{
    PLD_PauseMenu* pause_menu = SDL_calloc(1, sizeof(PLD_PauseMenu));

    pause_menu->resume_text = PLD_CreateText(context, "RESUME");
    pause_menu->retry_text = PLD_CreateText(context, "RETRY");
    pause_menu->exit_text = PLD_CreateText(context, "EXIT");

	pause_menu->LText = PLD_CreateText(context, "L | <");
	pause_menu->RText = PLD_CreateText(context, "> | R");

	char* offset_string;
	SDL_asprintf(&offset_string, "%.3f", (offset - context->config->offset) / 1000.0);
	pause_menu->offsetText = PLD_CreateText(context, offset_string);
	SDL_free(offset_string);

    return pause_menu;
}


void PLD_QuitPauseMenu(PLD_PauseMenu* pause_menu)
{
    PLD_DestroyText(pause_menu->resume_text);
    PLD_DestroyText(pause_menu->retry_text);
    PLD_DestroyText(pause_menu->exit_text);
	PLD_DestroyText(pause_menu->LText);
	PLD_DestroyText(pause_menu->RText);
	PLD_DestroyText(pause_menu->offsetText);

    SDL_free(pause_menu);
}


void PLD_ChangeOffsetText(PLD_Context* context, PLD_PauseMenu* pauseMenu, int offset)
{
	char* offset_string;
	
	PLD_DestroyText(pauseMenu->offsetText);
	
	SDL_asprintf(&offset_string, "%.3f", (offset - context->config->offset) / 1000.0);
	pauseMenu->offsetText = PLD_CreateText(context, offset_string);
	SDL_free(offset_string);
}

void PLD_SetPause(PLD_PauseMenu* pause_menu, bool* pause, bool musicPlaying, PLD_Music* music)
{
    // Prevent slider sound from playing (dirty)
    PLD_StopChannel(0);
    PLD_StopChannel(1);
    PLD_StopChannel(2);

    pause_menu->music = music;

    pause_menu->selected = 0;

	PLD_PAUSE_TIME = SDL_GetTicks();
	
	if (musicPlaying)
	{
    	PLD_PauseMusic(pause_menu->music);
    }
    
    *pause = true;
}


void PLD_RenderPauseMenu(PLD_Context* context, PLD_PauseMenu* pauseMenu)
{
    float coeff = 1.0f;

    if (SDL_GetTicks() - PLD_PAUSE_TIME < 100)
    {
        coeff = (SDL_GetTicks() - PLD_PAUSE_TIME) / 100.0f;
    }

    PLD_Image* background_image = PLD_GetImage(PLD_GAMEPLAY_IMAGE_PAUSE_BACKGROUND);
	SDL_FRect background_rect = { PLD_LOGICAL_WIDTH / 2 - background_image->width / 2,
                                  PLD_LOGICAL_HEIGHT / 2 - background_image->height / 2 * coeff,
                                  background_image->width,
                                  background_image->height * coeff};

	PLD_RenderImage(context, background_image, NULL, &background_rect, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

    PLD_Text* texts[3] = {pauseMenu->resume_text, pauseMenu->retry_text, pauseMenu->exit_text};

    for (int i = 0; i < 3; i++)
    {
        if (pauseMenu->selected == i)
        {
            PLD_SetTextColor(texts[i], 255, 0, 0);
        }
        else
        {
            PLD_SetTextColor(texts[i], 255, 255, 255);
        }
    }

    int width_center = PLD_LOGICAL_WIDTH / 2;
    int height_center = PLD_LOGICAL_HEIGHT / 2;
    int spacing = 70;
    int offset_y = height_center + 100;
	
	PLD_RenderText(context, pauseMenu->resume_text, width_center, height_center - spacing * 2, PLD_LOGICAL_WIDTH, 255, true);

    PLD_RenderText(context, pauseMenu->retry_text, width_center, height_center - spacing, PLD_LOGICAL_WIDTH, 255, true);

    PLD_RenderText(context, pauseMenu->exit_text, width_center, height_center, PLD_LOGICAL_WIDTH, 255, true);
	
	PLD_RenderText(context, pauseMenu->offsetText, width_center, offset_y, PLD_LOGICAL_WIDTH, 255, true);

	PLD_RenderText(context, pauseMenu->LText, width_center - 200, offset_y, PLD_LOGICAL_WIDTH, 255, true);
	
	PLD_RenderText(context, pauseMenu->RText, width_center + 200, offset_y, PLD_LOGICAL_WIDTH, 255, true);
}


PLD_PauseResult PLD_PauseEvent(PLD_Context* context, bool* pause, int* offset, PLD_PauseMenu* pauseMenu, bool musicPlaying, SDL_Event* event)
{
    PLD_MenuInput input = PLD_GetMenuPressedInput(context, event);

    switch (input)
    {
        case PLD_MENU_INPUT_LEFT:
            *offset -= 1;
            PLD_ChangeOffsetText(context, pauseMenu, *offset);
            break;

        case PLD_MENU_INPUT_L:
            *offset -= 10;
            PLD_ChangeOffsetText(context, pauseMenu, *offset);
            break;

        case PLD_MENU_INPUT_RIGHT:
            *offset += 1;
            PLD_ChangeOffsetText(context, pauseMenu, *offset);
            break;

        case PLD_MENU_INPUT_R:
            *offset += 10;
            PLD_ChangeOffsetText(context, pauseMenu, *offset);
            break;

        case PLD_MENU_INPUT_UP:
            if (pauseMenu->selected > 0)
            {
                pauseMenu->selected--;
            }
            break;

        case PLD_MENU_INPUT_DOWN:
            if (pauseMenu->selected < 2)
            {
                pauseMenu->selected++;
            }
            break;

        case PLD_MENU_INPUT_EAST:
            if (pauseMenu->selected == 0)
            {
                if (musicPlaying)
                {
                    PLD_ResumeMusic(pauseMenu->music);
                }
                *pause = false;
            }
            else if (pauseMenu->selected == 1)
            {
                pauseMenu->retry = true;
            }
            else
            {
                return PLD_PAUSE_SUCCESS;
            }
            break;

        case PLD_MENU_INPUT_START:
            if (musicPlaying)
            {
                PLD_ResumeMusic(pauseMenu->music);
            }
            *pause = false;
            break;

        default:
            break;
    }

    return PLD_PAUSE_CONTINUE;
}
