cmake_minimum_required(VERSION 4.1)

project(PLD)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(PLD_FFMPEG "Use ffmpeg instead of pl_mpeg" OFF)

if(PLD_FFMPEG)
	set(SRC_VIDEO src/PLD_video_ffmpeg.c)
	set(VIDEO_LIBS avcodec avformat avutil)
else()
	set(SRC_VIDEO src/PLD_video_plmpeg.c)
	set(VIDEO_LIBS "")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(
	include
	UI/include
	pl_mpeg/include
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
	src/PLD_utils.c
	
	UI/src/UI_animation.c
	UI/src/UI_choice.c
	UI/src/UI_popup.c
	UI/src/UI_utils.c
	${SRC_VIDEO}
)

target_link_libraries(${PROJECT_NAME}
    SDL3_image
    SDL3_mixer
    SDL3_ttf
    SDL3
    ${VIDEO_LIBS}
)

add_custom_command(TARGET PLD POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:PLD>/data
)
