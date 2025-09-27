#include <UDC/UDC_ppd.h>

#define UDC_PPD_DATA_INI_FILE "data.ini"

char* UDC_GetPPDDataIniPath(const char* directory)
{
    if (!directory) { return false; }

    char* path;
    if (SDL_asprintf(&path, "%s/%s", directory, UDC_PPD_DATA_INI_FILE) > 0)
    {
        if (SDL_GetPathInfo(path, NULL)) { return path; }
        SDL_free(path);
    }

    return NULL;
}

UDC_ChartInfo* UDC_LoadPPDDataIni(const char* path)
{
    return NULL;
}