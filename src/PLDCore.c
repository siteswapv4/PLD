#include "PLDCore.h"

int PLD_APPEAR_MAX = -1;

int PLD_RenderProgress(PLD_Context* context, PLD_Song* songInfo, float foreground_coeff)
{
	int start = 0;
	
	if (songInfo->dataIni->start > 0)
	{
		start = songInfo->dataIni->start;
	}
	
    double coeff = (double)(PLD_GetMusicPosition(songInfo->music) - start) / (songInfo->dataIni->end - start);

    PLD_Image* foreground = PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM);

    SDL_FRect resRect = {0, (PLD_LOGICAL_HEIGHT - 10) + ((foreground->height - 10)  * foreground_coeff), PLD_LOGICAL_WIDTH, 10};
    SDL_FRect posRect = {PLD_LOGICAL_WIDTH * coeff - 5, (PLD_LOGICAL_HEIGHT - 10) + ((foreground->height - 10) * foreground_coeff), 10, 10};

    PLD_DrawRectangle(context, &resRect, 100, 100, 255, 150);

    PLD_DrawRectangle(context, &posRect, 255, 255, 255, 255);

    return PLD_SUCCESS;
}


SDL_FPoint PLD_GetButtonPos(SDL_FPoint position, float angle, int distance, int amplitude, int frequency, float coeff)
{
    float new_distance = distance * coeff;
    float new_frequency = SDL_sin((frequency * SDL_PI_F) * coeff);
    float new_amplitude = new_frequency * (amplitude / 12.0f);

    SDL_FPoint temp_position = {position.x + new_amplitude, position.y - new_distance};
    temp_position.x -= position.x;
    temp_position.y -= position.y;

    float c = SDL_cos((angle) * (SDL_PI_F / 180.0f));
    float s = SDL_sin((angle) * (SDL_PI_F / 180.0f));

    SDL_FPoint new_position = {temp_position.x * c - temp_position.y * s, temp_position.x * s + temp_position.y * c};
    new_position.x += position.x;
    new_position.y += position.y;

    return new_position;
}


int PLD_CalculateButtonPos(PLD_Note* note, float coeff)
{
    SDL_FPoint position = PLD_GetButtonPos((SDL_FPoint){note->targetX, note->targetY}, note->angle, note->distance, note->amplitude, note->frequency, coeff);
    note->buttonX = position.x;
    note->buttonY = position.y;

    return PLD_SUCCESS;
}

int PLD_UpdateCurrentNotes(PLD_Song* songInfo, int index, int time)
{
    songInfo->currentNotes->len = 0;

    float noteTime = -1;

    while ((index < songInfo->notes->len) && (PLD_GetNote(songInfo->notes, index)->time - PLD_GetNote(songInfo->notes, index)->flyingTime <= time))
    {
        if ((PLD_GetNote(songInfo->notes, index)->time - PLD_SAD_TIMING < time) && (PLD_GetNote(songInfo->notes, index)->active) && (!PLD_IsChainBitType(PLD_GetNote(songInfo->notes, index)->type)))
        {
            if ((noteTime == -1) || (noteTime == PLD_GetNote(songInfo->notes, index)->time))
            {
                PLD_GetNote(songInfo->notes, index)->current = 1;
                PLD_ArrayListAdd(songInfo->currentNotes, PLD_GetNote(songInfo->notes, index));
                noteTime = PLD_GetNote(songInfo->notes, index)->time;
            }
        }
        index++;
    }

    return PLD_SUCCESS;
}


float PLD_GetAngleBetweenPoints(SDL_FPoint p1, SDL_FPoint p2)
{
    return SDL_atan2f(p2.y - p1.y, p2.x - p1.x);
}


SDL_FPoint PLD_MovePointToward(SDL_FPoint point, float angle, float distance)
{
	return (SDL_FPoint){point.x + SDL_cosf(angle) * distance, point.y + SDL_sinf(angle) * distance};
}


int PLD_RenderTrail(PLD_Context* context, PLD_Note* note, float coeff)
{
    int indices[24] = {0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9};
    SDL_Vertex vertices[10] = {0};
    float r, g, b;

    if (note->isMulti)
    {
        return PLD_SUCCESS;
    }

    if (note->type <= PLD_TYPE_CIRCLEHOLD)
    {
        int type = note->type;

        if (note->type > PLD_TYPE_CIRCLE)
        {
            type -= 4;
        }

        if (type == PLD_TYPE_TRIANGLE)
        {
            r = 0.0f;
            g = 0.8f;
            b = 0.6f;
        }
        else if (type == PLD_TYPE_SQUARE)
        {
            r = 0.9f;
            g = 0.4f;
            b = 0.8f;
        }
        else if (type == PLD_TYPE_CROSS)
        {
            r = 0.2f;
            g = 0.6f;
            b = 1.0f;
        }
        else
        {
            r = 1.0f;
            g = 0.1f;
            b = 0.4f;
        }
    }
    else
    {
        r = 1.0f;
        g = 1.0f;
        b = 0.2f;
    }

    for (int i = 0; i < 10; i++)
    {
        vertices[i].color.r = r;
        vertices[i].color.g = g;
        vertices[i].color.b = b;
        vertices[i].color.a = 1 - (i + 5) / 15.0f;
    }

    float new_coeff = coeff + 0.02f;
    SDL_FPoint new_position = PLD_GetButtonPos((SDL_FPoint){note->targetX, note->targetY}, note->angle, note->distance, note->amplitude, note->frequency, new_coeff);
    float angle = PLD_GetAngleBetweenPoints((SDL_FPoint){note->buttonX, note->buttonY}, new_position);
    vertices[0].position = PLD_MovePointToward(new_position, angle + 1.5f, 5.0f);
    vertices[1].position = PLD_MovePointToward(new_position, angle - 1.5f, 5.0f);

    for (int i = 1; i < 5; i++)
    {
        new_coeff += 0.1f;
        SDL_FPoint temp_position = PLD_GetButtonPos((SDL_FPoint){note->targetX, note->targetY}, note->angle, note->distance, note->amplitude, note->frequency, new_coeff);
        angle = PLD_GetAngleBetweenPoints(new_position, temp_position);
        new_position = temp_position;

        vertices[i * 2].position = PLD_MovePointToward(new_position, angle + 1.5f, 5.0f);
        vertices[i * 2 + 1].position = PLD_MovePointToward(new_position, angle - 1.5f, 5.0f);
    }

    SDL_RenderGeometry(context->renderer, NULL, vertices, 10, indices, 24);

    return PLD_SUCCESS;
}



int PLD_RenderTarget(PLD_Context* context, PLD_Note* note, float coeff)
{
    SDL_FRect targetRect;
    SDL_FRect jikuRect;
    SDL_FRect holdRect;

    targetRect.x = jikuRect.x = holdRect.x = note->targetX;
    targetRect.y = jikuRect.y = holdRect.y = note->targetY;

    targetRect.w = PLD_GetImage(note->targetName)->width;
    targetRect.h = PLD_GetImage(note->targetName)->height;

    jikuRect.w = PLD_GetImage(PLD_GAMEPLAY_IMAGE_JIKU)->width;
    jikuRect.h = PLD_GetImage(PLD_GAMEPLAY_IMAGE_JIKU)->height;

    holdRect.w = PLD_GetImage(PLD_GAMEPLAY_IMAGE_H + note->isMulti)->width;
    holdRect.h = PLD_GetImage(PLD_GAMEPLAY_IMAGE_H + note->isMulti)->height;

    if ((coeff <= 0.97) && (coeff >= 0.94))
    {
        targetRect.w *= 1.2; targetRect.h *= 1.2;
        
        jikuRect.w *= 1.2; jikuRect.h *= 1.2;

        holdRect.w *= 1.2; holdRect.h *= 1.2;
    }

    PLD_RenderImage(context, PLD_GetImage(note->targetName), NULL, &targetRect, true, 0, NULL, PLD_IMAGE_ALPHA_FULL);

    if (PLD_IsHoldType(note->type))
    {
        PLD_RenderImage(context, PLD_GetImage(PLD_GAMEPLAY_IMAGE_H + note->isMulti), NULL, &holdRect, true, 0, NULL, PLD_IMAGE_ALPHA_FULL);
    }
    
    if (!PLD_IsChainBitType(note->type))
    {
        PLD_RenderImage(context, PLD_GetImage(PLD_GAMEPLAY_IMAGE_JIKU), NULL, &jikuRect, true, 360.0 * (1 - coeff), NULL, PLD_IMAGE_ALPHA_FULL);
    }

    return PLD_SUCCESS;
}

int PLD_RenderConnect(PLD_Context* context, PLD_Note* note)
{
	if (!note->connect->active)
		return PLD_SUCCESS;

    int x = note->connect->buttonX - note->buttonX;
    int y = note->connect->buttonY - note->buttonY;

    y *= -1;

    int distance = SDL_sqrt(SDL_pow(x, 2) + SDL_pow(y, 2));
    double angle = SDL_atan2(x, y) * (180.0 / SDL_PI_F);

    SDL_FRect rect = {note->buttonX, note->buttonY, 60, distance};

    rect.x += rect.w / 2;
    rect.x -= rect.w;
    rect.y -= rect.h;

    SDL_FPoint center = {rect.w / 2, rect.h};

    PLD_RenderImage(context, PLD_GetImage(PLD_GAMEPLAY_IMAGE_CONNECT), NULL, &rect, false, angle, &center, PLD_IMAGE_ALPHA_FULL);

    return PLD_SUCCESS;
}

void PLD_RenderGame(PLD_Context* context, PLD_Song* songInfo, PLD_Video* video, int *index, int time, PLD_Text* text, PLD_Image* background)
{
    PLD_Image* video_image = NULL;
    int tempIndex = 0;
    float coeff = 0;

    if (video != NULL)
    {
    	video_image = PLD_GetVideoFrame(video);
    }

    if (video_image != NULL)
    {
        PLD_RenderImageLetterbox(context, video_image);
    }
    else if (background != NULL)
    {
        PLD_RenderImageLetterbox(context, background);
    }

    PLD_RenderEffects(context, time);

    PLD_RenderHolds(context, time);

    //Skip past notes
    while ((*index < songInfo->notes->len) && (PLD_GetNote(songInfo->notes, *index)->time < time - PLD_SAD_TIMING))
    {
        PLD_Note* note = PLD_GetNote(songInfo->notes, *index);

        if (note->active)
        {
            PLD_DisableNote(PLD_GetNote(songInfo->notes, *index));

            if (!PLD_IsChainBitType(note->type))
            {
                PLD_EvaluateNote(note, time, false);
            }
            else
            {
                PLD_StopChannel(PLD_SOUND_CHANNEL_CHAIN_SLIDE);
            }
        }

        note->current = 0;
        *index += 1;
    }

    tempIndex = *index;

    PLD_UpdateCurrentNotes(songInfo, *index, time);

    while ((tempIndex < songInfo->notes->len) && (PLD_GetNote(songInfo->notes, tempIndex)->time - PLD_GetNote(songInfo->notes, tempIndex)->flyingTime <= time))
    {
        PLD_Note* note = PLD_GetNote(songInfo->notes, tempIndex);

        if ((tempIndex > PLD_APPEAR_MAX) && (note->active))
        {
            PLD_APPEAR_MAX = tempIndex;

            PLD_PlayEffect(PLD_EFFECT_APPEAR, PLD_GAMEPLAY_IMAGE_APPEAR, note->targetX, note->targetY);
        }

        if (note->active)
        {
            if ((PLD_IsChainBitType(note->type)) && (note->time < time) && (PLD_IsHeld(note->type - 6)))
            {
                PLD_EvaluateNote(note, time, true);
            }
            else if ((songInfo->autoplay) && (note->time < time))
            {
                PLD_EvaluateNote(note, time, true);
            }

            coeff = ((float)note->time - (float)time) / (float)note->flyingTime;

            PLD_CalculateButtonPos(((PLD_Note*)songInfo->notes->data[tempIndex]), coeff);

            if (note->connect != NULL)
            {
                PLD_RenderConnect(context, note);
            }
        }
        
        tempIndex++;
    }

    tempIndex = *index;

    while ((tempIndex < songInfo->notes->len) && (PLD_GetNote(songInfo->notes, tempIndex)->time - PLD_GetNote(songInfo->notes, tempIndex)->flyingTime <= time))
    {
        PLD_Note* note = PLD_GetNote(songInfo->notes, tempIndex);
        
        if (note->active)
        {
            coeff = ((float)note->time - (float)time) / (float)note->flyingTime;
            PLD_RenderTarget(context, note, coeff);
        }

        tempIndex++;
    }

    tempIndex = *index;

    while ((tempIndex < songInfo->notes->len) && (PLD_GetNote(songInfo->notes, tempIndex)->time - PLD_GetNote(songInfo->notes, tempIndex)->flyingTime <= time))
    {
        PLD_Note* note = PLD_GetNote(songInfo->notes, tempIndex);

        if (note->active)
        {
            coeff = ((float)note->time - (float)time) / (float)note->flyingTime;

            PLD_RenderTrail(context, note, coeff);

            SDL_FRect buttonRect = {note->buttonX, note->buttonY, PLD_GetImage(note->buttonName)->width, PLD_GetImage(note->buttonName)->height};

            PLD_RenderImage(context, PLD_GetImage(note->buttonName), NULL, &buttonRect, true, 0, NULL, PLD_IMAGE_ALPHA_FULL);
        }

        tempIndex++;
    }

    SDL_FRect foreground_top_rect = {0.0f, 0.0f, PLD_LOGICAL_WIDTH, PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP)->height};
    SDL_FRect foreground_bottom_rect = {0.0f, PLD_LOGICAL_HEIGHT - PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM)->height, PLD_LOGICAL_WIDTH, PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM)->height};

    float foreground_coeff = 0.0f;

    if (time - songInfo->dataIni->start < 1000)
    {
        foreground_coeff = 1.0f - ((time - songInfo->dataIni->start) / 1000.0f);
    }

    foreground_top_rect.y -= PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP)->height * foreground_coeff;
    foreground_bottom_rect.y += PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM)->height * foreground_coeff;

    PLD_RenderEvaluate(context, time);
    PLD_RenderImage(context, PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP), NULL, &foreground_top_rect, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);
    PLD_RenderImage(context, PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_BOTTOM), NULL, &foreground_bottom_rect, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

    PLD_RenderProgress(context, songInfo, foreground_coeff);

    PLD_RenderText(context, text, 10 , 30 - (PLD_GetImage(PLD_GAMEPLAY_IMAGE_FOREGROUND_TOP)->height + 5) * foreground_coeff, PLD_MENU_TEXT_WIDTH, 255, false);

    PLD_RenderScore(context, foreground_coeff);
}

bool PLD_StickHeld(bool buttonStates[PLD_BUTTON_COUNT], bool left_stick)
{
    int pos;

    if (left_stick)
    {
        pos = PLD_BUTTON_LEFT_STICK_UP;
    }
    else
    {
        pos = PLD_BUTTON_RIGHT_STICK_UP;
    }

    for (int i = pos; i < pos + 4; i++)
    {
        if (buttonStates[i])
        {
            return true;
        }
    }

    return false;
}

int PLD_HandleMotion(PLD_Context* context, PLD_Song songInfo, int time)
{
    if (context->gamepad == NULL)
    {
        return PLD_FAILURE;
    }

    int trigger[2];
    int axis[2][2];
    bool held;

    axis[0][0] = SDL_GetGamepadAxis(context->gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    axis[0][1] = SDL_GetGamepadAxis(context->gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    axis[1][0] = SDL_GetGamepadAxis(context->gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    axis[1][1] = SDL_GetGamepadAxis(context->gamepad, SDL_GAMEPAD_AXIS_RIGHTX);

    for (int i = 0; i < 2; i++)
    {
        held = PLD_StickHeld(context->button_states, !i);

        for (int j = 0; j < 2; j++)
        {
            if (axis[i][j] < -context->config->joystick_deadzone)
            {
                context->button_states[PLD_BUTTON_LEFT_STICK_UP + (i * 4) + ((!j) * 2)] = false;
                context->button_states[PLD_BUTTON_LEFT_STICK_UP + (i * 4) + (j * 2)] = true;

                if (!held)
                {
                    PLD_KeyPress(context, songInfo.currentNotes, time, PLD_BUTTON_LEFT_STICK_UP + (i * 4) + (j * 2), false);
                }
            }
            else if (axis[i][j] > context->config->joystick_deadzone)
            {
                context->button_states[PLD_BUTTON_LEFT_STICK_DOWN + (i * 4) + ((!j) * 2)] = false;
                context->button_states[PLD_BUTTON_LEFT_STICK_DOWN + (i * 4) + (j * 2)] = true;

                if (!held)
                {
                    PLD_KeyPress(context, songInfo.currentNotes, time, PLD_BUTTON_LEFT_STICK_DOWN + (i * 4) + (j * 2), false);
                }
            }
            else
            {
                context->button_states[PLD_BUTTON_LEFT_STICK_UP + (i * 4) + ((!j) * 2)] = false;
                context->button_states[PLD_BUTTON_LEFT_STICK_DOWN + (i * 4) + ((!j) * 2)] = false;
            }
        }
    }

    trigger[0] = SDL_GetGamepadAxis(context->gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    trigger[1] = SDL_GetGamepadAxis(context->gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

    for (int i = 0; i < 2; i++)
    {
        held = context->button_states[PLD_BUTTON_LEFT_TRIGGER + i];

        if (trigger[i] > context->config->trigger_deadzone)
        {
            context->button_states[PLD_BUTTON_LEFT_TRIGGER + i] = true;

            if (!held)
            {
                PLD_KeyPress(context, songInfo.currentNotes, time, PLD_BUTTON_LEFT_TRIGGER + i, false);
            }
        }
        else
        {
            context->button_states[PLD_BUTTON_LEFT_TRIGGER + i] = false;
        }
    }

    return PLD_SUCCESS;
}

int PLD_LoadOffset(char* songPath)
{
    int offset;
    char* offsetPath;
    SDL_IOStream* offsetFile;

    SDL_asprintf(&offsetPath, "%soffset.bin", songPath);
    offsetFile = SDL_IOFromFile(offsetPath, "rb");
    SDL_free(offsetPath);

    if (offsetFile == NULL)
    {
        return 0;
    }

    if (SDL_ReadIO(offsetFile, &offset, sizeof(int)) != sizeof(int))
    {
        return 0;
    }

    SDL_CloseIO(offsetFile);

    return offset;
}

int PLD_WriteOffset(char* songPath, int offset)
{
    char* offsetPath;
    SDL_IOStream* offsetFile;

    SDL_asprintf(&offsetPath, "%soffset.bin", songPath);
    offsetFile = SDL_IOFromFile(offsetPath, "wb");
    SDL_free(offsetPath);

    if (offsetFile == NULL)
    {
        return PLD_FAILURE;
    }

    SDL_WriteIO(offsetFile, &offset, sizeof(int));

    SDL_CloseIO(offsetFile);

    return PLD_SUCCESS;
}


PLD_Gameplay* PLD_InitGameplay(PLD_Context* context, char* songPath, char* chartPath, PLD_DifficultyType difficulty, bool autoplay, PLD_Image* background, PLD_Text* text, PLD_DataIni* dataIni, PLD_Music* music, PLD_Video* video)
{
    PLD_Gameplay* gameplay = SDL_calloc(1, sizeof(PLD_Gameplay));

    PLD_DestroyImages(PLD_IMAGE_CATEGORY_MENU);
    PLD_LoadImages(context, PLD_IMAGE_CATEGORY_GAMEPLAY);

    gameplay->offset = context->config->offset;

    gameplay->background = background;
    gameplay->offset += PLD_LoadOffset(songPath);
    gameplay->text = text;
    gameplay->songPath = songPath;
    gameplay->video = video;

    PLD_APPEAR_MAX = -1;

    gameplay->songInfo.path = songPath;
    gameplay->songInfo.difficulty = difficulty;
    gameplay->songInfo.dataIni = dataIni;
    gameplay->songInfo.music = music;
    gameplay->songInfo.chartPath = chartPath;

	if (PLD_EndsWith(gameplay->songInfo.chartPath, ".ppd"))
	{
		if (PLD_LoadPPDChart(&gameplay->songInfo) != PLD_SUCCESS)
		{
		    return NULL;
		}
	}
	else
	{
		if (PLD_LoadDSCChart(&gameplay->songInfo) != PLD_SUCCESS)
		{
			return NULL;
		}
	}
	
    gameplay->pauseMenu = PLD_InitPauseMenu(context, gameplay->offset);

    PLD_InitHolds();
    PLD_InitEvaluate();

    gameplay->songInfo.autoplay = autoplay;
    
    if (video)
    	PLD_UpdateVideoTime(video, 0);

    gameplay->startTime = SDL_GetTicks();

    return gameplay;
}


int PLD_QuitGameplay(PLD_Context* context, PLD_Gameplay* gameplay)
{
    PLD_QuitPauseMenu(gameplay->pauseMenu);

    PLD_WriteOffset(gameplay->songPath, gameplay->offset - context->config->offset);
    PLD_DestroyArrayList(gameplay->songInfo.notes, SDL_free);
    PLD_DestroyArrayList(gameplay->songInfo.currentNotes, NULL);

    PLD_StopMusic(gameplay->songInfo.music);

    PLD_FreeEvaluate();
    PLD_FreeHolds();
    
    PLD_ClearPlayingEffects();

    PLD_DestroyImages(PLD_IMAGE_CATEGORY_GAMEPLAY);
    PLD_LoadImages(context, PLD_IMAGE_CATEGORY_MENU);

    SDL_free(gameplay);

    return PLD_SUCCESS;
}


int PLD_RetryGameplay(PLD_Context* context, PLD_Gameplay* gameplay)
{
    gameplay->time = 0;
    gameplay->index = 0;
    gameplay->musicPlaying = false;
    gameplay->pause = false;
    gameplay->pauseMenu->retry = false;
    gameplay->songInfo.currentNotes->len = 0;
    for (int i = 0; i < gameplay->songInfo.notes->len; i++)
    {
        PLD_Note* note = PLD_GetNote(gameplay->songInfo.notes, i);
        note->active = true;
        note->current = false;
    }

    PLD_StopMusic(gameplay->songInfo.music);
    PLD_FreeEvaluate();
    PLD_FreeHolds();
    PLD_ClearPlayingEffects();

    PLD_APPEAR_MAX = -1;

    PLD_InitHolds();
    PLD_InitEvaluate();
    
    if (gameplay->video)
    	PLD_UpdateVideoTime(gameplay->video, 0);

    gameplay->startTime = SDL_GetTicks();

    return PLD_SUCCESS;
}


PLD_GameplayResult PLD_GameplayLoop(PLD_Context* context, PLD_Gameplay* gameplay)
{
    if (!gameplay->pause)
    {
        if (!gameplay->musicPlaying)
        {
            if (gameplay->songInfo.dataIni->start < 0)
            {
                gameplay->time = SDL_GetTicks() - gameplay->startTime + gameplay->songInfo.dataIni->start + gameplay->offset;

                if (gameplay->time - gameplay->offset > 0)
                {
                    PLD_PlayMusic(gameplay->songInfo.music);
                    gameplay->musicPlaying = true;
                }
            }
            else if (gameplay->songInfo.dataIni->start > 0)
            {
                gameplay->time = gameplay->songInfo.dataIni->start + gameplay->offset;
                PLD_PlayMusic(gameplay->songInfo.music);
                PLD_SetMusicPosition(gameplay->songInfo.music, gameplay->songInfo.dataIni->start);
                gameplay->musicPlaying = true;
            }
            else
            {
                PLD_PlayMusic(gameplay->songInfo.music);
                gameplay->musicPlaying = true;
                gameplay->time = gameplay->offset;
            }
        }
        else
        {
            gameplay->time = PLD_GetMusicPosition(gameplay->songInfo.music) + gameplay->offset;
            
            if (gameplay->video)
                PLD_UpdateVideoTime(gameplay->video, PLD_GetMusicPosition(gameplay->songInfo.music));
        }

        PLD_HandleMotion(context, gameplay->songInfo, gameplay->time);

        PLD_ProcessHolds(context, gameplay->time);
    }

    PLD_RenderGame(context, &gameplay->songInfo, gameplay->video, &gameplay->index, gameplay->time, gameplay->text, gameplay->background);

    if (gameplay->pause)
    {
        PLD_RenderPauseMenu(context, gameplay->pauseMenu);
    }

    if (gameplay->time > gameplay->songInfo.dataIni->end)
    {
        return PLD_GAMEPLAY_SUCCESS;
    }

    return PLD_GAMEPLAY_CONTINUE;
}


int PLD_GameplayEvent(PLD_Context* context, PLD_Gameplay* gameplay, SDL_Event* event)
{
    bool gamepad = false;

    if ((event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) || ((event->type == SDL_EVENT_KEY_DOWN) && (event->key.repeat == 0)))
    {
        gamepad = event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;

        PLD_MenuInput input = PLD_GetMenuPressedInput(context, event);
        if (input == PLD_MENU_INPUT_START)
        {
            PLD_SetPause(gameplay->pauseMenu, &gameplay->pause, gameplay->musicPlaying, gameplay->songInfo.music);
        }
        else
        {
            if (gamepad)
            {
                PLD_KeyPress(context, gameplay->songInfo.currentNotes, gameplay->time, event->gbutton.button, false);
            }
            else
            {
                PLD_KeyPress(context, gameplay->songInfo.currentNotes, gameplay->time, event->key.scancode, true);
            }
        }
    }

    return PLD_SUCCESS;
}
