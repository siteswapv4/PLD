#include <PLD/PLD_video.h>
#include <PLD/PLD_utils.h>

#define PL_MPEG_IMPLEMENTATION
#include <pl_mpeg/pl_mpeg.h>

typedef struct PLD_Video
{
	bool quit;
	double currentTime;
	PLD_Image* image;
	SDL_Thread* thread;
	SDL_Mutex* mutex;
	int width;
	int height;
	plm_t* plm;
	plm_frame_t* frame;
	plm_frame_t* frame_copy;
}PLD_Video;

bool PLD_IsSupportedVideo(const char* path)
{
	return (PLD_EndsWith(path, ".mpeg")) || (PLD_EndsWith(path, ".mpg"));
}

int PLD_FreeVideo(PLD_Video* video)
{
	SDL_LockMutex(video->mutex);
    video->quit = true;
    SDL_UnlockMutex(video->mutex);
    
	SDL_DetachThread(video->thread);
	PLD_DestroyImage(video->image);

	if (!video->thread)
	{
		plm_destroy(video->plm);
		SDL_free(video->frame_copy);
		SDL_free(video);
	}

	return PLD_SUCCESS;
}

void PLD_CopyFrame(plm_frame_t* destination, const plm_frame_t* source)
{
	if (!destination->y.data)
	{
		destination->y.data = SDL_malloc(source->y.width * source->y.height);
		destination->y.width = source->y.width;
		destination->y.height = source->y.height;
	}
	if (!destination->cb.data)
	{
		destination->cb.data = SDL_malloc(source->cb.width * source->cb.height);
		destination->cb.width = source->cb.width;
		destination->cb.height = source->cb.height;
	}
	if (!destination->cr.data)
	{
		destination->cr.data = SDL_malloc(source->cr.width * source->cr.height);
		destination->cr.width = source->cr.width;
		destination->cr.height = source->cr.height;
	}

	SDL_memcpy(destination->y.data, source->y.data, source->y.width * source->y.height);
	SDL_memcpy(destination->cb.data, source->cb.data, source->cb.width * source->cb.height);
	SDL_memcpy(destination->cr.data, source->cr.data, source->cr.width * source->cr.height);
}

void PLD_DecodeFrame(PLD_Video* video, double time)
{
	if ((video->frame == NULL) || (video->frame->time < time) || (SDL_fabs(video->frame->time - time) >= 1))
	{
		SDL_UnlockMutex(video->mutex);

		if ((video->frame != NULL) && (SDL_abs(time - video->frame->time) >= 1))
		{
			plm_seek(video->plm, time, false);
		}

		video->frame = plm_decode_video(video->plm);
		SDL_LockMutex(video->mutex);

		if ((video->thread) && (video->frame))
		{
			PLD_CopyFrame(video->frame_copy, video->frame);
		}
	}
}

int PLD_VideoLoop(void* data)
{
    double time;
    PLD_Video* video = (PLD_Video*)data;

    SDL_LockMutex(video->mutex);
    while (!video->quit)
    {
        time = video->currentTime;
		PLD_DecodeFrame(video, time);

        SDL_UnlockMutex(video->mutex);
        SDL_Delay(1);
        SDL_LockMutex(video->mutex);
    }
    SDL_UnlockMutex(video->mutex);
    
    SDL_DestroyMutex(video->mutex);
    plm_destroy(video->plm); 
	if (video->frame_copy->y.data) { SDL_free(video->frame_copy->y.data); }
	if (video->frame_copy->cb.data) { SDL_free(video->frame_copy->cb.data); }
	if (video->frame_copy->cr.data) { SDL_free(video->frame_copy->cr.data); }
	SDL_free(video->frame_copy);   
    SDL_free(video);

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
	plm_set_loop(video->plm, true);
	
	video->width = plm_get_width(video->plm);
	video->height = plm_get_height(video->plm);
	
	SDL_Texture* texture;
	if ((texture = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video->width, video->height)) == NULL)
	{
		SDL_LogInfo(0, "Failed to create texture");
		goto error;
	}

	video->image = PLD_CreateImageFromTexture(context, texture);
	video->frame_copy = SDL_calloc(1, sizeof(plm_frame_t));

	video->mutex = SDL_CreateMutex();
	video->thread = SDL_CreateThread(PLD_VideoLoop, "video decoder", video);
	
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
		
    SDL_LockMutex(video->mutex);
    video->currentTime = time / 1000.0;
    SDL_UnlockMutex(video->mutex);
    
    return PLD_SUCCESS;
}

void PLD_UpdateYUVImage(PLD_Image* image, const plm_frame_t* frame)
{
	SDL_UpdateYUVTexture(image->texture, NULL, frame->y.data, frame->y.width, frame->cb.data, frame->cb.width, frame->cr.data, frame->cr.width);
}

PLD_Image* PLD_GetVideoFrame(PLD_Video* video)
{
	if (!video)
		return NULL;

	if (video->thread)
	{
		SDL_LockMutex(video->mutex);
		if (video->frame_copy->y.data)
		{
			PLD_UpdateYUVImage(video->image, video->frame_copy);
			SDL_UnlockMutex(video->mutex);

			return video->image;
		}
		SDL_UnlockMutex(video->mutex);
		return NULL;
	}
	else
	{
		PLD_DecodeFrame(video, video->currentTime);
		if (video->frame)
		{
			PLD_UpdateYUVImage(video->image, video->frame);
			return video->image;
		}
		else
		{
			return NULL;
		}
	}
}