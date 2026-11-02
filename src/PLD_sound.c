#include <PLD/PLD_sound.h>
#include <SDL3_mixer/SDL_mixer.h>

#define PLD_SOUND_DIRECTORY "sounds/"

static MIX_Track* PLD_sounds_tracks[PLD_SOUND_CHANNEL_COUNT] = {0};

static SDL_PropertiesID PLD_sounds = 0;

bool PLD_OpenAudio(PLD_Context* context)
{
	if (context->mixer)
	{
		SDL_Log("Audio already open");
		return false;
	}

    if (!MIX_Init()) { goto error; }
	
	context->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (!context->mixer) { goto error; }
	
	for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
	{
		PLD_sounds_tracks[i] = MIX_CreateTrack(context->mixer);
		if (!PLD_sounds_tracks[i]) { goto error; }
	}

    PLD_sounds = SDL_CreateProperties();
    if (!PLD_sounds) { goto error; }
	
	return true;
	
error:
	SDL_Log("%s", SDL_GetError());
	PLD_CloseAudio(context);
	
	return false;
}

void PLD_CloseAudio(PLD_Context* context)
{
    if (!context->mixer)
    {
        SDL_Log("Audio not opened");
        return;
    }

    SDL_DestroyProperties(PLD_sounds);
    PLD_sounds = 0;
    
    for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
    {
        if (PLD_sounds_tracks[i]) { MIX_DestroyTrack(PLD_sounds_tracks[i]); }
        PLD_sounds_tracks[i] = NULL;
    }

    MIX_DestroyMixer(context->mixer);
    context->mixer = NULL;
    MIX_Quit();
}

bool PLD_LoadSounds(PLD_Context* context)
{
    char* directory_path = PLD_asprintf("%s%s", context->data_path, PLD_SOUND_DIRECTORY);
    PLD_ArrayList* files = PLD_GetDirectoryContent(directory_path, true, false, false);
    
    for (int i = 0; i < files->len; i++)
    {
        char* full_path = PLD_asprintf("%s/%s", directory_path, (char*)files->data[i]);
        MIX_Audio* audio = MIX_LoadAudio(context->mixer, full_path, true);
        SDL_free(full_path);
        if (audio)
        {
            PLD_RemoveExtension((char*)files->data[i]);
            PLD_ToLower((char*)files->data[i]);
            SDL_SetPointerProperty(PLD_sounds, (char*)files->data[i], audio);
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        }
    }
    SDL_free(directory_path);
    PLD_DestroyArrayList(files, SDL_free);

    return true;
}


bool PLD_PlaySound(const char* name, PLD_SoundChannel channel, bool loop)
{
    MIX_Audio* audio = SDL_GetPointerProperty(PLD_sounds, name, NULL);
    if (!audio)
    {
        SDL_Log("Sound : '%s' not found", name);
        return false;
    }
    if (!MIX_SetTrackAudio(PLD_sounds_tracks[channel], audio))
    {
    	SDL_Log("%s", SDL_GetError());
    	return false;
    }
    
    bool retval = MIX_PlayTrack(PLD_sounds_tracks[channel], 0);
    MIX_SetTrackLoops(PLD_sounds_tracks[channel], loop ? -1 : 0);
    return retval;
}

void PLD_DestroySoundsCallback(void* userdata, SDL_PropertiesID props, const char* name)
{
    MIX_DestroyAudio(SDL_GetPointerProperty(props, name, NULL));
}

void PLD_DestroySounds()
{
    SDL_EnumerateProperties(PLD_sounds, PLD_DestroySoundsCallback, NULL);
}


void PLD_SetSoundEffectVolume(float volume)
{
	for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
	{
		MIX_SetTrackGain(PLD_sounds_tracks[i], volume);
	}
}

void PLD_StopChannel(PLD_SoundChannel channel)
{
	MIX_StopTrack(PLD_sounds_tracks[channel], 0);
}

void PLD_StopSounds()
{
    for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
    {
        MIX_StopTrack(PLD_sounds_tracks[i], 0);
    }
}

void PLD_PauseSounds(bool pause)
{
    for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
    {
        if (pause)
        {
            MIX_PauseTrack(PLD_sounds_tracks[i]);
        }
        else
        {
            MIX_ResumeTrack(PLD_sounds_tracks[i]);
        }
    }
}
