#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_context.h>

#define PLD_TYPE_MAX 17
#define PLD_TYPE_COUNT 17
#define PLD_TYPE_TRIANGLE 0
#define PLD_TYPE_SQUARE 1
#define PLD_TYPE_CROSS 2
#define PLD_TYPE_CIRCLE 3
#define PLD_TYPE_TRIANGLEHOLD 4
#define PLD_TYPE_SQUAREHOLD 5
#define PLD_TYPE_CROSSHOLD 6
#define PLD_TYPE_CIRCLEHOLD 7
#define PLD_TYPE_CHAINLBIT 10
#define PLD_TYPE_CHAINRBIT 11
#define PLD_TYPE_SLIDEL 12
#define PLD_TYPE_SLIDER 13
#define PLD_TYPE_CHAINL 15
#define PLD_TYPE_CHAINR 16

typedef struct PLD_Note
{
    int ID;
    int targetX;
    int targetY;
    int buttonX;
    int buttonY;
    int time;
    int rtime;
    int flyingTime;
    int angle;
    int distance;
    int amplitude;
    int frequency;
    int type;
    bool active;
    bool current;
    bool isMulti;
    PLD_ImageName targetName;
    PLD_ImageName buttonName;
    struct PLD_Note* connect;
    struct PLD_Note* nextChainBit;
}PLD_Note;

int PLD_IsNormalType(int type);

int PLD_IsHoldType(int type);

int PLD_IsSlideType(int type);

int PLD_IsChainType(int type);

int PLD_IsChainBitType(int type);

PLD_Note* PLD_GetNote(PLD_ArrayList* array, int index);

void PLD_DisableNote(PLD_Note* note);

void PLD_AddNoteOrdered(PLD_ArrayList* noteArray, PLD_Note* note);
