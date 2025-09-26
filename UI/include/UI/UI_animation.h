#pragma once
#include <SDL3/SDL.h>

typedef struct UI_AnimationState
{
	Uint64 time;
	SDL_FPoint position;
	SDL_FPoint scale;
	float rotation;
	SDL_FColor color;
}UI_AnimationState;

typedef struct UI_Animation UI_Animation;

typedef struct UI_AnimationQueue UI_AnimationQueue;

UI_Animation* UI_LoadAnimation_IO(SDL_IOStream* stream, bool close);

UI_Animation* UI_LoadAnimation(const char* path);

bool UI_SaveAnimation(UI_Animation* animation, const char* path);

bool UI_SaveAnimation_IO(UI_Animation* animation, SDL_IOStream* stream, bool close);

UI_Animation* UI_CreateAnimationFromStates(const UI_AnimationState* states, int num_states);

void UI_DestroyAnimation(UI_Animation* animation);

UI_AnimationQueue* UI_CreateAnimationQueue();

void UI_DestroyAnimationQueue(UI_AnimationQueue* queue); // callbacks will not be called

void UI_DestroyAnimationQueue(UI_AnimationQueue* queue);

void UI_ClearAnimationQueue(UI_AnimationQueue* queue);

bool UI_PlayAnimation(UI_AnimationQueue* queue, const UI_Animation* animation, SDL_Texture* texture, SDL_FPoint position, bool loop, Uint64 time);

bool UI_PlayAnimationWithCallback(UI_AnimationQueue* queue, const UI_Animation* animation, SDL_Texture* texture, SDL_FPoint position, bool loop, Uint64 time, void (*callback)(void* userdata), void* userdata);

bool UI_RenderAnimationQueue(SDL_Renderer* renderer, UI_AnimationQueue* queue, Uint64 time);