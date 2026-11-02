#pragma once
#include <SDL3/SDL.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_utils.h>

typedef enum PLD_SoundChannel
{
    PLD_SOUND_CHANNEL_INVALID = -1,
    PLD_SOUND_CHANNEL_NOTE,
    PLD_SOUND_CHANNEL_SLIDER,
    PLD_SOUND_CHANNEL_CHAIN_SLIDE,
    PLD_SOUND_CHANNEL_COUNT
}PLD_SoundChannel;

bool PLD_OpenAudio(PLD_Context* context);

void PLD_CloseAudio(PLD_Context* context);

bool PLD_LoadSounds(PLD_Context* context);

bool PLD_PlaySound(const char* name, PLD_SoundChannel channel, bool loop);

void PLD_DestroySounds();

void PLD_SetSoundEffectVolume(float volume);

void PLD_StopChannel(PLD_SoundChannel channel);

void PLD_StopSounds();

void PLD_PauseSounds(bool pause);