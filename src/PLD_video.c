#include "PLDVideo.h"
#include "PLDUtils.h"

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

typedef struct PLD_Video
{
	int width;
	int height;
	plm_t* plm;
	plm_frame_t* frame;
	PLD_Image* image;
	double currentTime;
}PLD_Video;

bool PLD_IsSupportedVideo(const char* path)
{
	return (PLD_EndsWith(path, ".mpeg")) || (PLD_EndsWith(path, ".mpg"));
}

int PLD_FreeVideo(PLD_Video* video)
{
	plm_destroy(video->plm);
	PLD_DestroyImage(video->image);

	return PLD_SUCCESS;
}

PLD_Video* PLD_LoadVideo(PLD_Context* context, const char* path, int time)
{
	if ((!context) || (!path))
		return NULL;
	
	PLD_Video* video = (PLD_Video*)SDL_calloc(1, sizeof(PLD_Video));
	
	if ((video->plm = plm_create_with_filename(path)) == NULL)
	{
		SDL_LogInfo(0, "Failed to load video");
		goto error;
	}
	
	plm_set_audio_enabled(video->plm, false);
	
	video->width = plm_get_width(video->plm);
	video->height = plm_get_height(video->plm);
	
	SDL_Texture* texture;
	if ((texture = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video->width, video->height)) == NULL)
	{
		SDL_LogInfo(0, "Failed to create texture");
		goto error;
	}

	video->image = PLD_CreateImageFromTexture(context, texture);
	return video;
	
error:
	if (video)
		PLD_FreeVideo(video);
	
	return NULL;
}

int PLD_UpdateVideoTime(PLD_Video* video, int time)
{
	if (!video)
		return PLD_FAILURE;

    video->currentTime = time / 1000.0;
    
    return PLD_SUCCESS;
}

PLD_Image* PLD_GetVideoFrame(PLD_Video* video)
{
	if (!video)
		return NULL;


	if (!plm_has_ended(video->plm))
	{
		if ((video->frame == NULL) || (video->frame->time < video->currentTime) || (SDL_abs(video->frame->time - video->currentTime) >= 1))
		{
			if ((video->frame != NULL) && (SDL_abs(video->currentTime - video->frame->time) >= 1))
			{
				plm_seek(video->plm, video->currentTime, false);
			}

			video->frame = plm_decode_video(video->plm);
			if (video->frame)
			{
				SDL_UpdateYUVTexture(video->image->texture, NULL, video->frame->y.data, video->frame->y.width, video->frame->cb.data, video->frame->cb.width, video->frame->cr.data, video->frame->cr.width);
			}
		}
	}

	return video->image;
}