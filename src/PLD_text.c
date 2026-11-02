#include <PLD/PLD_text.h>

#define PLD_FONT_FILE "font.otf"

typedef struct PLD_Text
{
    SDL_Texture* text;
    SDL_Color color;
    int width;
    int height;
}PLD_Text;

bool PLD_OpenFont(PLD_Context* context)
{
    if (context->font != NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font already open");
        return false;
    }

    float scale = (float)context->config->image_width / (float)PLD_LOGICAL_WIDTH;
    char* path;
    SDL_asprintf(&path, "%s%s", context->data_path, PLD_FONT_FILE);
    context->font = TTF_OpenFont(path, (int)(PLD_FONT_SIZE * scale));
    SDL_free(path);

    if (!context->font)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }

    return true;
}


PLD_Text* PLD_CreateText(PLD_Context* context, const char* text_string, int wrap_width)
{
    if (!context->font)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font not open");
        return NULL;
    }

    float scale = (float)PLD_LOGICAL_WIDTH / context->config->image_width;

    PLD_Text* text = SDL_calloc(1, sizeof(PLD_Text));

    text->color = (SDL_Color){255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(context->font, text_string, 0, text->color, wrap_width / scale);
    text->text = SDL_CreateTextureFromSurface(context->renderer, surface);
    SDL_DestroySurface(surface);
    
    text->width = text->text->w;
    text->height = text->text->h;

    return text;
}

bool PLD_GetTextSize(PLD_Context* context, PLD_Text* text, int* width, int* height)
{
    if (!context || !text) return false;

    float scale = (float)PLD_LOGICAL_WIDTH / context->config->image_width;
    *width = text->width * scale;
    *height = text->height * scale;

    return true;
}


bool PLD_SetTextColor(PLD_Text* text, SDL_Color color)
{
    if (!text) { return false; }
    text->color = color;
    return true;
}


bool PLD_RenderText(PLD_Context* context, PLD_Text* text, SDL_Point position, int max_width, int alpha, bool center, float scroll)
{
    scroll = SDL_clamp(scroll, 0.0f, 1.0f);

    float scale = (float)PLD_LOGICAL_WIDTH / context->config->image_width;

    SDL_FRect rect = {position.x, position.y, text->width * scale, text->height * scale};
    SDL_FRect src_rect = {0, 0, text->width, text->height};

    rect.y -= rect.h / 2;
    if (center)
    {
        rect.x -= rect.w / 2;
    }

    if ((max_width > 0) && (max_width < rect.w))
    {
        int max_scroll = rect.w - max_width;

        src_rect.x = max_scroll * scroll / scale;
        src_rect.w = max_width / scale;

        rect.w = max_width;
    }

    SDL_SetTextureColorMod(text->text, text->color.r, text->color.g, text->color.b);
    SDL_SetTextureAlphaMod(text->text, text->color.a);

    if (!SDL_GetRenderTarget(context->renderer))
        PLD_ScaleRect(&rect);
    else
    {
        rect.x /= scale;
        rect.y /= scale;
        rect.w /= scale;
        rect.h /= scale;
    }

    SDL_RenderTexture(context->renderer, text->text, &src_rect, &rect);

    return true;
}


void PLD_DestroyText(PLD_Text* text)
{
    if (text)
    {
        if (text->text) { SDL_DestroyTexture(text->text); }
        SDL_free(text);
    }
}


void PLD_CloseFont(PLD_Context* context)
{
    if (!context->font)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font not open");
        return;
    }

    TTF_CloseFont(context->font);
    context->font = NULL;
}