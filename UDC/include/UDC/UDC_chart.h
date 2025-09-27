#pragma once
#include <SDL3/SDL.h>

typedef struct UDC_ChartInfo
{
    char* author;
    char* thumb;
    char* audio;
    char* video;
    char* difficulty;

    Sint32 start_time;
    Sint32 end_time;

    Sint32 preview_start;
    Sint32 preview_end;
}UDC_ChartInfo;

void UDC_DestroyChartInfo(UDC_ChartInfo* info);