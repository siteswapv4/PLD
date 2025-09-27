#pragma once
#include <SDL3/SDL.h>
#include <UDC/UDC_chart.h>

char* UDC_GetDataIniPath(const char* directory);

UDC_ChartInfo* UDC_LoadPPDDataIni(const char* path);