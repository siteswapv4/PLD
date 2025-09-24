#include "UI_animation.h"
#include "UI_utils.h"

typedef struct UI_Animation
{
	UI_AnimationState* states;
	int num_states;
}UI_Animation;

typedef struct UI_AnimationNode
{
	const UI_Animation* animation;
	SDL_Texture* texture;
	SDL_FPoint position;
	bool loop;
	Uint64 start_time;
	int current_state;

	void (*callback)(void* userdata);
	void* userdata;

	struct UI_AnimationNode* next;
}UI_AnimationNode;

typedef struct UI_AnimationQueue
{
	UI_AnimationNode* head;
	UI_AnimationNode* tail;
}UI_AnimationQueue;

UI_Animation* UI_LoadAnimation_IO(SDL_IOStream* stream, bool close)
{
	if (!stream) { SDL_Log("Failed to load animation : stream NULL"); return NULL; }
	UI_Animation* animation = SDL_calloc(1, sizeof(UI_Animation));
	if (!animation) { goto error; }

	Uint32 num_states_disk;
	UI_READ_CHECK(stream, &num_states_disk, sizeof(Uint32));
	if (num_states_disk < 2) { SDL_Log("Failed to load animation : must have at least 2 states"); goto error; }
	animation->num_states = num_states_disk;
	animation->states = SDL_calloc(animation->num_states, sizeof(UI_Animation));
	for (int i = 0; i < animation->num_states; i++)
	{
		UI_READ_CHECK(stream, &animation->states[i].time, sizeof(Uint64));
		UI_READ_CHECK(stream, &animation->states[i].position.x, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].position.y, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].scale.x, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].scale.y, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].rotation, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].color.r, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].color.g, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].color.b, sizeof(float));
		UI_READ_CHECK(stream, &animation->states[i].color.a, sizeof(float));
	}
	
	if (close) { SDL_CloseIO(stream); }
	
	return animation;
	
error:
	SDL_Log("Failed to load animation : %s", SDL_GetError());
	if (animation) { UI_DestroyAnimation(animation); }
	if (stream && close) { SDL_CloseIO(stream); }
	return NULL;
}

UI_Animation* UI_LoadAnimation(const char* path)
{
	return UI_LoadAnimation_IO(SDL_IOFromFile(path, "rb"), true);
}

bool UI_SaveAnimation(UI_Animation* animation, const char* path)
{
	return UI_SaveAnimation_IO(animation, SDL_IOFromFile(path, "wb"), true);
}

bool UI_SaveAnimation_IO(UI_Animation* animation, SDL_IOStream* stream, bool close)
{
	if (!animation) { SDL_Log("Failed to save animation : animation NULL"); return false; }
	if (!stream) { SDL_Log("Failed to save animation : stream NULL"); return false; }

	Uint32 num_states_disk = animation->num_states;
	UI_WRITE_CHECK(stream, &num_states_disk, sizeof(Uint32));
	for (int i = 0; i < animation->num_states; i++)
	{
		UI_WRITE_CHECK(stream, &animation->states[i].time, sizeof(Uint64));
		UI_WRITE_CHECK(stream, &animation->states[i].position.x, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].position.y, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].scale.x, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].scale.y, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].rotation, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].color.r, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].color.g, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].color.b, sizeof(float));
		UI_WRITE_CHECK(stream, &animation->states[i].color.a, sizeof(float));
	}

	if (close) { SDL_CloseIO(stream); }

	return true;

error:
	SDL_Log("Failed to save animation : %s", SDL_GetError());
	if (stream && close) { SDL_CloseIO(stream); }
	return false;
}

UI_Animation* UI_CreateAnimationFromStates(const UI_AnimationState* states, int num_states)
{
	if (!states) { SDL_Log("Failed to create animation : states NULL"); return NULL; }
	if (num_states < 2) { SDL_Log("Failed to create animation : must have at least 2 states"); return NULL; }
	UI_Animation* animation = SDL_calloc(1, sizeof(UI_Animation));
	if (!animation) { goto error; }
	
	animation->num_states = num_states;
	animation->states = SDL_calloc(num_states, sizeof(UI_AnimationState));
	if (!animation->states) { goto error; }
	
	for (int i = 0; i < num_states; i++)
	{
		animation->states[i] = states[i];
	}
	
	return animation;
	
error:
	SDL_Log("Failed to create animation : %s", SDL_GetError());
	if (animation) { UI_DestroyAnimation(animation); }
	return NULL;
}

void UI_DestroyAnimation(UI_Animation* animation)
{
	if (animation)
	{
		if (animation->states) { SDL_free(animation->states); }
		SDL_free(animation);
	}
}

UI_AnimationQueue* UI_CreateAnimationQueue()
{
	UI_AnimationQueue* queue = SDL_calloc(1, sizeof(UI_AnimationQueue));
	if (!queue) { SDL_Log("Failed to create queue : %s", SDL_GetError()); return NULL; }
	return queue;
}

void UI_DestroyAnimationQueue(UI_AnimationQueue* queue)
{
	if (queue)
	{
		UI_ClearAnimationQueue(queue);
		SDL_free(queue);
	}
}

void UI_ClearAnimationQueue(UI_AnimationQueue* queue)
{
	if (queue)
	{
		UI_AnimationNode* it = queue->head;
		while (it)
		{
			UI_AnimationNode* next = it->next;
			SDL_free(it);
			it = next;
		}
		queue->head = NULL;
		queue->tail = NULL;
	}
}

bool UI_PlayAnimation(UI_AnimationQueue* queue, const UI_Animation* animation, SDL_Texture* texture, SDL_FPoint position, bool loop, Uint64 time)
{
	return UI_PlayAnimationWithCallback(queue, animation, texture, position, loop, time, NULL, NULL);
}

bool UI_PlayAnimationWithCallback(UI_AnimationQueue* queue, const UI_Animation* animation, SDL_Texture* texture, SDL_FPoint position, bool loop, Uint64 time, void (*callback)(void* userdata), void* userdata)
{
	if (!queue)     { SDL_Log("Failed to play animation : queue NULL"); return false; }
	if (!animation) { SDL_Log("Failed to play animation : animation NULL"); return false; }
	
	UI_AnimationNode* node = SDL_calloc(1, sizeof(UI_AnimationNode));
	if (!node) { goto error; }
	node->animation = animation;
	node->texture = texture;
	node->position = position;
	node->loop = loop;
	node->start_time = time;
	node->callback = callback;
	node->userdata = userdata;
	
	if (!queue->tail) // tail NULL means head NULL too
	{
		queue->head = node;
		queue->tail = node;
	}
	else
	{
		queue->tail->next = node;
		queue->tail = node;
	}
	
	return true;
	
error:
	SDL_Log("Failed to play animation : %s", SDL_GetError());
	if (node) { SDL_free(node); }
	return false;
}

bool UI_RenderAnimation(SDL_Renderer* renderer, UI_AnimationNode* node, Uint64 time)
{
	UI_AnimationState* current = &node->animation->states[node->current_state];
	UI_AnimationState* next = &node->animation->states[node->current_state + 1];
	if (node->start_time + next->time < time)
	{
		if (node->current_state + 2 >= node->animation->num_states)
		{
			if (node->loop)
			{
				node->current_state = 0;
				node->start_time = time;
				current = &node->animation->states[0];
				next = &node->animation->states[1];
			}
			else
			{
				if (node->callback) { node->callback(node->userdata); }
				return false;
			}
		}
		else
		{
			node->current_state++;
			current = next;
			next = &node->animation->states[node->current_state + 1];
		}
	}
	
	float coeff = (time - node->start_time - current->time) / (float)(next->time - current->time);
	
	SDL_FPoint position = {current->position.x + (next->position.x - current->position.x) * coeff + node->position.x, current->position.y + (next->position.y - current->position.y) * coeff + node->position.y};
	SDL_FPoint scale = {current->scale.x + (next->scale.x - current->scale.x) * coeff, current->scale.y + (next->scale.y - current->scale.y) * coeff};
	float rotation = current->rotation + (next->rotation - current->rotation) * coeff;
	float r = current->color.r + (next->color.r - current->color.r) * coeff;
	float g = current->color.g + (next->color.g - current->color.g) * coeff;
	float b = current->color.b + (next->color.b - current->color.b) * coeff;
	float a = current->color.a + (next->color.a - current->color.a) * coeff;

	SDL_FRect rect = {position.x - node->texture->w * scale.x / 2.0f, position.y - node->texture->h * scale.y / 2.0f, node->texture->w * scale.x, node->texture->h * scale.y};

	SDL_SetTextureColorModFloat(node->texture, r, g, b);	
	SDL_SetTextureAlphaModFloat(node->texture, a);
	SDL_RenderTextureRotated(renderer, node->texture, NULL, &rect, rotation, NULL, SDL_FLIP_NONE);
	
	return true;
}

bool UI_RenderAnimationQueue(SDL_Renderer* renderer, UI_AnimationQueue* queue, Uint64 time)
{
	if (!renderer) { SDL_Log("Failed to render animations : renderer NULL"); return false; }
	if (!queue) { SDL_Log("Failed to render animations : queue NULL"); return false; }
	
	UI_AnimationNode* it = queue->head;
	UI_AnimationNode* previous = NULL;
	UI_AnimationNode* next = NULL;
	while (it)
	{
		next = it->next;
		if (!UI_RenderAnimation(renderer, it, time))
		{
			if (it == queue->head)
			{
				queue->head = next;
			}
			if (it == queue->tail)
			{
				queue->tail = previous;
			}
			if (previous)
			{
				previous->next = next;
			}
			SDL_free(it);
			it = previous;
		}
		previous = it;
		it = next;
	}

	return true;
}