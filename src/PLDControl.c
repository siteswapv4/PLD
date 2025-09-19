#include "PLDControl.h"

PLD_ArrayList* PLD_GetPressedTypes(int pressedCode, PLD_ArrayList** keys)
{
    PLD_ArrayList* pressedTypes = PLD_CreateArrayList();

    for (int i=0; i < PLD_GAMEPLAY_INPUT_COUNT; i++)
    {
        for (int j = 0; j < keys[i]->len; j++)
        {
            if (pressedCode == *(int*)keys[i]->data[j])
            {
                PLD_ArrayListAdd(pressedTypes, PLD_DupInt(i));
            }
        }
    }

    return pressedTypes;
}


int PLD_TypeInKeyTypes(PLD_ArrayList* keyTypes, int type)
{
    int i;

    for (i=0; i<keyTypes->len; i++)
    {
        if (*(int*)keyTypes->data[i] == type)
        {
            return i;
        }
        else if ((type == PLD_GAMEPLAY_INPUT_L) && (*(int*)keyTypes->data[i] == PLD_GAMEPLAY_INPUT_NORTH))
        {
            return i;
        }
        else if ((type == PLD_GAMEPLAY_INPUT_L) && (*(int*)keyTypes->data[i] == PLD_GAMEPLAY_INPUT_WEST))
        {
            return i;
        }
        else if ((type == PLD_GAMEPLAY_INPUT_R) && (*(int*)keyTypes->data[i] == PLD_GAMEPLAY_INPUT_SOUTH))
        {
            return i;
        }
        else if ((type == PLD_GAMEPLAY_INPUT_R) && (*(int*)keyTypes->data[i] == PLD_GAMEPLAY_INPUT_EAST))
        {
            return i;
        }
    }

    return -1;
}

void PLD_KeyPress(PLD_Context* context, PLD_ArrayList* currentNotes, int time, int pressedCode, bool keyboard)
{
    bool isHit = false;
    bool valid = false;
    PLD_ArrayList** keys;
    PLD_ArrayList* keyTypes;

    if (keyboard)
    {
        keys = context->config->gameplay_keys;
    }
    else
    {
        keys = context->config->gameplay_buttons;
    }

    keyTypes = PLD_GetPressedTypes(pressedCode, keys);

    if (keyTypes->len > 0)
    {
        for (int i = 0; i < keyTypes->len; i++)
        {
            int heldType = *(int*)keyTypes->data[i];

            if (PLD_IsHeld(heldType))
            {
                PLD_AddHoldKey(heldType, pressedCode, keyboard);
            }
        }

        for (int i = 0;  i < currentNotes->len; i++)
        {
            PLD_Note* note = PLD_GetNote(currentNotes, i);

            if (note->active)
            {
                int type = note->type;

                //SIMPLIFY NOTE TYPE FROM 0 TO 5

                if (PLD_IsHoldType(type))
                {
                    type -= 4;
                }
                else if (PLD_IsSlideType(type))
                {
                    type -= 8;
                }
                else if (PLD_IsChainType(type))
                {
                    type -= 11;
                }

                if (keyTypes->len > currentNotes->len) //If there's more buttons pressed than number of notes
                {
                    PLD_EvaluateNote(note, time, false);
                }
                else
                {
                    int hitIndex = PLD_TypeInKeyTypes(keyTypes, type);

                    if (hitIndex != -1)
                    {
                        isHit = true;

                        SDL_free(PLD_ArrayListRemoveAt(keyTypes, hitIndex));
                
                        valid = PLD_EvaluateNote(note, time, true);

                        if ((PLD_IsNormalType(type)) && (PLD_IsHeld(type)))
                        {
                            PLD_DropHolds();
                        }
                        
                        if (valid)
                        {
                            if (PLD_IsHoldType(note->type) || (PLD_IsChainType(note->type)))
                            {
                                PLD_AddHoldKey(type, pressedCode, keyboard);

                                if (PLD_IsHoldType(note->type))
                                {
                                    PLD_SetHoldTimeout(time);
                                }
                            }
                        }
                    }
                    else if (currentNotes->len == 1)
                    {
                        PLD_EvaluateNote(note, time, false);
                    }
                }
            }
        }

        if (!isHit)
        {
            if (currentNotes->len > 1)
            {
                for (int i = 0; i < currentNotes->len; i++)
                {
                    PLD_Note* note = PLD_GetNote(currentNotes, i);

                    if (note->active)
                    {
                        PLD_EvaluateNote(note, time, false);
                    }
                }
            }

            PLD_PlaySound("sound", 0, 0);
        }
    }

    PLD_DestroyArrayList(keyTypes, SDL_free);
}
