#include <PLD/PLD_music.h>

typedef struct PLD_Music
{
    MIX_Audio* music;

    int moved_position;
    int played_samples;
    int last_sample_time;
    int paused_time;
    int sample_rate;
}PLD_Music;

static MIX_Track* PLD_music_track = NULL;
static PLD_Music* PLD_current_music = NULL;

bool PLD_IsSupportedMusic(const char* path)
{
    return (PLD_EndsWithInsensitive(path, ".mp3")) ||
           (PLD_EndsWithInsensitive(path, ".wav")) ||
           (PLD_EndsWithInsensitive(path, ".flac")) ||
           (PLD_EndsWithInsensitive(path, ".ogg")) ||
           (PLD_EndsWithInsensitive(path, ".opus"));
}

bool PLD_InitMusic(PLD_Context* context)
{
    PLD_music_track = MIX_CreateTrack(context->mixer);
	if (!PLD_music_track) return false;

    return true;
}

void PLD_QuitMusic()
{
    if (PLD_music_track)
        MIX_DestroyTrack(PLD_music_track);
}

PLD_Music* PLD_LoadMusic(PLD_Context* context, const char* path)
{
    PLD_Music* music = SDL_calloc(1, sizeof(PLD_Music));

    music->music = MIX_LoadAudio(context->mixer, path, false);
    if (music->music == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load music : %s", SDL_GetError());
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

bool PLD_PlayMusic(PLD_Context* context, PLD_Music* music)
{
	int music_frequency = 0;
	int music_channels = 0;
	SDL_AudioFormat music_format = 0;
	
    music->played_samples = 0;
    music->last_sample_time = SDL_GetTicks();

    MIX_SetPostMixCallback(context->mixer, PLD_PostMixCallback, NULL);
    SDL_AudioSpec spec;
    music_format = MIX_GetMixerFormat(context->mixer, &spec);
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

void PLD_UpdateMusic()
{
    
}