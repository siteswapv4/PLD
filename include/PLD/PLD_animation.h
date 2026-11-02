#pragma once
#include <SDL3/SDL.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_image.h>

typedef struct PLD_AnimationState
{
	Uint64 time;
	SDL_FPoint position;
	SDL_FPoint scale;
	float rotation;
	SDL_FColor color;
}PLD_AnimationState;

typedef struct PLD_Animation PLD_Animation;

typedef struct PLD_AnimationQueue PLD_AnimationQueue;

PLD_Animation* PLD_LoadAnimation_IO(SDL_IOStream* stream, bool close);

PLD_Animation* PLD_LoadAnimation(const char* path);

bool PLD_SaveAnimation(PLD_Animation* animation, const char* path);

bool PLD_SaveAnimation_IO(PLD_Animation* animation, SDL_IOStream* stream, bool close);

PLD_Animation* PLD_CreateAnimationFromStates(const PLD_AnimationState* states, int num_states);

void PLD_DestroyAnimation(PLD_Animation* animation);

PLD_AnimationQueue* PLD_CreateAnimationQueue();

void PLD_DestroyAnimationQueue(PLD_AnimationQueue* queue); // callbacks will not be called

void PLD_DestroyAnimationQueue(PLD_AnimationQueue* queue);

void PLD_ClearAnimationQueue(PLD_AnimationQueue* queue);

bool PLD_PlayAnimation(PLD_AnimationQueue* queue, const PLD_Animation* animation, PLD_Image* image, SDL_FPoint position, bool loop, Uint64 time);

bool PLD_PlayAnimationWithCallback(PLD_AnimationQueue* queue, const PLD_Animation* animation, PLD_Image* image, SDL_FPoint position, bool loop, Uint64 time, void (*callback)(void* userdata), void* userdata);

bool PLD_RenderAnimationQueue(PLD_Context* context, PLD_AnimationQueue* queue, Uint64 time);