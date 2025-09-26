#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <PLD/PLD_video.h>
#include <PLD/PLD_utils.h>

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
	PLD_Image* image;
	SDL_Thread* thread;
	SDL_Mutex* mutex;
	int width;
	int height;

	AVFormatContext* context;
    AVCodecContext* codec_context;
    const AVCodec* codec;
    AVFrame* frame;
    AVPacket* packet;
    int stream_index;
    AVStream* stream;

	PLD_Plane y;
	PLD_Plane u;
	PLD_Plane v;
}PLD_Video;

bool PLD_IsSupportedVideo(const char* path)
{
	return (PLD_EndsWith(path, ".mp4")) ||
		   (PLD_EndsWith(path, ".mkv")) ||
		   (PLD_EndsWith(path, ".webm")) ||
		   (PLD_EndsWith(path, ".mpg")) ||
		   (PLD_EndsWith(path, ".mpeg"));
}

int PLD_FreeVideo(PLD_Video* video)
{
	SDL_LockMutex(video->mutex);
	SDL_DetachThread(video->thread);
	PLD_DestroyImage(video->image);
    video->quit = true;
    SDL_UnlockMutex(video->mutex);

	return PLD_SUCCESS;
}

void PLD_AllocFrame(PLD_Video* video)
{
	video->y.pitch = video->frame->linesize[0];
	video->y.height = video->frame->height;

	video->u.pitch = video->frame->linesize[1];
	video->u.height = video->frame->height / 2;

	video->v.pitch = video->frame->linesize[2];
	video->v.height = video->frame->height / 2;

	video->y.data = SDL_malloc(video->y.pitch * video->y.height);
	video->u.data = SDL_malloc(video->u.pitch * video->u.height);
	video->v.data = SDL_malloc(video->v.pitch * video->v.height);
}

void PLD_CopyFrame(PLD_Video* video)
{
	SDL_memcpy(video->y.data, video->frame->data[0], video->y.pitch * video->y.height);
	SDL_memcpy(video->u.data, video->frame->data[1], video->u.pitch * video->u.height);
	SDL_memcpy(video->v.data, video->frame->data[2], video->v.pitch * video->v.height);
}

void PLD_DestroyFrame(PLD_Video* video)
{
	SDL_free(video->y.data);
	SDL_free(video->u.data);
	SDL_free(video->v.data);
}

int PLD_VideoLoop(void* data)
{
    bool first = true;
    double time = 0.0;
    double video_time = 0.0;
    int64_t timestamp = 0;
    PLD_Video* video = (PLD_Video*)data;

    SDL_LockMutex(video->mutex);
    while (!video->quit)
    {
        time = video->currentTime;

        if ((first) || (video_time < time) || (SDL_abs(video_time - time) >= 1))
        {
            SDL_UnlockMutex(video->mutex);

            if ((!first) && (SDL_abs(time - video_time) >= 1))
            {
                av_seek_frame(video->context, video->stream_index, timestamp, AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(video->codec_context);
            }

            bool frame_decoded = false;

            while (!frame_decoded)
            {
                if (av_read_frame(video->context, video->packet) < 0) {
                    break;
                }

                if (video->packet->stream_index == video->stream_index)
                {
                    avcodec_send_packet(video->codec_context, video->packet);

                    if (avcodec_receive_frame(video->codec_context, video->frame) == 0)
                    {
                        video_time = video->frame->pts * av_q2d(video->stream->time_base);
                        timestamp = (int64_t)(time / av_q2d(video->stream->time_base));
                        frame_decoded = true;
                    }
                }

				av_packet_unref(video->packet);
            }

            first = false;

			SDL_LockMutex(video->mutex);

			if (frame_decoded)
			{
				if (video->y.data == NULL)
				{
					PLD_AllocFrame(video);
				}

				PLD_CopyFrame(video);

				av_frame_unref(video->frame);
			}
		}

        SDL_UnlockMutex(video->mutex);

        SDL_Delay(1);

        SDL_LockMutex(video->mutex);
    }
    SDL_UnlockMutex(video->mutex);

	if (video->y.data != NULL)
	{
		PLD_DestroyFrame(video);
	}

    SDL_DestroyMutex(video->mutex);
    av_frame_free(&video->frame);
    av_packet_free(&video->packet);
    avcodec_free_context(&video->codec_context);
    avformat_close_input(&video->context);

    SDL_free(video);

    return PLD_SUCCESS;
}

PLD_Video* PLD_LoadVideo(PLD_Context* context, const char* path, int time)
{	
	PLD_Video* video = (PLD_Video*)SDL_calloc(1, sizeof(PLD_Video));
	
	if ((avformat_open_input(&video->context, path, NULL, NULL) < 0) || (avformat_find_stream_info(video->context, NULL) < 0))
	{
		SDL_Log("Failed to load video");
		goto error;
	}

	video->stream_index = av_find_best_stream(video->context, AVMEDIA_TYPE_VIDEO, -1, -1, &video->codec, 0);
	video->codec_context = avcodec_alloc_context3(video->codec);
	avcodec_parameters_to_context(video->codec_context, video->context->streams[video->stream_index]->codecpar);
    avcodec_open2(video->codec_context, video->codec, NULL);

	video->stream = video->context->streams[video->stream_index];
	video->frame = av_frame_alloc();
    video->packet = av_packet_alloc();

	video->width = video->stream->codecpar->width;
	video->height = video->stream->codecpar->height;
	
	SDL_Texture* texture;
	if ((texture = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video->width, video->height)) == NULL)
	{
		SDL_Log("Failed to create texture");
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
	{
		return NULL;
	}

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
