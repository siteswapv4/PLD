#include <PLD/PLD_sound.h>

#define PLD_SOUND_DIRECTORY "sounds/"

static MIX_Mixer* PLD_mixer = NULL;
static MIX_Track* PLD_music_track = NULL;
static MIX_Track* PLD_sounds_tracks[PLD_SOUND_CHANNEL_COUNT] = {0};

static SDL_PropertiesID PLD_sounds = 0;
static PLD_Music* PLD_current_music = NULL;

bool PLD_IsSupportedAudio(const char* path)
{
    return (PLD_EndsWith(path, ".mp3")) ||
           (PLD_EndsWith(path, ".wav")) ||
           (PLD_EndsWith(path, ".flac")) ||
           (PLD_EndsWith(path, ".ogg")) ||
           (PLD_EndsWith(path, ".opus"));
}

bool PLD_OpenAudio()
{
	if (PLD_mixer)
	{
		SDL_Log("Audio already open");
		return false;
	}
	
	PLD_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (!PLD_mixer) { goto error; }
	
	PLD_music_track = MIX_CreateTrack(PLD_mixer);
	if (!PLD_music_track) { goto error; }
	
	for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
	{
		PLD_sounds_tracks[i] = MIX_CreateTrack(PLD_mixer);
		if (!PLD_sounds_tracks[i]) { goto error; }
	}

    PLD_sounds = SDL_CreateProperties();
    if (!PLD_sounds) { goto error; }
	
	return true;
	
error:
	SDL_Log("%s", SDL_GetError());
	if (PLD_mixer)
	{
		MIX_DestroyMixer(PLD_mixer);
		PLD_mixer = NULL;
	}
	if (PLD_music_track)
	{
		MIX_DestroyTrack(PLD_music_track);
		PLD_music_track = NULL;
	}
	for (int i = 0; i < PLD_SOUND_CHANNEL_COUNT; i++)
	{
		MIX_DestroyTrack(PLD_sounds_tracks[i]);
		PLD_sounds_tracks[i] = NULL;
	}
	
	return false;
}

bool PLD_LoadSounds(PLD_Context* context)
{
    char* directory_path = PLD_asprintf("%s%s", context->data_path, PLD_SOUND_DIRECTORY);
    char** files = SDL_GlobDirectory(directory_path, NULL, 0, NULL);
    for (int i = 0; files[i]; i++)
    {
        char* full_path = PLD_asprintf("%s/%s", directory_path, files[i]);
        MIX_Audio* audio = MIX_LoadAudio(PLD_mixer, full_path, true);
        SDL_free(full_path);
        if (audio)
        {
            PLD_RemoveExtension(files[i]);
            SDL_SetPointerProperty(PLD_sounds, files[i], audio);
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        }
    }
    SDL_free(directory_path);
    SDL_free(files);

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
    
    return MIX_PlayTrack(PLD_sounds_tracks[channel], 0);
}

void PLD_DestroySoundsCallback(void* userdata, SDL_PropertiesID props, const char* name)
{
    MIX_DestroyAudio(SDL_GetPointerProperty(props, name, NULL));
}

void PLD_DestroySounds()
{
    SDL_EnumerateProperties(PLD_sounds, PLD_DestroySoundsCallback, NULL);
    SDL_DestroyProperties(PLD_sounds);
    PLD_sounds = 0;
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


PLD_Music* PLD_LoadMusic(PLD_Context* context, const char* path)
{
    PLD_Music* music = SDL_calloc(1, sizeof(PLD_Music));

    music->music = MIX_LoadAudio(PLD_mixer, path, false);
    if (music->music == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        SDL_free(music);
        return NULL;
    }

    return music;
}


static void PLD_PostMixCallback(void* udata, MIX_Mixer* mixer, const SDL_AudioSpec* spec, float* pcm, int len)
{
    if ((PLD_current_music != NULL) && (!MIX_TrackPaused(PLD_music_track)))
    {
        PLD_current_music->played_samples += len * sizeof(float);
        PLD_current_music->last_sample_time = SDL_GetTicks();
        PLD_current_music->sample_rate = ((spec->format & 0xff) * spec->freq * spec->channels) / 8;
    }
}


bool PLD_SetMusicPosition(PLD_Music* music, int position)
{
    if (!MIX_SetTrackPlaybackPosition(PLD_music_track, MIX_TrackMSToFrames(PLD_music_track, position)))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }

    music->moved_position = position;
    music->played_samples = 0;
    music->last_sample_time = SDL_GetTicks();
    
    return true;
}


int PLD_GetMusicPosition(PLD_Music* music)
{
    if (PLD_current_music != music)
    {
        return 0;
    }

    if (!MIX_TrackPaused(PLD_music_track))
    {
    	double temp_time = (double)music->played_samples / music->sample_rate;
    	int time = temp_time * 1000;
        time += SDL_GetTicks() - music->last_sample_time;

        return (int)time + music->moved_position;
    }
    else
    {
        return music->paused_time;
    }
}


bool PLD_PlayMusic(PLD_Music* music)
{
	int music_frequency = 0;
	int music_channels = 0;
	SDL_AudioFormat music_format = 0;
	
    music->played_samples = 0;
    music->last_sample_time = SDL_GetTicks();

    MIX_SetPostMixCallback(PLD_mixer, PLD_PostMixCallback, NULL);
    SDL_AudioSpec spec;
    music_format = MIX_GetMixerFormat(PLD_mixer, &spec);
    music_frequency = spec.freq;
    music_format = spec.format;
    music_channels = spec.channels;

    music->sample_rate = ((music_format & 0xff) * music_frequency * music_channels) / 8;

    MIX_SetTrackAudio(PLD_music_track, music->music);
    MIX_PlayTrack(PLD_music_track, 0);

    PLD_current_music = music;

    return true;
}


bool PLD_StopMusic(PLD_Music* music)
{
    PLD_current_music = NULL;
    music->moved_position = 0;

    MIX_StopTrack(PLD_music_track, 0);

    return true;
}


void PLD_PauseMusic(PLD_Music* music)
{    
    music->paused_time = PLD_GetMusicPosition(music);

    MIX_PauseTrack(PLD_music_track);
}


void PLD_ResumeMusic(PLD_Music* music)
{
    MIX_ResumeTrack(PLD_music_track);

    music->last_sample_time = SDL_GetTicks();
}


bool PLD_MusicIsPaused(PLD_Music* music)
{
    return MIX_TrackPaused(PLD_music_track);
}


void PLD_DestroyMusic(PLD_Music* music)
{
    if (music != NULL)
    {
        if (music->music)
        {
            MIX_DestroyAudio(music->music);
        }

        SDL_free(music);
    }
}


void PLD_SetMusicVolume(float volume)
{
    MIX_SetTrackGain(PLD_music_track, volume);
}
