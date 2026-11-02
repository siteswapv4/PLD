#include <PLD/PLD_image.h>
#include <PLD/PLD_utils.h>

#define PLD_IMAGE_EXTENSION ".png"
#define PLD_IMAGE_DIRECTORY "images/"

const char* PLD_IMAGES_CATEGORIES_STRINGS[PLD_IMAGE_CATEGORY_COUNT] = {
    "gameplay",
    "menu",
    "result"
};

const char* PLD_IMAGES_STRINGS[PLD_IMAGE_COUNT] = {    
    "0B",
    "0BM",
    "0T",
    "0TM",
    "1B",
    "1BM",
    "1T",
    "1TM",
    "2B",
    "2BM",
    "2T",
    "2TM",
    "3B",
    "3BM",
    "3T",
    "3TM",
    "12B",
    "12BM",
    "12T",
    "12TM",
    "13B",
    "13BM",
    "13T",
    "13TM",
    "15B",
    "15BM",
    "15T",
    "15TM",
    "16B",
    "16BM",
    "16T",
    "16TM",
    "H",
    "HM",
    "JIKU",
    "worst",
    "sad",
    "safe",
    "fine",
    "cool",
    "appear",
    "hit",
    "connect",
    "num",
    "foreground_top",
    "foreground_bottom",
    "pause_background",
    "hold_bar",
    "max_slide_text",
    "max_slide_glow",

    "song_menu_title",
    "song_menu_bottom",
    "song_bar",
    "song_info",
    "background",
    "logo",
    "cheap_icon",
    "standard_icon",
    "great_icon",
    "excellent_icon",
    "perfect_icon",

    "background",
    "dashboard",
    "highscore"
};


static PLD_Image* PLD_images[PLD_IMAGE_COUNT] = {0};


void PLD_GetImagesBounds(PLD_ImageCategory category, int* start, int* end)
{
    switch (category)
    {
        case PLD_IMAGE_CATEGORY_GAMEPLAY:
            *start = PLD_GAMEPLAY_IMAGE_0B;
            *end = PLD_MENU_IMAGE_SONG_MENU_TITLE;
            break;

        case PLD_IMAGE_CATEGORY_MENU:
            *start = PLD_MENU_IMAGE_SONG_MENU_TITLE;
            *end = PLD_RESULT_IMAGE_BACKGROUND;
            break;

        case PLD_IMAGE_CATEGORY_RESULT:
            *start = PLD_RESULT_IMAGE_BACKGROUND;
            *end = PLD_IMAGE_COUNT;
            break;

        default:
            *start = 0;
            *end = 0;
            break;
    }
}

bool PLD_IsSupportedImage(const char* path)
{
    return (PLD_EndsWithInsensitive(path, ".bmp")) ||
           (PLD_EndsWithInsensitive(path, ".gif")) ||
           (PLD_EndsWithInsensitive(path, ".jpg")) ||
           (PLD_EndsWithInsensitive(path, ".jpeg")) ||
           (PLD_EndsWithInsensitive(path, ".png")) ||
           (PLD_EndsWithInsensitive(path, ".webp"));
}

bool PLD_LoadImages(PLD_Context* context, PLD_ImageCategory category)
{
    // Loop from start to end-1
    int start_index = 0;
    int end_index = 0;

    PLD_GetImagesBounds(category, &start_index, &end_index);

    if ((start_index == 0) && (end_index == 0))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid image category");
        return false;
    }

    for (int i = start_index; i < end_index; i++)
    {
        char* image_path;
        SDL_asprintf(&image_path, "%s%s%s/%s%s", context->data_path, PLD_IMAGE_DIRECTORY, PLD_IMAGES_CATEGORIES_STRINGS[category], PLD_IMAGES_STRINGS[i], PLD_IMAGE_EXTENSION);

        PLD_images[i] = PLD_LoadImage(context, image_path);
        SDL_free(image_path);

        if (PLD_images[i] == NULL)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            PLD_DestroyImages(category);

            return false;
        }
    }

    return true;
}


void PLD_DestroyImages(PLD_ImageCategory category)
{
    int start_index = 0;
    int end_index = 0;

    PLD_GetImagesBounds(category, &start_index, &end_index);

    if ((start_index == 0) && (end_index == 0))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid image category");
        return;
    }

    for (int i = start_index; i < end_index; i++)
    {
        if (PLD_images[i] != NULL)
        {
            PLD_DestroyImage(PLD_images[i]);
            PLD_images[i] = NULL;
        }
    }
}


PLD_Image* PLD_GetImage(PLD_ImageName name)
{
    PLD_Image* image = PLD_images[name];

    if (image == NULL)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Requested non initialized image");
    }

    return image;
}

PLD_Image* PLD_CreateImage(PLD_Context* context, int width, int height)
{
    PLD_Image* image = SDL_calloc(1, sizeof(PLD_Image));

    float image_scale = (float)PLD_LOGICAL_WIDTH / context->config->image_width;

    image->texture = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width / image_scale, height / image_scale);
    if (!image->texture)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture");
        SDL_free(image);
        return NULL;
    }

    image->width = width;
    image->height = height;

    return image;
}

bool PLD_SetRenderTarget(PLD_Context* context, PLD_Image* image)
{
    if (!image)
        return SDL_SetRenderTarget(context->renderer, NULL);

    return SDL_SetRenderTarget(context->renderer, image->texture);
}

PLD_Image* PLD_CreateImageFromTexture(PLD_Context* context, SDL_Texture* texture)
{
    PLD_Image* image = SDL_calloc(1, sizeof(PLD_Image));

    image->texture = texture;
    
    image->width = image->texture->w * ((float)PLD_LOGICAL_WIDTH / (float)context->config->image_width);
    image->height = image->texture->h * ((float)PLD_LOGICAL_HEIGHT / (float)context->config->image_height);

    return image;
}


PLD_Image* PLD_LoadImage(PLD_Context* context, const char* path)
{
    SDL_Texture* texture = IMG_LoadTexture(context->renderer, path);
    if (texture == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s from '%s'", SDL_GetError(), path);

        return NULL;
    }

    return PLD_CreateImageFromTexture(context, texture);
}


void PLD_DestroyImage(PLD_Image* image)
{
    if (image != NULL)
    {
        if (image->texture != NULL)
        {
            SDL_DestroyTexture(image->texture);
        }

        SDL_free(image);
    }
}

bool PLD_SetImageColorMod(PLD_Image* image, Uint8 r, Uint8 g, Uint8 b)
{
    return SDL_SetTextureColorMod(image->texture, r, g, b);
}

bool PLD_RenderImage(PLD_Context* context, PLD_Image* image, SDL_FRect* src_rect, SDL_FRect* dst_rect, bool center, double rotation, SDL_FPoint* rotation_center, int alpha)
{
    float width_coeff = ((float)context->config->image_width / PLD_LOGICAL_WIDTH);
    float height_coeff = ((float)context->config->image_height / PLD_LOGICAL_HEIGHT);
    SDL_FRect render_src_rect;
    SDL_FRect render_dst_rect;

    if (src_rect)
    {
        render_src_rect.x = src_rect->x * width_coeff;
        render_src_rect.y = src_rect->y * height_coeff;
        render_src_rect.w = src_rect->w * width_coeff;
        render_src_rect.h = src_rect->h * height_coeff;
    }

    if (dst_rect)
    {
        render_dst_rect = *dst_rect;
    }
    
    SDL_SetTextureAlphaMod(image->texture, alpha);

    if ((dst_rect != NULL) && (center))
    {
        render_dst_rect.x -= render_dst_rect.w / 2;
        render_dst_rect.y -= render_dst_rect.h / 2;
    }

    if (dst_rect)
    {
        if (!SDL_GetRenderTarget(context->renderer))
        {
            PLD_ScaleRect(&render_dst_rect);
        }
        else
        {
            render_dst_rect.x *= width_coeff;
            render_dst_rect.y *= height_coeff;
            render_dst_rect.w *= width_coeff;
            render_dst_rect.h *= height_coeff;
        }
    }

    if (rotation != 0)
    {
        if (rotation_center != NULL)
        {
            SDL_FPoint copy = *rotation_center;
            PLD_ScaleCenterPoint(&copy);
            return SDL_RenderTextureRotated(context->renderer, image->texture, src_rect != NULL ? &render_src_rect : NULL, dst_rect != NULL ? &render_dst_rect : NULL, rotation, &copy, SDL_FLIP_NONE);
        }
        else
        {
            return SDL_RenderTextureRotated(context->renderer, image->texture, src_rect != NULL ? &render_src_rect : NULL, dst_rect != NULL ? &render_dst_rect : NULL, rotation, NULL, SDL_FLIP_NONE);
        }
    }
    else
    {
        return SDL_RenderTexture(context->renderer, image->texture, src_rect != NULL ? &render_src_rect : NULL, dst_rect != NULL ? &render_dst_rect : NULL);
    }
}


bool PLD_RenderImageLetterbox(PLD_Context* context, PLD_Image* image)
{
    SDL_FRect rect = {0};
    Uint8 alpha;
    SDL_GetTextureAlphaMod(image->texture, &alpha);
    SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(context->renderer, NULL);
    
    float image_ratio = (float)image->width / image->height;
    float game_ratio = (float)PLD_LOGICAL_WIDTH / PLD_LOGICAL_HEIGHT;

    if (image_ratio > game_ratio)
    {
        rect.w = PLD_LOGICAL_WIDTH;
        rect.h = PLD_LOGICAL_WIDTH / image_ratio;
        rect.x = 0.0f;
        rect.y = (PLD_LOGICAL_HEIGHT - rect.h) / 2.0f;
    }
    else
    {
        rect.h = PLD_LOGICAL_HEIGHT;
        rect.w = PLD_LOGICAL_HEIGHT * image_ratio;
        rect.y = 0.0f;
        rect.x = (PLD_LOGICAL_WIDTH - rect.w) / 2.0f;
    }

    PLD_ScaleRect(&rect);

    return SDL_RenderTexture(context->renderer, image->texture, NULL, &rect);
}

PLD_Image* PLD_LoadSVG(PLD_Context* context, const char* path, int width, int height)
{
    return PLD_LoadSVG_IO(context, SDL_IOFromFile(path, "rb"), true, width, height);
}

PLD_Image* PLD_LoadSVG_IO(PLD_Context* context, SDL_IOStream* stream, bool close, int width, int height)
{
    if (!stream) { SDL_Log("Failed to load SVG : stream NULL"); return NULL; }

    float image_scale = (float)PLD_LOGICAL_WIDTH / context->config->image_width;
    SDL_Surface* surface = IMG_LoadSizedSVG_IO(stream, width / image_scale, height / image_scale);
    if (!surface) { goto error; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(context->renderer, surface);
    if (!texture) { goto error; }
    SDL_DestroySurface(surface);

    PLD_Image* image = SDL_calloc(1, sizeof(PLD_Image));
    image->width = width;
    image->height = height;
    image->texture = texture;

    if (close) { SDL_CloseIO(stream); }
    return image;

error:
    SDL_Log("Failed to load SVG : %s", SDL_GetError());
    if (surface) { SDL_DestroySurface(surface); }
    if (texture) { SDL_DestroyTexture(texture); }
    if (stream && close) { SDL_CloseIO(stream); }
    return NULL;
}