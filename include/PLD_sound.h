#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <PLD_context.h>
#include <PLDUtils.h>

typedef enum PLD_SoundChannel
{
    PLD_SOUND_CHANNEL_INVALID = -1,
    PLD_SOUND_CHANNEL_NOTE,
    PLD_SOUND_CHANNEL_SLIDER,
    PLD_SOUND_CHANNEL_CHAIN_SLIDE,
    PLD_SOUND_CHANNEL_COUNT
}PLD_SoundChannel;

typedef struct PLD_Music
{
    MIX_Audio* music;

    int moved_position;
    int played_samples;
    int last_sample_time;
    int paused_time;
    int sample_rate;
}PLD_Music;

bool PLD_OpenAudio();

bool PLD_LoadSounds(PLD_Context* context);

bool PLD_PlaySound(const char* name, PLD_SoundChannel channel, bool loop);

void PLD_DestroySounds();

void PLD_SetSoundEffectVolume(float volume);

void PLD_StopChannel(PLD_SoundChannel channel);


PLD_Music* PLD_LoadMusic(PLD_Context* context, const char* path);

bool PLD_SetMusicPosition(PLD_Music* music, int position); // in ms

int PLD_GetMusicPosition(PLD_Music* music); // in ms

bool PLD_PlayMusic(PLD_Music* music);

bool PLD_StopMusic(PLD_Music* music);

void PLD_PauseMusic(PLD_Music* music);

void PLD_ResumeMusic(PLD_Music* music);

bool PLD_MusicIsPaused(PLD_Music* music);

void PLD_DestroyMusic(PLD_Music* music);

void PLD_SetMusicVolume(float volume);
