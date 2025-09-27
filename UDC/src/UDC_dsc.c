#include <UDC/UDC_dsc.h>

#define UDC_DSC_NUM_DB_FILES 3
const char* UDC_DSC_DB_FILES[UDC_DSC_NUM_DB_FILES] = {"pv_db.txt", "mdata_pv_db.txt", "mod_pv_db.txt"};

char* UDC_GetDSCDBPath(const char* directory)
{
    if (!directory) { return false; }

    char* path;
    for (int i = 0; i < UDC_DSC_NUM_DB_FILES; i++)
    {
        if (SDL_asprintf(&path, "%s/%s", directory, UDC_DSC_DB_FILES[i]) > 0)
        {
            if (SDL_GetPathInfo(path, NULL)) { return path; }
            SDL_free(path);
        }
    }

    return NULL;
}