#include "PLDVideo.h"

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"


/*
 *                            -----WARNING-----
 * I'm very unsure about the operations made on YUV textures here
 * If this ever crashes or produces brocken visuals on some platforms or with specific videos
 * it's possible to rewrite the file so that the middle frame is reallocated each cycle and so 
 * it'll be possible to use SDL_UpdateYUVTexture again !
 */


typedef struct PLD_Plane
{
	Uint8* data;
	int pitch;
	int height;
}PLD_Plane;


typedef struct PLD_Video
{
	bool quit;
	double currentTime;
	plm_frame_t* frame;
	PLD_Image* image;
	SDL_Thread* thread;
	SDL_Mutex* mutex;
	int width;
	int height;
	plm_t* plm;

	PLD_Plane y;
	PLD_Plane v;
	PLD_Plane u;
}PLD_Video;


int PLD_FreeVideo(PLD_Video* video)
{
	SDL_LockMutex(video->mutex);
    video->quit = true;
    SDL_UnlockMutex(video->mutex);
    
	SDL_DetachThread(video->thread);
	
	PLD_DestroyImage(video->image);

	return PLD_SUCCESS;
}


void PLD_AllocFrame(PLD_Video* video)
{
	video->y.pitch = video->frame->y.width;
	video->y.height = video->frame->y.height;

	video->u.pitch = video->frame->cb.width;
	video->u.height = video->frame->cb.height;

	video->v.pitch = video->frame->cr.width;
	video->v.height = video->frame->cr.height;

	video->y.data = SDL_malloc(video->y.pitch * video->y.height);
	video->u.data = SDL_malloc(video->u.pitch * video->u.height);
	video->v.data = SDL_malloc(video->v.pitch * video->v.height);
}


void PLD_CopyFrame(PLD_Video* video)
{
	SDL_memcpy(video->y.data, video->frame->y.data, video->y.pitch * video->y.height);
	SDL_memcpy(video->u.data, video->frame->cb.data, video->u.pitch * video->u.height);
	SDL_memcpy(video->v.data, video->frame->cr.data, video->v.pitch * video->v.height);
}


void PLD_DestroyFrame(PLD_Video* video)
{
	if (video->y.data != NULL)
	{
		SDL_free(video->y.data);
		SDL_free(video->u.data);
		SDL_free(video->v.data);
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

        if (!plm_has_ended(video->plm))
        {
            if ((video->frame == NULL) || (video->frame->time < time) || (SDL_abs(video->frame->time - time) >= 1))
            {
                SDL_UnlockMutex(video->mutex);

                if ((video->frame != NULL) && (SDL_abs(time - video->frame->time) >= 1))
                {
                    plm_seek(video->plm, time, false);
                }

                video->frame = plm_decode_video(video->plm);
                SDL_LockMutex(video->mutex);

				if (video->frame)
				{
					if (video->y.data == NULL)
					{
						PLD_AllocFrame(video);
					}

					PLD_CopyFrame(video);
				}
            }
        }
        SDL_UnlockMutex(video->mutex);
        SDL_Delay(1);
        SDL_LockMutex(video->mutex);
    }
    SDL_UnlockMutex(video->mutex);
    
	PLD_DestroyFrame(video);
    SDL_DestroyMutex(video->mutex);
    plm_destroy(video->plm);    
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
	
	video->width = plm_get_width(video->plm);
	video->height = plm_get_height(video->plm);
	
	SDL_Texture* texture;
	if ((texture = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video->width, video->height)) == NULL)
	{
		SDL_LogInfo(0, "Failed to create texture");
		goto error;
	}

	video->image = PLD_CreateImageFromTexture(context, texture);

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

PLD_Image* PLD_GetVideoFrame(PLD_Video* video)
{
	if (!video)
		return NULL;

	int y_size;
	int uv_size;

	SDL_LockMutex(video->mutex);
	
	if (video->y.data != NULL)
	{
		SDL_UpdateYUVTexture(video->image->texture, NULL, video->y.data, video->y.pitch, video->u.data, video->u.pitch, video->v.data, video->v.pitch);

		SDL_UnlockMutex(video->mutex);

		return video->image;
	}

	SDL_UnlockMutex(video->mutex);
	
	return NULL;
}
