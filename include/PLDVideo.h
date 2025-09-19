#pragma once
#include "PLD.h"

#include <PLD_context.h>
#include <PLD_image.h>


typedef struct PLD_Video PLD_Video;


int PLD_FreeVideo(PLD_Video* video);

PLD_Video* PLD_LoadVideo(PLD_Context* context, const char* path, int time);

int PLD_UpdateVideoTime(PLD_Video* video, int time);

PLD_Image* PLD_GetVideoFrame(PLD_Video* video);