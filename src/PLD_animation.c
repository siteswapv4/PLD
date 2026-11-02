#include <PLD/PLD_animation.h>
#include <PLD/PLD_utils.h>

typedef struct PLD_Animation
{
	PLD_AnimationState* states;
	int num_states;
}PLD_Animation;

typedef struct PLD_AnimationNode
{
	const PLD_Animation* animation;
	PLD_Image* image;
	SDL_FPoint position;
	bool loop;
	Uint64 start_time;
	int current_state;

	void (*callback)(void* userdata);
	void* userdata;

	struct PLD_AnimationNode* next;
}PLD_AnimationNode;

typedef struct PLD_AnimationQueue
{
	PLD_AnimationNode* head;
	PLD_AnimationNode* tail;
}PLD_AnimationQueue;

PLD_Animation* PLD_LoadAnimation_IO(SDL_IOStream* stream, bool close)
{
	if (!stream) { SDL_Log("Failed to load animation : stream NULL"); return NULL; }
	PLD_Animation* animation = SDL_calloc(1, sizeof(PLD_Animation));
	if (!animation) { goto error; }

	Uint32 num_states_disk;
	PLD_READ_CHECK(stream, &num_states_disk, sizeof(Uint32));
	if (num_states_disk < 2) { SDL_Log("Failed to load animation : must have at least 2 states"); goto error; }
	animation->num_states = num_states_disk;
	animation->states = SDL_calloc(animation->num_states, sizeof(PLD_AnimationState));
	for (int i = 0; i < animation->num_states; i++)
	{
		PLD_READ_CHECK(stream, &animation->states[i].time, sizeof(Uint64));
		PLD_READ_CHECK(stream, &animation->states[i].position.x, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].position.y, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].scale.x, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].scale.y, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].rotation, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].color.r, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].color.g, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].color.b, sizeof(float));
		PLD_READ_CHECK(stream, &animation->states[i].color.a, sizeof(float));
	}
	
	if (close) { SDL_CloseIO(stream); }
	
	return animation;
	
error:
	SDL_Log("Failed to load animation : %s", SDL_GetError());
	if (animation) { PLD_DestroyAnimation(animation); }
	if (stream && close) { SDL_CloseIO(stream); }
	return NULL;
}

PLD_Animation* PLD_LoadAnimation(const char* path)
{
	return PLD_LoadAnimation_IO(SDL_IOFromFile(path, "rb"), true);
}

bool PLD_SaveAnimation(PLD_Animation* animation, const char* path)
{
	return PLD_SaveAnimation_IO(animation, SDL_IOFromFile(path, "wb"), true);
}

bool PLD_SaveAnimation_IO(PLD_Animation* animation, SDL_IOStream* stream, bool close)
{
	if (!animation) { SDL_Log("Failed to save animation : animation NULL"); return false; }
	if (!stream) { SDL_Log("Failed to save animation : stream NULL"); return false; }

	Uint32 num_states_disk = animation->num_states;
	PLD_WRITE_CHECK(stream, &num_states_disk, sizeof(Uint32));
	for (int i = 0; i < animation->num_states; i++)
	{
		PLD_WRITE_CHECK(stream, &animation->states[i].time, sizeof(Uint64));
		PLD_WRITE_CHECK(stream, &animation->states[i].position.x, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].position.y, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].scale.x, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].scale.y, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].rotation, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].color.r, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].color.g, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].color.b, sizeof(float));
		PLD_WRITE_CHECK(stream, &animation->states[i].color.a, sizeof(float));
	}

	if (close) { SDL_CloseIO(stream); }

	return true;

error:
	SDL_Log("Failed to save animation : %s", SDL_GetError());
	if (stream && close) { SDL_CloseIO(stream); }
	return false;
}

PLD_Animation* PLD_CreateAnimationFromStates(const PLD_AnimationState* states, int num_states)
{
	if (!states) { SDL_Log("Failed to create animation : states NULL"); return NULL; }
	if (num_states < 2) { SDL_Log("Failed to create animation : must have at least 2 states"); return NULL; }
	PLD_Animation* animation = SDL_calloc(1, sizeof(PLD_Animation));
	if (!animation) { goto error; }
	
	animation->num_states = num_states;
	animation->states = SDL_calloc(num_states, sizeof(PLD_AnimationState));
	if (!animation->states) { goto error; }
	
	for (int i = 0; i < num_states; i++)
	{
		animation->states[i] = states[i];
	}
	
	return animation;
	
error:
	SDL_Log("Failed to create animation : %s", SDL_GetError());
	if (animation) { PLD_DestroyAnimation(animation); }
	return NULL;
}

void PLD_DestroyAnimation(PLD_Animation* animation)
{
	if (animation)
	{
		if (animation->states) { SDL_free(animation->states); }
		SDL_free(animation);
	}
}

PLD_AnimationQueue* PLD_CreateAnimationQueue()
{
	PLD_AnimationQueue* queue = SDL_calloc(1, sizeof(PLD_AnimationQueue));
	if (!queue) { SDL_Log("Failed to create queue : %s", SDL_GetError()); return NULL; }
	return queue;
}

void PLD_DestroyAnimationQueue(PLD_AnimationQueue* queue)
{
	if (queue)
	{
		PLD_ClearAnimationQueue(queue);
		SDL_free(queue);
	}
}

void PLD_ClearAnimationQueue(PLD_AnimationQueue* queue)
{
	if (queue)
	{
		PLD_AnimationNode* it = queue->head;
		while (it)
		{
			PLD_AnimationNode* next = it->next;
			SDL_free(it);
			it = next;
		}
		queue->head = NULL;
		queue->tail = NULL;
	}
}

bool PLD_PlayAnimation(PLD_AnimationQueue* queue, const PLD_Animation* animation, PLD_Image* image, SDL_FPoint position, bool loop, Uint64 time)
{
	return PLD_PlayAnimationWithCallback(queue, animation, image, position, loop, time, NULL, NULL);
}

bool PLD_PlayAnimationWithCallback(PLD_AnimationQueue* queue, const PLD_Animation* animation, PLD_Image* image, SDL_FPoint position, bool loop, Uint64 time, void (*callback)(void* userdata), void* userdata)
{
	if (!queue)     { SDL_Log("Failed to play animation : queue NULL"); return false; }
	if (!animation) { SDL_Log("Failed to play animation : animation NULL"); return false; }
	
	PLD_AnimationNode* node = SDL_calloc(1, sizeof(PLD_AnimationNode));
	if (!node) { goto error; }
	node->animation = animation;
	node->image = image;
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

bool PLD_RenderAnimation(PLD_Context* context, PLD_AnimationNode* node, Uint64 time)
{
	PLD_AnimationState* current = &node->animation->states[node->current_state];
	PLD_AnimationState* next = &node->animation->states[node->current_state + 1];
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

	SDL_FRect rect = {position.x - node->image->width * scale.x / 2.0f, position.y - node->image->height * scale.y / 2.0f, node->image->width * scale.x, node->image->height * scale.y};

	PLD_SetImageColorMod(node->image, r * 255, g * 255, b * 255);	
	PLD_RenderImage(context, node->image, NULL, &rect, false, rotation, NULL, a * 255);
	
	return true;
}

bool PLD_RenderAnimationQueue(PLD_Context* context, PLD_AnimationQueue* queue, Uint64 time)
{
	if (!context) { SDL_Log("Failed to render animations : context NULL"); return false; }
	if (!queue) { SDL_Log("Failed to render animations : queue NULL"); return false; }
	
	PLD_AnimationNode* it = queue->head;
	PLD_AnimationNode* previous = NULL;
	PLD_AnimationNode* next = NULL;
	while (it)
	{
		next = it->next;
		if (!PLD_RenderAnimation(context, it, time))
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