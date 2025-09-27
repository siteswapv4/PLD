#include <UDC/UDC_phb.h>

#define UDC_PHB_SONG_INFO_FILE "song.json"

char* UDC_GetPHBSongInfoPath(const char* directory)
{
    if (!directory) { return false; }

    char* path;
    if (SDL_asprintf(&path, "%s/%s", directory, UDC_PHB_SONG_INFO_FILE) > 0)
    {
        if (SDL_GetPathInfo(path, NULL)) { return path; }
        SDL_free(path);
    }

    return NULL;
}

UDC_ChartInfo* UDC_LoadPHBSongInfo(const char* path)
{
    return NULL;
}