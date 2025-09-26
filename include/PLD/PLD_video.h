#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_context.h>
#include <PLD/PLD_image.h>

typedef struct PLD_Video PLD_Video;

bool PLD_IsSupportedVideo(const char* path);

int PLD_FreeVideo(PLD_Video* video);

PLD_Video* PLD_LoadVideo(PLD_Context* context, const char* path, int time);

int PLD_UpdateVideoTime(PLD_Video* video, int time);

PLD_Image* PLD_GetVideoFrame(PLD_Video* video);