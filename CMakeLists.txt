cmake_minimum_required(VERSION 4.0)

project(PLD)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(PLD_FFMPEG "Use ffmpeg instead of pl_mpeg" OFF)

if(PLD_VIDEO_FFMPEG)
	set(SRC_VIDEO src/PLD_video_ffmpeg.c)
	set(VIDEO_LIBS "avcodec" "avformat" "avutil")
else()
	set(SRC_VIDEO src/PLD_video_plmpeg.c)
	set(VIDEO_LIBS "")
endif()

if(PLD_MUSIC_FFMPEG)
    set(SRC_AUDIO src/PLD_music_ffmpeg.c)
    set(AUDIO_LIBS "avcodec" "avformat" "avutil" "swresample")
else()
    set(SRC_AUDIO src/PLD_music_sdlmixer.c)
    set(AUDIO_LIBS "")
endif()

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(
	include
	pl_mpeg/include
	md5/include
)

add_executable(${PROJECT_NAME}
	src/PLD.c
	src/PLD_array_list.c
	src/PLD_context.c
	src/PLD_control.c
	src/PLD_core.c
	src/PLD_dsc.c
	src/PLD_effect.c
	src/PLD_evaluate.c
	src/PLD_hold.c
	src/PLD_image.c
	src/PLD_menu.c
	src/PLD_note.c
	src/PLD_pause.c
	src/PLD_ppd.c
	src/PLD_result.c
	src/PLD_song_menu.c
	src/PLD_sound.c
	src/PLD_start_menu.c
	src/PLD_text.c
	src/PLD_touch.c
	src/PLD_utils.c
	src/PLD_user.c	
	src/PLD_touch_config.c

	src/PLD_animation.c
	src/PLD_choice.c
	src/PLD_popup.c
	
	md5/src/md5.c

	${SRC_VIDEO}
	${SRC_AUDIO}
)

target_link_libraries(${PROJECT_NAME}
    SDL3_image
    SDL3_ttf
    SDL3_mixer
    SDL3
    ${VIDEO_LIBS}
    ${AUDIO_LIBS}
)

add_custom_command(TARGET PLD POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:PLD>/data
)
