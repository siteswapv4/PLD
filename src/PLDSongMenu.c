#include "PLDSongMenu.h"

const char* PLD_PPD_DIFFICULTY_FILES[PLD_DIFFICULTY_MAX] = {"Easy.ppd", "Normal.ppd", "Hard.ppd", "Extreme.ppd", "ExtraExtreme.ppd", "Base.ppd"};

const char* PLD_DSC_DIFFICULTY_FILES[PLD_DIFFICULTY_MAX] = {"Easy.dsc", "Normal.dsc", "Hard.dsc", "Extreme.dsc", "ExtraExtreme.dsc", "Base.dsc"};

const char* PLD_SONG_MENU_DIFFICULTY_STRING[PLD_DIFFICULTY_MAX] = {"EASY", "NORMAL", "HARD", "EXTREME", "EXEX", "BASE"};


int PLD_CompareString(const void* left, const void* right)
{
	const char* a = *(const void**)left;
	const char* b = *(const void**)right;
	
	return SDL_strcmp(a, b);
}


int PLD_GetSongMenuIndex(PLD_SongMenu* menu, int offset)
{
    return ((menu->currentSong + offset) % menu->songNames->len + menu->songNames->len) % menu->songNames->len;
}


int PLD_LoadSongInfo(PLD_Context* context, PLD_SongMenu* menu)
{
    for (int i = 0; i < menu->directory_content->len; i++)
    {
    	for (int j = PLD_DIFFICULTY_MAX - 2; j >= 0; j--)
    	{
			if ((!menu->difficulties[j]) && ((PLD_EndsWith((char*)menu->directory_content->data[i], PLD_PPD_DIFFICULTY_FILES[j])) || (PLD_EndsWith((char*)menu->directory_content->data[i], PLD_DSC_DIFFICULTY_FILES[j]))))
		    {
		        menu->difficulties[j] = true;
		        menu->chartPaths[j] = SDL_strdup((char*)menu->directory_content->data[i]);
		        
		        if (menu->currentDifficulty < j)
		        {
		            menu->currentDifficulty = j;
		        }
		    }
	   }
    }

    return PLD_SUCCESS;
}


int PLD_LoadSongMedia(PLD_Context* context, PLD_SongMenu* menu)
{
    for (int i = 0; i < menu->directory_content->len; i++)
    {
        if (PLD_IsSupportedAudio((char*)menu->directory_content->data[i]))
        {
    		menu->music = PLD_LoadMusic(context, (char*)menu->directory_content->data[i]);
    		break;
    	}
    }
    
    for (int i = 0; i < menu->directory_content->len; i++)
    {
    	if (PLD_IsSupportedVideo((char*)menu->directory_content->data[i]))
    	{
    		menu->video = PLD_LoadVideo(context, (char*)menu->directory_content->data[i], menu->dataIni->thumbTimeStart);
    		break;
    	}
    }
    
    for (int i = 0; i < menu->directory_content->len; i++)
    {
        if (PLD_IsSupportedImage((char*)menu->directory_content->data[i]))        {
            menu->backgroundPath = SDL_strdup((char*)menu->directory_content->data[i]);
            if (!menu->video)
            {
                menu->background_image = PLD_LoadImage(context, (char*)menu->directory_content->data[i]);
            }
            break;
        }
    }

    PLD_PlayMusic(menu->music);
    PLD_SetMusicPosition(menu->music, menu->dataIni->thumbTimeStart);
	
	return PLD_SUCCESS;
}


int PLD_DestroySongContent(PLD_Context* context, PLD_SongMenu* menu)
{
    for (int i = 0; i < PLD_DIFFICULTY_MAX - 1; i++)
    {
        if (menu->difficulties[i])
        {
            SDL_free(menu->chartPaths[i]);
            menu->chartPaths[i] = NULL;
        }
    }

    if (menu->currentSongPath != NULL)
    {
        SDL_free(menu->currentSongPath);
        menu->currentSongPath = NULL;
    }

    if (menu->dataIni != NULL)
    {
        SDL_free(menu->dataIni);
        menu->dataIni = NULL;
    }

    if (menu->directory_content != NULL)
    {
        PLD_DestroyArrayList(menu->directory_content, SDL_free);
        menu->directory_content = NULL;
    }

    if (menu->background_image)
    {
        PLD_DestroyImage(menu->background_image);
        menu->background_image = NULL;
    }

    if (menu->backgroundPath != NULL)
    {
        SDL_free(menu->backgroundPath);
        menu->backgroundPath = NULL;
    }
    
    if (menu->video != NULL)
    {
    	PLD_FreeVideo(menu->video);
    	menu->video = NULL;
    }
    
    if (menu->music != NULL)
    {
    	PLD_StopMusic(menu->music);
    	PLD_DestroyMusic(menu->music);
    	menu->music = NULL;
    }

    for (int i = 0; i < PLD_DIFFICULTY_MAX - 1; i++)
    {
        menu->difficulties[i] = false;
    }

    menu->state = PLD_SONGMENU_STATE_SONG;
    menu->currentDifficulty = PLD_DIFFICULTY_NONE;

    return PLD_SUCCESS;
}

int PLD_UpdateSongMenuCurrentSong(PLD_Context* context, PLD_SongMenu* menu, int offset)
{
	char* iniPath = NULL;

    if (menu->songNames->len > 0)
    {
        // This is kinda black magic, don't touch it
        // It'll be very inefficient if offset > PLD_SONG_LIST_MAX / 2
        for (int k = 0; k < SDL_abs(offset); k++)
        {
            if (offset > 0)
            {
                PLD_DestroyText(menu->texts[0]);

                for (int i = 1; i < PLD_SONG_LIST_MAX; i++)
                {
                    menu->texts[i - 1] = menu->texts[i];
                }

                menu->texts[PLD_SONG_LIST_MAX - 1] = PLD_CreateText(context, menu->songNames->data[PLD_GetSongMenuIndex(menu, PLD_SONG_LIST_MAX / 2 + 1)]);
                menu->currentSong = PLD_GetSongMenuIndex(menu, 1);
            }
            else if (offset < 0)
            {
                PLD_DestroyText(menu->texts[PLD_SONG_LIST_MAX - 1]);

                for (int i = PLD_SONG_LIST_MAX - 2; i >= 0; i--)
                {
                    menu->texts[i + 1] = menu->texts[i];
                }

                menu->texts[0] = PLD_CreateText(context, menu->songNames->data[PLD_GetSongMenuIndex(menu, (PLD_SONG_LIST_MAX / 2 + 1) * -1)]);
                menu->currentSong = PLD_GetSongMenuIndex(menu, -1);
            }
        }

        PLD_DestroySongContent(context, menu);

        SDL_asprintf(&menu->currentSongPath, "%s/%s/%s/", menu->song_directory, menu->current_directory, (char*)menu->songNames->data[menu->currentSong]);
        
        SDL_asprintf(&iniPath, "%sdata.ini", menu->currentSongPath);
        menu->dataIni = PLD_LoadDataIni(iniPath);
        SDL_free(iniPath);

        if (menu->dataIni != NULL)
        {
            menu->directory_content = PLD_GetDirectoryContent(menu->currentSongPath, true, false, true);

            PLD_LoadSongInfo(context, menu);
            
            if (context->config->load_song_menu_media)
            {
                PLD_LoadSongMedia(context, menu);
            }
        }
    }

    return PLD_SUCCESS;
}

int PLD_UpdateSongMenuDifficulty(PLD_Context* context, PLD_SongMenu* menu, int move)
{
    do
    {
        menu->currentDifficulty += move;

        if (menu->currentDifficulty > PLD_DIFFICULTY_MAX - 2)
        {
            menu->currentDifficulty = 0;
        }
        else if (menu->currentDifficulty < 0)
        {
            menu->currentDifficulty = PLD_DIFFICULTY_MAX - 2;
        }
    }while (!menu->difficulties[menu->currentDifficulty]);

    return PLD_SUCCESS;
}


int PLD_LoadSongMenuDirectory(PLD_Context* context, PLD_SongMenu* menu)
{
    PLD_DestroySongContent(context, menu);

    if (menu->songNames != NULL)
    {
        PLD_DestroyArrayList(menu->songNames, SDL_free);
    }
    char* tempPath;
	SDL_asprintf(&tempPath, "%s/%s/", menu->song_directory, menu->current_directory); 
    menu->songNames = PLD_GetDirectoryContent(tempPath, false, true, false);
    SDL_free(tempPath);

    menu->currentSong = 0;

    if (menu->songNames->len > 0)
    {
    	SDL_qsort(menu->songNames->data, menu->songNames->len, sizeof(menu->songNames->data[0]), PLD_CompareString);

        int count = 0;
        for (int i = PLD_SONG_LIST_MAX / 2 * -1; i <= PLD_SONG_LIST_MAX / 2; i++)
        {
            if (menu->texts[count] != NULL)
            {
                PLD_DestroyText(menu->texts[count]);
            }

            menu->texts[count] = PLD_CreateText(context, menu->songNames->data[PLD_GetSongMenuIndex(menu, i)]);
            count++;
        }
    }

    PLD_UpdateSongMenuCurrentSong(context, menu, 0);

    return PLD_SUCCESS;
}


PLD_SongMenu* PLD_LoadSongMenu(PLD_Context* context)
{
    PLD_SongMenu* menu = SDL_calloc(1, sizeof(PLD_SongMenu));
	
    PLD_LoadImages(context, PLD_IMAGE_CATEGORY_MENU);

	menu->state = PLD_SONGMENU_STATE_SONG;

    menu->backgroundRect.w = PLD_LOGICAL_WIDTH;
    menu->backgroundRect.h = PLD_LOGICAL_HEIGHT;

    menu->textPos = PLD_LOGICAL_HEIGHT / 2 - (PLD_FONT_SIZE / 2);
    menu->textPos -= 80 * (PLD_SONG_LIST_MAX / 2);

    for (int i = 0; i < PLD_DIFFICULTY_MAX - 1; i++)
    {
    	menu->difficultiesText[i] = PLD_CreateText(context, PLD_SONG_MENU_DIFFICULTY_STRING[i]);
    }
    menu->no_song_text = PLD_CreateText(context, "Press a key to add song folder");

    menu->current_directory = SDL_strdup("");
    SDL_asprintf(&menu->song_directory, "%s/songs/", context->data_path);

    PLD_LoadSongMenuDirectory(context, menu);

    return menu; 
}

void PLD_FolderCallback(void* userdata, const char* directory)
{
    PLD_Context* context = ((void**)userdata)[0];
    PLD_SongMenu* song_menu = ((void**)userdata)[1];

    SDL_free(song_menu->song_directory);
    song_menu->song_directory = SDL_strdup(directory);
    PLD_LoadSongMenuDirectory(context, song_menu);

    SDL_free(userdata);
}

int PLD_MenuKeyPress(PLD_Context* context, PLD_SongMenu* menu, SDL_Event* event)
{
    if (menu->songNames->len == 0)
    {
        if (PLD_GetMenuPressedInput(context, event) != PLD_MENU_INPUT_INVALID)
        {
            void** data = SDL_malloc(2 * sizeof(void*));
            data[0] = context;
            data[1] = menu;
            PLD_ShowDirectoryPicker(PLD_FolderCallback, data);
        }

        return PLD_SUCCESS;
    }

    switch (PLD_GetMenuPressedInput(context, event))
    {
        case PLD_MENU_INPUT_EAST:
            if (menu->state == PLD_SONGMENU_STATE_DIFFICULTY)
            {
                if (menu->music == NULL)
                {
                    PLD_LoadSongMedia(context, menu);
                }

            	PLD_StopMusic(menu->music);
                context->config->autoplay = false;
                menu->state = PLD_SONGMENU_STATE_CORE;
            }
            else if (menu->state == PLD_SONGMENU_STATE_SONG)
            {
                if (menu->dataIni != NULL)
                {
                    if (menu->currentDifficulty != PLD_DIFFICULTY_NONE)
                    {
                        menu->state = PLD_SONGMENU_STATE_DIFFICULTY;
                    }
                }
                else
                {
                    char* new_path;
                    SDL_asprintf(&new_path, "%s/%s", menu->current_directory, (char*)menu->songNames->data[menu->currentSong]);
                    SDL_free(menu->current_directory);
                    menu->current_directory = new_path;

                    PLD_LoadSongMenuDirectory(context, menu);
                }
            }
            
            break;

        case PLD_MENU_INPUT_SOUTH:
            if (menu->state == PLD_SONGMENU_STATE_DIFFICULTY)
            {
                menu->state = PLD_SONGMENU_STATE_SONG;
            }
            else if (SDL_strlen(menu->current_directory) != 0)
            {
                int i;
                for (i = SDL_strlen(menu->current_directory) - 1; (i >= 0) && (menu->current_directory[i] != '\\') && (menu->current_directory[i] != '/'); i--);
                menu->current_directory[i+1] = '\0';

                PLD_LoadSongMenuDirectory(context, menu);
            }
            break;

        case PLD_MENU_INPUT_NORTH:
            if (menu->state == PLD_SONGMENU_STATE_DIFFICULTY)
            {
                if (menu->music == NULL)
                {
                    PLD_LoadSongMedia(context, menu);
                }

            	PLD_StopMusic(menu->music);
                context->config->autoplay = true;
                menu->state = PLD_SONGMENU_STATE_CORE;
            }
            else if ((menu->dataIni != NULL) && (menu->music == NULL))
            {
                PLD_LoadSongMedia(context, menu);
            }
            break;

        case PLD_MENU_INPUT_UP:
            if (menu->state == PLD_SONGMENU_STATE_SONG)
            {
                PLD_UpdateSongMenuCurrentSong(context, menu, -1);
            }
            else if (menu->state == PLD_SONGMENU_STATE_DIFFICULTY)
            {
                PLD_UpdateSongMenuDifficulty(context, menu, -1);
            }
            break;

        case PLD_MENU_INPUT_DOWN:
            if (menu->state == PLD_SONGMENU_STATE_SONG)
            {
                PLD_UpdateSongMenuCurrentSong(context, menu, 1);
            }
            else if (menu->state == PLD_SONGMENU_STATE_DIFFICULTY)
            {
                PLD_UpdateSongMenuDifficulty(context, menu, 1);
            }
            break;

        case PLD_MENU_INPUT_L:
            if (menu->state == PLD_SONGMENU_STATE_SONG)
            {
                PLD_UpdateSongMenuCurrentSong(context, menu, (PLD_SONG_LIST_MAX / 2 + 1) * -1 );
            }
            break;

        case PLD_MENU_INPUT_R:
            if (menu->state == PLD_SONGMENU_STATE_SONG)
            {
                PLD_UpdateSongMenuCurrentSong(context, menu, PLD_SONG_LIST_MAX / 2 + 1);
            }
            break;

        default:
            break;
    }

    return PLD_SUCCESS;
}

int PLD_RenderSongMenu(PLD_Context* context, PLD_SongMenu* menu)
{
    PLD_Image* video_image = NULL;
    PLD_Image* song_bar_image = PLD_GetImage(PLD_MENU_IMAGE_SONG_BAR);
    SDL_FRect song_bar_rect = {30.0f, 0.0f, song_bar_image->width, song_bar_image->height};
    int alpha[PLD_SONG_LIST_MAX] = {100, 120, 140, 160, 255, 160, 140, 120, 100};

    if (menu->songNames->len > 0)
    {
        PLD_RenderImageLetterbox(context, PLD_GetImage(PLD_MENU_IMAGE_BACKGROUND));

        if (menu->video != NULL)
        {
            video_image = PLD_GetVideoFrame(menu->video);
        }

        if (video_image != NULL)
        {
            PLD_RenderImageLetterbox(context, video_image);
        }
        else if (menu->background_image != NULL)
        {
            PLD_RenderImageLetterbox(context, menu->background_image);
        }
    }

    PLD_Image* bottom_image = PLD_GetImage(PLD_MENU_IMAGE_SONG_MENU_BOTTOM);
    SDL_FRect bottom_rect = { 0.0f, PLD_LOGICAL_HEIGHT - bottom_image->height, bottom_image->width, bottom_image->height };
    PLD_RenderImage(context, bottom_image, NULL, &bottom_rect, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

    PLD_Image* title_image = PLD_GetImage(PLD_MENU_IMAGE_SONG_MENU_TITLE);
    SDL_FRect title_rect = {PLD_LOGICAL_WIDTH - title_image->width, 10.0f, title_image->width, title_image->height};
    PLD_RenderImage(context, title_image, NULL, &title_rect, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

    int textPos = menu->textPos;

    if (menu->songNames->len > 0)
    {
        for (int i = 0; i < PLD_SONG_LIST_MAX; i++)
        {
            song_bar_rect.y = textPos - 30;
            PLD_RenderImage(context, song_bar_image, NULL, &song_bar_rect, false, 0, NULL, alpha[i]);

            PLD_RenderText(context, menu->texts[i], song_bar_rect.x + 50, textPos, PLD_MENU_TEXT_WIDTH, alpha[i], false);

            textPos += 80;
        }
        
        if (menu->state == PLD_SONGMENU_STATE_DIFFICULTY)
        {
            PLD_Image* info_image = PLD_GetImage(PLD_MENU_IMAGE_SONG_INFO);
            SDL_FRect info_rect = {PLD_LOGICAL_WIDTH - info_image->width, PLD_LOGICAL_HEIGHT - info_image->height - bottom_image->height, info_image->width, info_image->height};
            PLD_RenderImage(context, info_image, NULL, &info_rect, false, 0, NULL, PLD_IMAGE_ALPHA_FULL);

            int position_x = info_rect.x + 150;
            int position_y = info_rect.y + 117;

            for (int i = 0; i < PLD_DIFFICULTY_MAX - 1; i++)
            {
                if (!menu->difficulties[i])
                {
                    PLD_SetTextColor(menu->difficultiesText[i], 150, 150, 150);
                }
                else if (i == menu->currentDifficulty)
                {
                    PLD_SetTextColor(menu->difficultiesText[i], 255, 0, 0);
                }
                else
                {
                    PLD_SetTextColor(menu->difficultiesText[i], 255, 255, 255);
                }
                PLD_RenderText(context, menu->difficultiesText[i], position_x, position_y, PLD_LOGICAL_WIDTH, 255, true);
                
                position_y += 72;
            }
        }
    }
    else
    {
        PLD_RenderText(context, menu->no_song_text, PLD_LOGICAL_WIDTH / 2.0f, PLD_LOGICAL_HEIGHT / 2.0f, PLD_LOGICAL_WIDTH, 255, true);
    }

    return PLD_SUCCESS;
}

int PLD_FreeMenu(PLD_SongMenu* menu)
{
    for (int i = 0; i < PLD_SONG_LIST_MAX; i++)
    {
        if (menu->texts[i] != NULL)
        {
            PLD_DestroyText(menu->texts[i]);
        }
    }    

    if (menu->currentSongPath != NULL)
    {
        SDL_free(menu->currentSongPath);
    }

    if (menu->current_directory != NULL)
    {
        SDL_free(menu->current_directory);
    }

    PLD_DestroyArrayList(menu->songNames, SDL_free);
    PLD_DestroyImage(menu->background_image);

    SDL_free(menu);

    return PLD_SUCCESS;
}

int PLD_SongMenuLoop(PLD_Context* context, PLD_SongMenu* menu)
{
    if (menu->music != NULL)
    {  
        if (PLD_GetMusicPosition(menu->music) > menu->dataIni->thumbTimeEnd)
        {
            PLD_SetMusicPosition(menu->music, menu->dataIni->thumbTimeStart);
        }

        if (menu->video != NULL)
        {
            PLD_UpdateVideoTime(menu->video, PLD_GetMusicPosition(menu->music));
        }
    }

    PLD_RenderSongMenu(context, menu);

    return PLD_SUCCESS;
}


void PLD_QuitSongMenu(PLD_Context* context, PLD_SongMenu* song_menu)
{
    PLD_DestroySongContent(context, song_menu);

    PLD_DestroyArrayList(song_menu->songNames, SDL_free);

    if (song_menu->current_directory != NULL)
    {
        SDL_free(song_menu->current_directory);
    }

    for (int i = 0; i < PLD_SONG_LIST_MAX; i++)
    {
        PLD_DestroyText(song_menu->texts[i]);
    }

    for (int i = 0; i < PLD_DIFFICULTY_MAX - 1; i++)
    {
        PLD_DestroyText(song_menu->difficultiesText[i]);
        SDL_free(song_menu->chartPaths[i]);
    }

    PLD_DestroyText(song_menu->no_song_text);
    SDL_free(song_menu->song_directory);

    PLD_DestroyImages(PLD_IMAGE_CATEGORY_MENU);
    SDL_free(song_menu);
}