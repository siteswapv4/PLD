#include <PLD/PLD_note.h>

PLD_Note* PLD_GetNote(PLD_ArrayList* array, int index)
{
    return array->data[index];
}

int PLD_IsNormalType(int type)
{
    return (type == PLD_TYPE_TRIANGLE) || (type == PLD_TYPE_SQUARE) || (type == PLD_TYPE_CROSS) || (type == PLD_TYPE_CIRCLE);
}

int PLD_IsHoldType(int type)
{
    return (type == PLD_TYPE_TRIANGLEHOLD) || (type == PLD_TYPE_SQUAREHOLD) || (type == PLD_TYPE_CROSSHOLD) || (type == PLD_TYPE_CIRCLEHOLD);
}

int PLD_IsSlideType(int type)
{
    return (type == PLD_TYPE_SLIDEL) || (type == PLD_TYPE_SLIDER);
}

int PLD_IsChainType(int type)
{
    return (type == PLD_TYPE_CHAINL) || (type == PLD_TYPE_CHAINR);
}

int PLD_IsChainBitType(int type)
{
    return (type == PLD_TYPE_CHAINLBIT) || (type == PLD_TYPE_CHAINRBIT);
}

void PLD_DisableNote(PLD_Note* note)
{
    PLD_Note* next = note->nextChainBit;
    note->active = false;

    while (next != NULL)
    {
        next->active = false;
        next = next->nextChainBit;
    }
}

void PLD_AddNoteOrdered(PLD_ArrayList* notes, PLD_Note* note)
{
    for (int i = notes->len - 1; i >= 0; i--)
    {
        if (PLD_GetNote(notes, i)->time - PLD_GetNote(notes, i)->flyingTime <= note->time - note->flyingTime)
        {
            PLD_ArrayListAddAt(notes, note, i + 1);
            return;
        }
    }

    PLD_ArrayListAddAt(notes, note, 0);
}