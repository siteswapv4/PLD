#include "PLDHolds.h"

typedef struct
{
	int code;
	bool keyboard;
}PLD_HoldKey;

typedef struct
{
    PLD_ArrayList* value[PLD_GAMEPLAY_INPUT_COUNT];
    SDL_FRect rectangle[PLD_GAMEPLAY_INPUT_COUNT];
    PLD_Image* images[PLD_GAMEPLAY_INPUT_COUNT];
    int previous_points;
    int timeout;
}PLD_Hold;

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

int PLD_InitHolds()
{
    hold = SDL_calloc(1, sizeof(PLD_Hold));

    for (int i = 0; i < 4; i++)
    {
  		hold->images[i] = PLD_GetImage(PLD_GAMEPLAY_IMAGE_0B + i * 4);

        hold->rectangle[i].w = hold->images[i]->width / 1.2;
        hold->rectangle[i].h = hold->images[i]->height / 1.2;
        hold->rectangle[i].y = 800;
        hold->rectangle[i].x = 750 + hold->images[i]->width / 1.2 * i;
    }

    for (int i = 0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        hold->value[i] = PLD_CreateArrayList();
    }

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
    return hold->value[type]->len;
}


int PLD_AddHoldKey(int type, int key, bool keyboard)
{
	PLD_HoldKey* holdKey = SDL_malloc(sizeof(PLD_HoldKey));
	
	holdKey->code = key;
	holdKey->keyboard = keyboard;
	
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

    return PLD_SUCCESS;
}

int PLD_RenderHolds(PLD_Context* context, int time)
{
	bool first = true;
	
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
        		
        		SDL_FRect rect = {hold->rectangle[0].x - hold->rectangle[0].w / 2, hold->rectangle[0].y - (hold->rectangle[0].h / 2) * coeff, hold->rectangle[3].x - hold->rectangle[0].x + hold->rectangle[0].w, hold->rectangle[0].h * coeff};

                PLD_DrawRectangle(context, &rect, 0, 0, 0, 100);
        	}
        	
            SDL_FRect holdRect = hold->rectangle[i];

            PLD_RenderImage(context, hold->images[i], NULL, &holdRect, true, 0, NULL, PLD_IMAGE_ALPHA_FULL);
        }
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
