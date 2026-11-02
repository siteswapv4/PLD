#include <PLD/PLD_user.h>
#include <PLD/PLD_utils.h>
#include <md5/md5.h>

#define PLD_USER_DATA_HASH_SIZE 32
#define PLD_USER_DATA_ELEMENT_COUNT 12
#define PLD_USER_DATA_ELEMENT_SIZE 4 //sizeof(Uint32)

#define PLD_USER_DATA_SIZE (PLD_USER_DATA_HASH_SIZE + PLD_USER_DATA_ELEMENT_COUNT * PLD_USER_DATA_ELEMENT_SIZE) // in bytes

typedef struct PLD_UserDataInternal
{
    Uint32 number;
    PLD_UserData data;
}PLD_UserDataInternal;

static Uint32 PLD_num_user_data_entries = 0;
static SDL_PropertiesID PLD_user_database = 0;
static SDL_IOStream* PLD_user_datastream = NULL;

void PLD_FreeUserDataInternal(void* userdata, void* value)
{
    SDL_free(value);
}

bool PLD_InitUserData(PLD_Context* context)
{
    if (PLD_user_database) return true;

    PLD_UserDataInternal* data = NULL;
    PLD_num_user_data_entries = 0;

    PLD_user_database = SDL_CreateProperties();
    if (!PLD_user_database) goto error;

    char* path = PLD_asprintf("%suserdata.bin", context->data_path);
    PLD_user_datastream = SDL_IOFromFile(path, "rb+");
    if (!PLD_user_datastream)
    {
        PLD_user_datastream = SDL_IOFromFile(path, "wb+");
        SDL_free(path);
        if (!PLD_user_datastream) goto error;
        PLD_WRITE_CHECK(PLD_user_datastream, &PLD_num_user_data_entries, sizeof(Uint32));
        SDL_FlushIO(PLD_user_datastream);
    }
    else if (SDL_ReadIO(PLD_user_datastream, &PLD_num_user_data_entries, sizeof(Uint32)) == sizeof(Uint32))
    {
        SDL_free(path);

        for (Uint32 i = 0; i < PLD_num_user_data_entries; i++)
        {
            data = SDL_calloc(1, sizeof(PLD_UserDataInternal));
            char hash[PLD_USER_DATA_HASH_SIZE + 1] = {0};

            PLD_READ_CHECK(PLD_user_datastream, hash, PLD_USER_DATA_HASH_SIZE);
            PLD_READ_CHECK(PLD_user_datastream, &data->data.cool_count, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.fine_count, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.safe_count, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.sad_count, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.worst_count, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.combo, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.score, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.percentage, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.num_tries, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.grade, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.accuracy, sizeof(Uint32));
            PLD_READ_CHECK(PLD_user_datastream, &data->data.offset, sizeof(Uint32));

            data->number = i;

            if (!SDL_SetPointerPropertyWithCleanup(PLD_user_database, hash, data, PLD_FreeUserDataInternal, NULL))
                goto error;
            data = NULL;
        }
    }
    else
    {
        SDL_free(path);
        PLD_num_user_data_entries = 0;
        if (SDL_SeekIO(PLD_user_datastream, 0, SDL_IO_SEEK_SET) < 0) goto error;
        PLD_WRITE_CHECK(PLD_user_datastream, &PLD_num_user_data_entries, sizeof(Uint32));
        SDL_FlushIO(PLD_user_datastream);
    }

    return true;

error:
    SDL_Log("Failed to init user data : '%s'", SDL_GetError());

    if (data) SDL_free(data);

    PLD_QuitUserData();
    return false;
}

void PLD_QuitUserData()
{
    if (PLD_user_database)
    {
        SDL_DestroyProperties(PLD_user_database);
        PLD_user_database = 0;
    }
    if (PLD_user_datastream)
    {
        SDL_CloseIO(PLD_user_datastream);
        PLD_user_datastream = NULL;
    }
}

bool PLD_ChartToHash(const char* path, char hash[PLD_USER_DATA_HASH_SIZE])
{
    size_t size = 0;
    Uint8* file = SDL_LoadFile(path, &size);
    if (!file) return false;

    MD5Context ctx;
    md5Init(&ctx);
    md5Update(&ctx, file, size);
    md5Finalize(&ctx);
    SDL_free(file);

    static const char hex[] = "0123456789abcdef";

    for (int i = 0; i < 16; i++)
    {
        hash[i * 2]     = hex[(ctx.digest[i] >> 4) & 0xF];
        hash[i * 2 + 1] = hex[ctx.digest[i] & 0xF];
    }

    return true;
}

bool PLD_GetUserData(const char* chart_path, PLD_UserData* out_data)
{
    if (!out_data) return false;

    char hash[PLD_USER_DATA_HASH_SIZE + 1] = {0};
    if (!PLD_ChartToHash(chart_path, hash)) goto error;

    PLD_UserDataInternal* data = SDL_GetPointerProperty(PLD_user_database, hash, NULL);
    if (!data) return false;

    *out_data = data->data;
    return true;

error:
    SDL_Log("Failed to get user data");
    return false;
}

bool PLD_WriteUserData(const char hash[PLD_USER_DATA_HASH_SIZE], const PLD_UserDataInternal* data)
{
    if (SDL_SeekIO(PLD_user_datastream, data->number * PLD_USER_DATA_SIZE + sizeof(Uint32), SDL_IO_SEEK_SET) < 0) goto error;
    PLD_WRITE_CHECK(PLD_user_datastream, hash, PLD_USER_DATA_HASH_SIZE);
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.cool_count, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.fine_count, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.safe_count, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.sad_count, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.worst_count, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.combo, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.score, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.percentage, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.num_tries, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.grade, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.accuracy, sizeof(Uint32));
    PLD_WRITE_CHECK(PLD_user_datastream, &data->data.offset, sizeof(Uint32));

    if (SDL_SeekIO(PLD_user_datastream, 0, SDL_IO_SEEK_SET) < 0) goto error;
    PLD_WRITE_CHECK(PLD_user_datastream, &PLD_num_user_data_entries, sizeof(Uint32));

    SDL_FlushIO(PLD_user_datastream);

    return true;

error:
    SDL_Log("Failed to write user data");
    return false;
}

bool PLD_SaveUserData(const char* chart_path, const PLD_UserData* in_data)
{
    if (!in_data) return false;

    char hash[PLD_USER_DATA_HASH_SIZE + 1] = {0};
    if (!PLD_ChartToHash(chart_path, hash)) goto error;

    PLD_UserDataInternal* existing_data = SDL_GetPointerProperty(PLD_user_database, hash, NULL);
    if (existing_data)
    {
        if (in_data->score > existing_data->data.score)
        {
            existing_data->data.cool_count = in_data->cool_count;
            existing_data->data.fine_count = in_data->fine_count;
            existing_data->data.safe_count = in_data->safe_count;
            existing_data->data.sad_count = in_data->sad_count;
            existing_data->data.worst_count = in_data->worst_count;
            existing_data->data.combo = in_data->combo;
            existing_data->data.score = in_data->score;
            existing_data->data.percentage = in_data->percentage;
            existing_data->data.grade = in_data->grade;
            existing_data->data.accuracy = in_data->accuracy;
        }
        existing_data->data.num_tries = in_data->num_tries;
        existing_data->data.offset = in_data->offset;

        if (!PLD_WriteUserData(hash, existing_data)) goto error;
    }
    else
    {
        existing_data = SDL_calloc(1, sizeof(PLD_UserDataInternal));
        existing_data->data = *in_data;
        existing_data->number = PLD_num_user_data_entries;

        if (!SDL_SetPointerPropertyWithCleanup(PLD_user_database, hash, existing_data, PLD_FreeUserDataInternal, NULL))
        {
            SDL_free(existing_data);
            goto error;
        }

        PLD_num_user_data_entries++;

        if (!PLD_WriteUserData(hash, existing_data))
        {
            SDL_ClearProperty(PLD_user_database, hash);
            PLD_num_user_data_entries--;
            goto error;
        }
    }

    return true;

error:
    SDL_Log("Failed to save user data");
    return false;
}