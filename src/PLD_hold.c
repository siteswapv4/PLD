#include <PLD/PLD_hold.h>

typedef struct
{
	int code;
	bool keyboard;
}PLD_HoldKey;

typedef struct
{
    PLD_ArrayList* value[PLD_GAMEPLAY_INPUT_COUNT];
    PLD_Image* images[PLD_GAMEPLAY_INPUT_COUNT];
    int previous_points;
    int timeout;
    bool autoplay;

    Uint64 last_slide_l;
    Uint64 last_slide_r;
}PLD_Hold;

#define PLD_CHAIN_HOLD_THRESHOLD 150

PLD_Hold* hold = NULL;

int PLD_CountHolds()
{
    int count = 0;

    for (int i = 0; i < 4; i++)
    {
        if (hold->value[i]->len != 0)
        {
            count++;
        }
    }

    return count;
}

int PLD_InitHolds(bool autoplay)
{
    hold = SDL_calloc(1, sizeof(PLD_Hold));

    for (int i = 0; i < 4; i++)
    {
  		hold->images[i] = PLD_GetImage(PLD_GAMEPLAY_IMAGE_0B + i * 4);
    }

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        hold->value[i] = PLD_CreateArrayList();
    }

    hold->autoplay = autoplay;

    return PLD_SUCCESS;
}

int PLD_DropHolds()
{
    for (int i = 0; i < 4; i++)
    {
		for (int j = 0; j < hold->value[i]->len; j++)
		{
			SDL_free(hold->value[i]->data[j]);
		}
        
		hold->value[i]->len = 0;
    }

    hold->timeout = 0;
    hold->previous_points = 0;

    return PLD_SUCCESS;
}


int PLD_IsHeld(int type)
{    
    if ((type == PLD_GAMEPLAY_INPUT_L) && (SDL_GetTicks() - hold->last_slide_l < PLD_CHAIN_HOLD_THRESHOLD))
        return true;
    else if ((type == PLD_GAMEPLAY_INPUT_R) && (SDL_GetTicks() - hold->last_slide_r < PLD_CHAIN_HOLD_THRESHOLD))
        return true;
    
    return hold->value[type]->len;
}


int PLD_AddHoldKey(int type, int key, bool keyboard)
{
	PLD_HoldKey* holdKey = SDL_malloc(sizeof(PLD_HoldKey));
	
	holdKey->code = key;
	holdKey->keyboard = keyboard;

    if (type == PLD_GAMEPLAY_INPUT_L)
    {
        hold->last_slide_l = SDL_GetTicks();
    }
    else if (type == PLD_GAMEPLAY_INPUT_R)
    {
        hold->last_slide_r = SDL_GetTicks();
    }
	
    return PLD_ArrayListAdd(hold->value[type], holdKey);
}

int PLD_SetHoldTimeout(int time)
{
    hold->timeout = time + PLD_HOLD_TIMEOUT;
    hold->previous_points = 0;

    return PLD_SUCCESS;
}

int PLD_ProcessHolds(PLD_Context* context, int time)
{
    if (!hold->autoplay)
    {
        for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
        {
            if (hold->value[i]->len > 0)
            {
                for (int j = hold->value[i]->len - 1; j >= 0; j--)
                {
                    PLD_HoldKey* holdKey = hold->value[i]->data[j];
                    
                    if (holdKey->keyboard)
                    {
                        if (!context->key_states[holdKey->code])
                        {
                            SDL_free(PLD_ArrayListRemoveAt(hold->value[i], j));
                        }
                    }
                    else
                    {
                        if (!context->button_states[holdKey->code])
                        {
                            SDL_free(PLD_ArrayListRemoveAt(hold->value[i], j));
                        }
                    }
                }

                if ((i < 4) && (!hold->value[i]->len))
                {
                    PLD_DropHolds();
                }
            }
        }
    }

    int hold_count = PLD_CountHolds();
    if (hold_count > 0)
    {
        int elapsed_frames = 0;

        if (time > hold->timeout)
        {
            elapsed_frames = PLD_HOLD_MAX_FRAMES;
            PLD_AddScore(PLD_HOLD_MAX_BONUS * hold_count);
            PLD_DropHolds();
        }
        else
        {
            elapsed_frames = (time + PLD_HOLD_TIMEOUT - hold->timeout) / (1000.0f / 60.0f) ;
        }

        int hold_points = PLD_HOLD_POINTS * elapsed_frames * hold_count;

        PLD_AddScore(hold_points - hold->previous_points);
        hold->previous_points = hold_points;
    }

    if (hold->value[PLD_GAMEPLAY_INPUT_L]->len)
        hold->last_slide_l = SDL_GetTicks();
    if (hold->value[PLD_GAMEPLAY_INPUT_R]->len)
        hold->last_slide_r = SDL_GetTicks();

    return PLD_SUCCESS;
}

int PLD_RenderHolds(PLD_Context* context, int time)
{
	bool first = true;
    int num_held = 0;

    int hold_size = 90;
    int hold_spacing = 60;
    int start_x = 710;
    int start_y = 800;
	
    for (int i = 0; i < 4; i++)
    {
        if (PLD_IsHeld(i))
        {
        	if (first)
        	{
                float coeff = 1.0f;
        		first = false;

                if ((time + PLD_HOLD_TIMEOUT) - hold->timeout <= 100)
                {
                    coeff = ((time + PLD_HOLD_TIMEOUT) - hold->timeout) / 100.0f;
                }
        		
        		SDL_FRect rect = {start_x - 100, start_y + 50 * (1.0f - coeff), 700, 100 * coeff};

                PLD_RenderImage(context, PLD_GetImage(PLD_GAMEPLAY_IMAGE_HOLD_BAR), NULL, &rect, false, 0, NULL, 200);
        	}
        	
            SDL_FRect rect = {start_x + hold_spacing * num_held, start_y, hold_size, hold_size};
            PLD_RenderImage(context, hold->images[i], NULL, &rect, false, 0, NULL, 255);
            num_held++;
        }
    }

    if (!first)
    {
        PLD_RenderNumber(context, start_x + 300, start_y + 25, hold->previous_points, 0.75f, false);
    }

    return PLD_SUCCESS;
}

int PLD_FreeHolds()
{
    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        PLD_DestroyArrayList(hold->value[i], SDL_free);
    }

    SDL_free(hold);
    hold = NULL;

    return PLD_SUCCESS;
}