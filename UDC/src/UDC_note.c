#include <UDC/UDC_note.h>

bool UDC_IsBase(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type >= UDC_NOTE_TRIANGLE) && (note->type <= UDC_NOTE_STAR);
}

bool UDC_IsHold(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type >= UDC_NOTE_TRIANGLE_HOLD) && (note->type <= UDC_NOTE_CIRCLE_HOLD);
}

bool UDC_IsSustain(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type >= UDC_NOTE_TRIANGLE_SUSTAIN) && (note->type <= UDC_NOTE_STAR_SUSTAIN);
}

bool UDC_IsDouble(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type >= UDC_NOTE_TRIANGLE_DOUBLE) && (note->type <= UDC_NOTE_STAR_DOUBLE);
}

bool UDC_IsRush(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type >= UDC_NOTE_TRIANGLE_RUSH) && (note->type <= UDC_NOTE_STAR_RUSH);
}

bool UDC_IsSlide(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type == UDC_NOTE_SLIDE_L) || (note->type == UDC_NOTE_SLIDE_R);
}

bool UDC_IsChain(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type == UDC_NOTE_CHAIN_L) || (note->type == UDC_NOTE_CHAIN_R);
}

bool UDC_IsSustainStart(UDC_Note* note)
{
    if (!note) { return false; }
    return (UDC_IsSustain(note)) && (!note->previous);
}

bool UDC_IsSustainEnd(UDC_Note* note)
{
    if (!note) { return false; }
    return (UDC_IsSustain(note)) && (!note->next);
}

bool UDC_IsChainStart(UDC_Note* note)
{
    if (!note) { return false; }
    return (UDC_IsChain(note)) && (!note->previous);
}

bool UDC_IsChainBit(UDC_Note* note)
{
    if (!note) { return false; }
    return (UDC_IsChain(note)) && (note->previous) && (note->next);
}

bool UDC_IsChainEnd(UDC_Note* note)
{
    if (!note) { return false; }
    return (UDC_IsChain(note)) && (!note->next);
}

bool UDC_IsLinkStart(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type == UDC_NOTE_STAR_LINK) && (!note->previous);
}

bool UDC_IsLinkBit(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type == UDC_NOTE_STAR_LINK) && (note->previous) && (note->next);
}

bool UDC_IsLinkEnd(UDC_Note* note)
{
    if (!note) { return false; }
    return (note->type == UDC_NOTE_STAR_LINK) && (!note->next);
}

bool UDC_IsMulti(UDC_Note* notes, int num_notes, int index)
{
    if (!notes) { return false; }

    return ((index > 0)             && (notes[index - 1].time == notes[index].time)) ||
           ((index < num_notes - 1) && (notes[index + 1].time == notes[index].time));
}