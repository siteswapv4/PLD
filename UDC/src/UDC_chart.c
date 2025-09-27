#include <UDC/UDC_chart.h>

void UDC_DestroyChartInfo(UDC_ChartInfo* info)
{
    if (info)
    {
        if (info->author)     { SDL_free(info->author); }
        if (info->thumb)      { SDL_free(info->thumb); }
        if (info->audio)      { SDL_free(info->audio); }
        if (info->video)      { SDL_free(info->audio); }
        if (info->difficulty) { SDL_free(info->difficulty); }
        SDL_free(info);
    }
}