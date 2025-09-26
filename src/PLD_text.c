#include <PLD/PLD_text.h>

#define PLD_FONT_FILE "font.otf"

typedef struct PLD_Text
{
    TTF_Text* text;
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

    char* path;
    SDL_asprintf(&path, "%s%s", context->data_path, PLD_FONT_FILE);
    context->font = TTF_OpenFont(path, PLD_FONT_SIZE);
    SDL_free(path);

    if (!context->font)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return false;
    }

    context->text_engine = TTF_CreateRendererTextEngine(context->renderer);
    if (!context->text_engine)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        TTF_CloseFont(context->font);
        context->font = NULL;
    }

    return true;
}


PLD_Text* PLD_CreateText(PLD_Context* context, const char* text_string)
{
    if (!context->font)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font not open");
        return NULL;
    }

    PLD_Text* text = SDL_calloc(1, sizeof(PLD_Text));
    text->text = TTF_CreateText(context->text_engine, context->font, text_string, 0);
    if (!text->text)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create text");
        return NULL;
    }
    text->color = (SDL_Color){255, 255, 255, 255};
    TTF_GetTextSize(text->text, &text->width, &text->height);

    return text;
}


bool PLD_SetTextColor(PLD_Text* text, SDL_Color color)
{
    if (!text) { return false; }
    text->color = color;
    return true;
}


bool PLD_RenderText(PLD_Context* context, PLD_Text* text, int posx, int posy, int max_width, int alpha, bool center)
{
    SDL_Point position = {posx, posy};

    position.y -= text->height / 2;
    if (center)
    {
        position.x -= text->width / 2;
    }

    if (max_width < text->width)
    {
        SDL_Rect rect = {position.x, position.y, max_width, text->height};
        SDL_SetRenderClipRect(context->renderer, &rect);
    }

    TTF_SetTextColor(text->text, text->color.r, text->color.g, text->color.b, text->color.a);
    TTF_DrawRendererText(text->text, position.x, position.y);
    SDL_SetRenderClipRect(context->renderer, NULL);
    return true;
}


void PLD_DestroyText(PLD_Text* text)
{
    if (text)
    {
        if (text->text) { TTF_DestroyText(text->text); }
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

    TTF_DestroyRendererTextEngine(context->text_engine);
    TTF_CloseFont(context->font);
    context->text_engine = NULL;
    context->font = NULL;
}