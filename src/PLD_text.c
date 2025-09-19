#include <PLD_text.h>


#define PLD_FONT_FILE "font.otf"


static TTF_Font* PLD_font = NULL;


bool PLD_OpenFont(PLD_Context* context)
{
    if (PLD_font != NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font already open");
        return false;
    }

    char* path;
    SDL_asprintf(&path, "%s%s", context->data_path, PLD_FONT_FILE);
    PLD_font = TTF_OpenFont(path, PLD_FONT_SIZE);
    SDL_free(path);

    if (PLD_font == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }

    return true;
}


PLD_Text* PLD_CreateText(PLD_Context* context, const char* text_string)
{
    if (PLD_font == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font not open");
        return NULL;
    }

    PLD_Text* text = SDL_calloc(1, sizeof(PLD_Text));
    SDL_Surface* surface = NULL;
    SDL_Color color = {255, 255, 255, 255};

    surface = TTF_RenderText_Blended(PLD_font, text_string, 0, color);
    if (surface == NULL)
    {
        goto error;
    }

    text->texture = SDL_CreateTextureFromSurface(context->renderer, surface);
    if (text->texture == NULL)
    {
        goto error;
    }

    SDL_DestroySurface(surface);

    return text;

error:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());

    if (surface != NULL)
    {
        SDL_DestroySurface(surface);
    }

    if (text != NULL)
    {
        PLD_DestroyText(text);
    }

    return NULL;
}


bool PLD_SetTextColor(PLD_Text* text, Uint8 r, Uint8 g, Uint8 b)
{
    return SDL_SetTextureColorMod(text->texture, r, g, b);
}


bool PLD_RenderText(PLD_Context* context, PLD_Text* text, int posx, int posy, int max_width, int alpha, bool center)
{
    SDL_FRect src_rect = {0, 0, text->texture->w, text->texture->h};
    SDL_FRect dst_rect = {0, 0, text->texture->w, text->texture->h};

    if (src_rect.w > max_width)
    {
        src_rect.w = max_width;
        dst_rect.w = max_width;
    }

    dst_rect.x = posx;
    dst_rect.y = posy;

    dst_rect.y -= dst_rect.h / 2;
    if (center)
    {
        dst_rect.x -= dst_rect.w / 2;
    }

    SDL_SetTextureAlphaMod(text->texture, alpha);

    return SDL_RenderTexture(context->renderer, text->texture, &src_rect, &dst_rect);
}


void PLD_DestroyText(PLD_Text* text)
{
    if (text)
    {
        if (text->texture != NULL)
        {
            SDL_DestroyTexture(text->texture);
        }

        SDL_free(text);
    }
}


void PLD_CloseFont()
{
    if (PLD_font == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font not open");
        return;
    }

    TTF_CloseFont(PLD_font);
    PLD_font = NULL;
}
