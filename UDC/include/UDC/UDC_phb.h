#pragma once
#include <SDL3/SDL.h>
#include <UDC/UDC_chart.h>

char* UDC_GetPHBSongInfoPath(const char* directory);

UDC_ChartInfo* UDC_LoadPHBSongInfo(const char* path);