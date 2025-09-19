#pragma once
#include "PLD.h"
#include "PLD_image.h"
#include "PLDNotes.h"
#include "PLDUtils.h"


int PLD_AssignNotesImages(PLD_ArrayList* notes);


int PLD_PPDLinkChainslide(PLD_ArrayList* notes);


int PLD_LinkMulti(PLD_Note* multi[4], int count);


int PLD_LoadPPDChart(PLD_Song* songInfo);
