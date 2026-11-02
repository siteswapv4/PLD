#include <PLD/PLD_utils.h>

typedef struct
{
    bool concat;
    bool dir;
    bool file;
    PLD_ArrayList* list;
}PLD_EnumData;

int PLD_Center(SDL_FRect* value)
{
    value->x -= value->w / 2;
    value->y -= value->h / 2;

    return 0;
}

int* PLD_DupInt(int value) 
{
    int* retval = SDL_malloc(sizeof(int));
    *retval = value;

    return retval;
}

bool PLD_StartsWith(const char* source, const char* target)
{
    return SDL_strncmp(source, target, SDL_strlen(target)) == 0;
}

bool PLD_EndsWith(const char* source, const char* target)
{
	int lenSource = SDL_strlen(source);
	int lenTarget = SDL_strlen(target);

	if (lenSource < lenTarget)
		return false;
		
	source = &source[lenSource - lenTarget];
	
	return !SDL_strcmp(source, target);
}

bool PLD_StartsWithInsensitive(const char* source, const char* target)
{
    return SDL_strncasecmp(source, target, SDL_strlen(target)) == 0;
}

bool PLD_EndsWithInsensitive(const char* source, const char* target)
{
	int lenSource = SDL_strlen(source);
	int lenTarget = SDL_strlen(target);

	if (lenSource < lenTarget)
		return false;
		
	source = &source[lenSource - lenTarget];
	
	return !SDL_strcasecmp(source, target);
}

int PLD_ContainsChar(char* list, char c, int cutLen)
{
    int i;

    for (i=0; i<cutLen; i++)
    {
        if (list[i] == c)
        {
            return 1;
        }
    }

    return 0;
}

int PLD_SkipUselessChar(SDL_IOStream* file, char* cut, int cutLen)
{
    char c;
    size_t ret;

    if (file != NULL)
    {
        ret = SDL_ReadIO(file, &c, sizeof(char));
        
        while ((ret != 0) && (PLD_ContainsChar(cut, c, cutLen)))
        {
            ret = SDL_ReadIO(file, &c, sizeof(char));
        }

        if (ret == 0)
        {
            return 1;
        }
            
        SDL_SeekIO(file, -1, SDL_IO_SEEK_CUR);

        return 0;
    }

    return 1;
}

int PLD_ExtractSequence(SDL_IOStream* file, char** string, char* cut, int cutLen)
{
    int len = 0, startPos;
    char c;
    size_t ret;

    if (PLD_SkipUselessChar(file, cut, cutLen) != 0)
    {
        return 1;
    }

    startPos = SDL_TellIO(file);

    if (*string != NULL)
    {
        SDL_free(*string);
    }

    ret = SDL_ReadIO(file, &c, sizeof(char));
    while ((ret != 0) && (!PLD_ContainsChar(cut, c, cutLen)))
    {
        ret = SDL_ReadIO(file, &c, sizeof(char));
        len++;
    }

    *string = SDL_calloc(len + 1, sizeof(char));
    SDL_SeekIO(file, startPos, SDL_IO_SEEK_SET);

    SDL_ReadIO(file, *string, sizeof(char) * len);

    SDL_SeekIO(file, 1L, SDL_IO_SEEK_CUR);

    return 0;
}


SDL_EnumerationResult PLD_EnumCallback(void* userdata, const char* dirname, const char* fname)
{
    PLD_EnumData* data = userdata;
    SDL_PathInfo info;
    char* fullPath;

    SDL_asprintf(&fullPath, "%s%s", dirname, fname);

    SDL_GetPathInfo(fullPath, &info);

    if (((info.type == SDL_PATHTYPE_DIRECTORY) && (data->dir)) || ((info.type == SDL_PATHTYPE_FILE) && (data->file)))
    {
        if (data->concat)
        {
            PLD_ArrayListAdd(data->list, SDL_strdup(fullPath));
        }
        else
        {
            PLD_ArrayListAdd(data->list, SDL_strdup(fname));
        }
    }

    SDL_free(fullPath);

    return SDL_ENUM_CONTINUE;
}


PLD_ArrayList* PLD_GetDirectoryContent(const char* path, bool onlyFiles, bool onlyDirs, bool concatPath)
{
	PLD_EnumData data = {0};

    data.file = onlyFiles;
    data.dir = onlyDirs;
    data.concat = concatPath;
	data.list = PLD_CreateArrayList();

    if (!SDL_EnumerateDirectory(path, PLD_EnumCallback, &data))
    {
        SDL_Log("Failed to enumerate '%s' : '%s'", path, SDL_GetError());
    }
    
    return data.list;
}

char* PLD_GetLastOccurenceInString(const char* source, size_t length, char* targets)
{
    for (int i = length ? length - 1 : SDL_strlen(source) - 1; i >= 0; i--)
    {
        if (SDL_strchr(targets, source[i]))
        {
            return (char*)&source[i];
        }
    }

    return NULL;
}

bool PLD_RemoveExtension(char* source)
{
    char* last_dot = PLD_GetLastOccurenceInString(source, SDL_strlen(source), ".");
    if (last_dot)
    {
        *last_dot = '\0';
        return true;
    }

    return false;
}

bool PLD_ToLower(char* source)
{
    for (char* it = source; *it; it++)
    {
        *it = SDL_tolower(*it);
    }
    
    return true;
}

char* PLD_KeepFilename(char* source)
{
    size_t length = SDL_strlen(source);
    char* last_dot = PLD_GetLastOccurenceInString(source, length, ".");
    char* last_slash = PLD_GetLastOccurenceInString(source, length, "\\/");

    if (last_dot) { *last_dot = '\0'; }
    
    return last_slash ? last_slash : source;
}

char* PLD_asprintf(const char* fmt, ...)
{
    char* res;
    va_list ap;
    va_start(ap, fmt);
    int retval = SDL_vasprintf(&res, fmt, ap);
    va_end(ap);

    return retval >= 0 ? res : NULL;
}

typedef struct PLD_OpenFolderData
{
    void (*callback)(void* userdata, const char* directory);
    void* userdata;
    char* directory;
}PLD_OpenFolderData;

void PLD_MainThreadOpenFolderCallback(void* userdata)
{
    PLD_OpenFolderData* data = userdata;
    data->callback(data->userdata, data->directory);
    SDL_free(data->directory);
    SDL_free(data);
}

void PLD_OpenFolderCallback(void* userdata, const char* const* files, int filter)
{
    PLD_OpenFolderData* data = userdata;

    if ((files) && (files[0]))
    {
        data->directory = SDL_strdup(files[0]);
        SDL_RunOnMainThread(PLD_MainThreadOpenFolderCallback, data, false);
    }
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

static void (*g_callback)(void* userdata, const char* directory) = NULL;
static void* g_userdata = NULL;

EMSCRIPTEN_KEEPALIVE
void PLD_CallDirectoryCallback(char* dir)
{
    if (g_callback)
    {
        g_callback(g_userdata, dir);
    }
}
#endif

bool PLD_ShowDirectoryPicker(void (*callback)(void* userdata, const char* directory), void* userdata)
{
#ifdef __EMSCRIPTEN__
    g_callback = callback;
    g_userdata = userdata;

    EM_ASM
    ({
        var input = document.createElement('input');
        input.type = 'file';
        input.webkitdirectory = true;
        input.multiple = true;
        input.style.display = 'none';
        document.body.appendChild(input);

        input.onchange = async function(ev)
        {
            var files = ev.target.files;
            if (files.length === 0)
            {
                document.body.removeChild(input);
                return;
            }
            var mountPoint = "/picked";
            var rootDirName = files[0].webkitRelativePath.split("/")[0];
            var fullMountPath = mountPoint + "/" + rootDirName;
            try { FS.mkdir(mountPoint); } catch(e) {}
            for (var i = 0; i < files.length; i++)
            {
                let file = files[i];
                let relPath = file.webkitRelativePath || file.name;
                let fullPath = mountPoint + "/" + relPath;
                let parts = relPath.split("/");
                let dirPath = mountPoint;
                for (let j = 0; j < parts.length - 1; j++)
                {
                    dirPath += "/" + parts[j];
                    try { FS.mkdir(dirPath); } catch(e) {}
                }
                let data = new Uint8Array(await file.arrayBuffer());
                FS.writeFile(fullPath, data);
            }
            var sp = _malloc(fullMountPath.length + 1);
            stringToUTF8(fullMountPath, sp, fullMountPath.length + 1);
            Module.ccall("PLD_CallDirectoryCallback", "void", ["number"], [sp]);
            _free(sp);
            document.body.removeChild(input);
        };
        input.click();
    });

    return true;
#else
    PLD_OpenFolderData* data = SDL_malloc(sizeof(PLD_OpenFolderData));
    data->callback = callback;
    data->userdata = userdata;
    SDL_ShowOpenFolderDialog(PLD_OpenFolderCallback, data, NULL, NULL, false);

    return true;
#endif
}

float PLD_GetScrollFromDuration(Uint64 duration)
{
    return (SDL_GetTicks() % duration) / (duration / 2.0f) - 0.5f;
}

SDL_EnumerationResult PLD_FilePresentCallback(void* userdata, const char* dirname, const char* fname)
{
    bool* empty = userdata;

    if (SDL_strcmp(fname, ".") == 0 || SDL_strcmp(fname, "..") == 0)
    {
        return SDL_ENUM_CONTINUE;
    }

    *empty = false;
    return SDL_ENUM_SUCCESS;
}

bool PLD_DirectoryIsEmpty(const char* path)
{
    bool empty = true;

    if (!SDL_EnumerateDirectory(path, PLD_FilePresentCallback, &empty))
    {
        return false;
    }

    return empty;
}

bool PLD_CopyDirectory(const char* source, const char* destination, bool replace_files)
{
    char* source_path = NULL;
    char* destination_path = NULL;
    char** files = NULL;
    int num_files = 0;

    if (!SDL_CreateDirectory(destination))
    {
        SDL_Log("Failed to create directory : '%s'", SDL_GetError());
        goto error;
    }

    files = SDL_GlobDirectory(source, NULL, 0, &num_files);
    if (!files)
    {
        SDL_Log("Failed to glob directory : '%s'", SDL_GetError());
        goto error;
    }

    SDL_Log("%s : %d", source, num_files);

    for (int i = 0; i < num_files; i++)
    {
        SDL_asprintf(&source_path, "%s/%s", source, files[i]);
        SDL_asprintf(&destination_path, "%s/%s", destination, files[i]);

        if (!SDL_GetPathInfo(destination_path, NULL) || replace_files)
        {
            SDL_PathInfo info;
            if (!SDL_GetPathInfo(source_path, &info))
            {
                SDL_Log("Failed to get path info : '%s'", SDL_GetError());
                goto error;
            }

            if (info.type == SDL_PATHTYPE_FILE)
            {
                if (!SDL_CopyFile(source_path, destination_path))
                {
                    SDL_Log("Failed to copy file : '%s'", SDL_GetError());
                    goto error;
                }
            }
            else
            {
                if (!SDL_CreateDirectory(destination_path))
                {
                    SDL_Log("Failed to create directory : '%s'", SDL_GetError());
                    goto error;
                }
            }
        }

        SDL_free(source_path);
        SDL_free(destination_path);
        source_path = NULL;
        destination_path = NULL;
    }

    SDL_free(files);
    return true;

error:
    if (files) SDL_free(files);
    if (source_path) SDL_free(source_path);
    if (destination_path) SDL_free(destination_path);

    return false;
}

char PLD_PeekByteFromStream(SDL_IOStream* stream)
{
    char c = 0;
    SDL_ReadIO(stream, &c, 1);
    SDL_SeekIO(stream, -1, SDL_IO_SEEK_CUR);
    return c;
}

bool PLD_FileExists(const char* path)
{
    SDL_PathInfo info;
    return SDL_GetPathInfo(path, &info);
}

int PLD_CountDigits(int number)
{
    if (number == 0) return 1;
    
    if (number < 0)
        number *= 1;

    int count = 0;
    while (number != 0)
    {
        count++;
        number /= 10;
    }

    return count;
}