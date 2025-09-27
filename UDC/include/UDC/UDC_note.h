#pragma once
#include <SDL3/SDL.h>

typedef enum UDC_NoteType
{
    UDC_NOTE_INVALID = -1,

    UDC_NOTE_TRIANGLE,
    UDC_NOTE_SQUARE,
    UDC_NOTE_CROSS,
    UDC_NOTE_CIRCLE,
    UDC_NOTE_STAR,

    UDC_NOTE_TRIANGLE_HOLD,
    UDC_NOTE_SQUARE_HOLD,
    UDC_NOTE_CROSS_HOLD,
    UDC_NOTE_CIRCLE_HOLD,

    UDC_NOTE_TRIANGLE_SUSTAIN,
    UDC_NOTE_SQUARE_SUSTAIN,
    UDC_NOTE_CROSS_SUSTAIN,
    UDC_NOTE_CIRCLE_SUSTAIN,
    UDC_NOTE_STAR_SUSTAIN,

    UDC_NOTE_TRIANGLE_DOUBLE,
    UDC_NOTE_SQUARE_DOUBLE,
    UDC_NOTE_CROSS_DOUBLE,
    UDC_NOTE_CIRCLE_DOUBLE,
    UDC_NOTE_STAR_DOUBLE,

    UDC_NOTE_TRIANGLE_RUSH,
    UDC_NOTE_SQUARE_RUSH,
    UDC_NOTE_CROSS_RUSH,
    UDC_NOTE_CIRCLE_RUSH,
    UDC_NOTE_STAR_RUSH,

    UDC_NOTE_SLIDE_L,
    UDC_NOTE_SLIDE_R,

    UDC_NOTE_CHAIN_L,
    UDC_NOTE_CHAIN_R,

    UDC_NOTE_STAR_LINK,

    UDC_NOTE_COUNT
}UDC_NoteType;

typedef struct UDC_Note
{
    UDC_NoteType type;
    Sint32 time;
    Sint32 flying_time;
    Sint32 duration;
    Sint32 position_x;
    Sint32 position_y;
    Sint32 angle;
    Sint32 distance;
    Sint32 frequency;
    Sint32 amplitude;
    
    struct UDC_Note* next;
    struct UDC_Note* previous;
}UDC_Note;

bool UDC_IsBase(UDC_Note* note);

bool UDC_IsHold(UDC_Note* note);

bool UDC_IsSustain(UDC_Note* note);

bool UDC_IsDouble(UDC_Note* note);

bool UDC_IsRush(UDC_Note* note);

bool UDC_IsSlide(UDC_Note* note);

bool UDC_IsChain(UDC_Note* note);

bool UDC_IsSustainStart(UDC_Note* note);

bool UDC_IsSustainEnd(UDC_Note* note);

bool UDC_IsChainStart(UDC_Note* note);

bool UDC_IsChainBit(UDC_Note* note);

bool UDC_IsChainEnd(UDC_Note* note);

bool UDC_IsLinkStart(UDC_Note* note);

bool UDC_IsLinkBit(UDC_Note* note);

bool UDC_IsLinkEnd(UDC_Note* note);

bool UDC_IsMulti(UDC_Note* notes, int num_notes, int index);