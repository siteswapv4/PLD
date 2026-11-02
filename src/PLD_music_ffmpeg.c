#include <PLD/PLD_music.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

typedef struct PLD_Music
{
    AVFormatContext* format_context;
    const AVCodec* codec;
    int stream_index;
    const AVStream* stream;
    AVCodecContext* decoder;
    AVPacket* packet;
    AVFrame* frame;

    SDL_AudioStream* playback_stream;

    int moved_position;
    int played_samples;
    int last_sample_time;
    int paused_time;
    int sample_rate;

    Uint64 decode_time;

    bool paused;
}PLD_Music;

typedef struct PLD_FFmpegIO
{
	SDL_IOStream* io;
}PLD_FFmpegIO;

static PLD_Music* PLD_current_music = NULL;
static SDL_Thread* PLD_music_thread = NULL;
static SDL_Mutex* PLD_music_mutex = NULL;
static bool PLD_stop_music_thread = false;

void PLD_MusicLogic()
{
    if (!PLD_current_music) return;
    if (PLD_current_music->paused) return;

    Uint64 now = SDL_GetTicks();

    if ((now - PLD_current_music->decode_time) < 10)
    {
        return;
    }

    PLD_current_music->decode_time = now;

    while (SDL_GetAudioStreamQueued(PLD_current_music->playback_stream) < (PLD_current_music->sample_rate * 2))
    {
        if (av_read_frame(PLD_current_music->format_context, PLD_current_music->packet) < 0)
            break;

        if (PLD_current_music->packet->stream_index != PLD_current_music->stream_index)
        {
            av_packet_unref(PLD_current_music->packet);
            continue;
        }

        avcodec_send_packet(PLD_current_music->decoder, PLD_current_music->packet);

        while (avcodec_receive_frame(PLD_current_music->decoder, PLD_current_music->frame) == 0)
        {
            SDL_PutAudioStreamPlanarData(PLD_current_music->playback_stream, (void*)PLD_current_music->frame->data, PLD_current_music->decoder->ch_layout.nb_channels, PLD_current_music->frame->nb_samples);
        }

        av_frame_unref(PLD_current_music->frame);
        av_packet_unref(PLD_current_music->packet);
    }
}

int PLD_MusicLoop(void* userdata)
{
    while (true)
    {
        SDL_LockMutex(PLD_music_mutex);
        if (PLD_stop_music_thread)
        {
            SDL_UnlockMutex(PLD_music_mutex);
            return 0;
        }
        PLD_MusicLogic();
        SDL_UnlockMutex(PLD_music_mutex);

        SDL_Delay(1);
    }

    return 0;
}

bool PLD_IsSupportedMusic(const char* path)
{
    return (PLD_EndsWithInsensitive(path, ".mp3")) ||
           (PLD_EndsWithInsensitive(path, ".wav")) ||
           (PLD_EndsWithInsensitive(path, ".flac")) ||
           (PLD_EndsWithInsensitive(path, ".ogg")) ||
           (PLD_EndsWithInsensitive(path, ".opus")) ||
           (PLD_EndsWithInsensitive(path, ".mp4")) ||
           (PLD_EndsWithInsensitive(path, ".webm")) ||
           (PLD_EndsWithInsensitive(path, ".mkv"));
}

bool PLD_InitMusic(PLD_Context* context)
{
    PLD_music_mutex = SDL_CreateMutex();
    PLD_music_thread = SDL_CreateThread(PLD_MusicLoop, "PLD music thread", NULL);
    PLD_stop_music_thread = false;

    return true;
}

void PLD_QuitMusic()
{
    if (PLD_music_thread)
    {
        SDL_LockMutex(PLD_music_mutex);
        PLD_stop_music_thread = true;
        SDL_UnlockMutex(PLD_music_mutex);

        SDL_WaitThread(PLD_music_thread, NULL);
        SDL_DestroyMutex(PLD_music_mutex);

        PLD_music_thread = NULL;
        PLD_music_mutex = NULL;
    }
}

static SDL_AudioFormat PLD_GetAudioFormat(enum AVSampleFormat format)
{
    switch (format)
    {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            return SDL_AUDIO_U8;

        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            return SDL_AUDIO_S16;

        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
            return SDL_AUDIO_S32;

        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            return SDL_AUDIO_F32;

        default:
            return SDL_AUDIO_F32;
    }
}

static int PLD_FFmpegRead(void* opaque, uint8_t* buffer, int buffer_size)
{
	PLD_FFmpegIO* io = (PLD_FFmpegIO*)opaque;

	size_t bytes_read = SDL_ReadIO(io->io, buffer, buffer_size);

	if (bytes_read > 0)
		return (int)bytes_read;

	if (SDL_GetIOStatus(io->io) == SDL_IO_STATUS_EOF)
		return AVERROR_EOF;

	return AVERROR(EIO);
}

static int64_t PLD_FFmpegSeek(void* opaque, int64_t offset, int whence)
{
	PLD_FFmpegIO* io = (PLD_FFmpegIO*)opaque;

	// FFmpeg uses this to ask for the size of the input.
	if (whence == AVSEEK_SIZE)
	{
		Sint64 size = SDL_GetIOSize(io->io);

		if (size < 0)
			return AVERROR(EIO);

		return size;
	}

	SDL_IOWhence sdl_whence;

	switch (whence)
	{
		case SEEK_SET:
			sdl_whence = SDL_IO_SEEK_SET;
			break;

		case SEEK_CUR:
			sdl_whence = SDL_IO_SEEK_CUR;
			break;

		case SEEK_END:
			sdl_whence = SDL_IO_SEEK_END;
			break;

		default:
			return AVERROR(EINVAL);
	}

	Sint64 position = SDL_SeekIO(io->io, offset, sdl_whence);

	if (position < 0)
		return AVERROR(EIO);

	return position;
}

PLD_Music* PLD_LoadMusic(PLD_Context* context, const char* path)
{
    PLD_Music* music = (PLD_Music*)SDL_calloc(1, sizeof(PLD_Music));

    if (!music)
        goto error;

    // Open the file through SDL3.
    SDL_IOStream* io = SDL_IOFromFile(path, "rb");

    if (!io)
    {
        SDL_Log("Failed to open music file '%s': %s",
            path,
            SDL_GetError());
        goto error;
    }

    // Wrapper used as the AVIOContext opaque pointer.
    PLD_FFmpegIO* ffmpeg_io =
        (PLD_FFmpegIO*)SDL_malloc(sizeof(PLD_FFmpegIO));

    if (!ffmpeg_io)
    {
        SDL_Log("Failed to allocate PLD_FFmpegIO");

        SDL_CloseIO(io);
        goto error;
    }

    ffmpeg_io->io = io;

    // FFmpeg I/O buffer.
    const int buffer_size = 32 * 1024;

    unsigned char* buffer =
        (unsigned char*)av_malloc(buffer_size);

    if (!buffer)
    {
        SDL_Log("Failed to allocate FFmpeg I/O buffer");

        SDL_free(ffmpeg_io);
        SDL_CloseIO(io);
        goto error;
    }

    AVIOContext* avio = avio_alloc_context(
        buffer,
        buffer_size,
        0,                  // read-only
        ffmpeg_io,
        PLD_FFmpegRead,
        NULL,               // no write callback
        PLD_FFmpegSeek
    );

    if (!avio)
    {
        SDL_Log("Failed to create AVIOContext");

        av_free(buffer);
        SDL_free(ffmpeg_io);
        SDL_CloseIO(io);
        goto error;
    }

    // Allocate the format context ourselves so we can
    // attach the custom AVIOContext.
    music->format_context = avformat_alloc_context();

    if (!music->format_context)
    {
        SDL_Log("Failed to allocate AVFormatContext");

        avio_context_free(&avio);
        SDL_free(ffmpeg_io);
        SDL_CloseIO(io);
        goto error;
    }

    music->format_context->pb = avio;
    music->format_context->flags |= AVFMT_FLAG_CUSTOM_IO;

    /*
     * The actual bytes come from our SDL_IOStream.
     *
     * Passing path here is still useful because FFmpeg can use
     * the filename/extension as format information.
     */
    int ret = avformat_open_input(
        &music->format_context,
        path,
        NULL,
        NULL
    );

    if (ret < 0)
    {
        char error[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, error, sizeof(error));

        SDL_Log(
            "avformat_open_input('%s') failed: %s",
            path,
            error
        );

        goto error;
    }

    ret = avformat_find_stream_info(
        music->format_context,
        NULL
    );

    if (ret < 0)
    {
        char error[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, error, sizeof(error));

        SDL_Log(
            "avformat_find_stream_info failed: %s",
            error
        );

        goto error;
    }

    music->stream_index =
        av_find_best_stream(
            music->format_context,
            AVMEDIA_TYPE_AUDIO,
            -1,
            -1,
            &music->codec,
            0
        );

    if (music->stream_index < 0)
        goto error;

    music->stream =
        music->format_context->streams[music->stream_index];

    for (int i = 0;
         i < music->format_context->nb_streams;
         i++)
    {
        if (i != music->stream_index)
        {
            music->format_context->streams[i]->discard =
                AVDISCARD_ALL;
        }
    }

    music->decoder =
        avcodec_alloc_context3(music->codec);

    if (!music->decoder)
        goto error;

    music->decoder->thread_count = 0;

    if (avcodec_parameters_to_context(
            music->decoder,
            music->stream->codecpar) < 0)
    {
        goto error;
    }

    if (avcodec_open2(
            music->decoder,
            music->codec,
            NULL) < 0)
    {
        goto error;
    }

    music->packet = av_packet_alloc();
    music->frame = av_frame_alloc();

    if (!music->packet || !music->frame)
        goto error;

    SDL_AudioSpec spec = {
        PLD_GetAudioFormat(music->decoder->sample_fmt),
        music->decoder->ch_layout.nb_channels,
        music->decoder->sample_rate
    };

    music->playback_stream =
        SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &spec,
            NULL,
            NULL
        );

    if (!music->playback_stream)
        goto error;

    SDL_GetAudioStreamFormat(
        music->playback_stream,
        NULL,
        &spec
    );

    music->sample_rate =
        ((SDL_AUDIO_BITSIZE(spec.format) / 8) *
         spec.freq *
         spec.channels);

    return music;

error:
    SDL_Log("Failed to load music");
    PLD_DestroyMusic(music);
    return NULL;
}

static void PLD_PostMixCallback(void* udata, const SDL_AudioSpec* spec, float* pcm, int len)
{
    if ((PLD_current_music != NULL) && (!PLD_current_music->paused))
    {
        PLD_current_music->played_samples += len;
        PLD_current_music->last_sample_time = SDL_GetTicks();
    }
}

bool PLD_SetMusicPosition(PLD_Music* music, int position)
{
    SDL_LockMutex(PLD_music_mutex);

    int64_t timestamp = av_rescale(position, music->stream->time_base.den, music->stream->time_base.num * 1000);

    if (av_seek_frame(music->format_context, music->stream_index, timestamp, AVSEEK_FLAG_BACKWARD) < 0)
    {
        return false;
    }

    avcodec_flush_buffers(music->decoder);
    SDL_ClearAudioStream(music->playback_stream);

    music->moved_position = position;
    music->played_samples = 0;
    music->last_sample_time = SDL_GetTicks();
    music->decode_time = SDL_GetTicks();

    SDL_UnlockMutex(PLD_music_mutex);

    return true;
}

int PLD_GetMusicPosition(PLD_Music* music)
{
    if (PLD_current_music != music)
    {
        return 0;
    }

    if (!PLD_current_music->paused)
    {
        double temp_time = (double)music->played_samples / music->sample_rate;
        int time = temp_time * 1000;
        time += SDL_GetTicks() - music->last_sample_time;

        return time + music->moved_position;
    }
    else
    {
        return music->paused_time;
    }
}

bool PLD_PlayMusic(PLD_Context* context, PLD_Music* music)
{
    SDL_LockMutex(PLD_music_mutex);

    music->played_samples = 0;
    music->last_sample_time = SDL_GetTicks();
    music->decode_time = SDL_GetTicks();
    music->paused = false;

    SDL_AudioDeviceID devid = SDL_GetAudioStreamDevice(music->playback_stream);
    SDL_SetAudioPostmixCallback(devid, PLD_PostMixCallback, NULL);

    PLD_current_music = music;
    PLD_SetMusicPosition(music, 0);

    SDL_SetAudioStreamGain(music->playback_stream, context->config->music_volume);
    SDL_ResumeAudioStreamDevice(music->playback_stream);

    SDL_UnlockMutex(PLD_music_mutex);

    return true;
}

bool PLD_StopMusic(PLD_Music* music)
{
    SDL_LockMutex(PLD_music_mutex);

    SDL_PauseAudioStreamDevice(music->playback_stream);
    SDL_ClearAudioStream(music->playback_stream);

    PLD_current_music = NULL;

    music->moved_position = 0;
    music->played_samples = 0;
    music->paused = false;

    SDL_UnlockMutex(PLD_music_mutex);

    return true;
}

void PLD_PauseMusic(PLD_Music* music)
{
    SDL_LockMutex(PLD_music_mutex);

    music->paused_time = PLD_GetMusicPosition(music);

    SDL_PauseAudioStreamDevice(music->playback_stream);

    music->paused = true;

    SDL_UnlockMutex(PLD_music_mutex);
}

void PLD_ResumeMusic(PLD_Music* music)
{
    SDL_LockMutex(PLD_music_mutex);

    SDL_ResumeAudioStreamDevice(music->playback_stream);

    music->paused = false;
    music->last_sample_time = SDL_GetTicks();
    music->decode_time = SDL_GetTicks();

    SDL_UnlockMutex(PLD_music_mutex);
}

bool PLD_MusicIsPaused(PLD_Music* music)
{
    return music->paused;
}

static void PLD_FreeFFmpegIO(AVFormatContext* format_context)
{
    if (!format_context)
        return;

    AVIOContext* avio = format_context->pb;

    if (avio)
    {
        PLD_FFmpegIO* ffmpeg_io =
            (PLD_FFmpegIO*)avio->opaque;

        // avformat_close_input() normally closes the AVIOContext
        // only when it owns the I/O. With custom IO, explicitly
        // free it after closing the format context.

        SDL_IOStream* io = ffmpeg_io ? ffmpeg_io->io : NULL;

        avio_context_free(&avio);

        if (io)
            SDL_CloseIO(io);

        SDL_free(ffmpeg_io);

        format_context->pb = NULL;
    }
}

void PLD_DestroyMusic(PLD_Music* music)
{
    SDL_LockMutex(PLD_music_mutex);

    if (!music) return;

    if (PLD_current_music == music)
    {
        PLD_current_music = NULL;
    }

    PLD_FreeFFmpegIO(music->format_context);
    if (music->playback_stream) SDL_DestroyAudioStream(music->playback_stream);
    if (music->frame) av_frame_free(&music->frame);
    if (music->packet) av_packet_free(&music->packet);
    if (music->decoder) avcodec_free_context(&music->decoder);
    if (music->format_context) avformat_close_input(&music->format_context);

    SDL_free(music);

    SDL_UnlockMutex(PLD_music_mutex);
}

void PLD_SetMusicVolume(float volume)
{
    SDL_LockMutex(PLD_music_mutex);

    if (PLD_current_music)
    {
        SDL_SetAudioStreamGain(PLD_current_music->playback_stream, volume);
    }

    SDL_UnlockMutex(PLD_music_mutex);
}

void PLD_UpdateMusic()
{
    if (PLD_music_thread) return;

    PLD_MusicLogic();
}
