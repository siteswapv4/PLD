source ~/emsdk/emsdk_env.sh

emcc \
  src/PLD.c src/PLD_array_list.c src/PLD_context.c src/PLD_control.c src/PLD_core.c src/PLD_dsc.c src/PLD_effect.c src/PLD_evaluate.c src/PLD_hold.c src/PLD_image.c src/PLD_menu.c src/PLD_note.c src/PLD_pause.c src/PLD_ppd.c src/PLD_result.c src/PLD_song_menu.c src/PLD_sound.c src/PLD_start_menu.c src/PLD_text.c src/PLD_utils.c src/PLD_video_plmpeg.c \
  UI/src/*.c \
  UDC/src/*.c \
  -o index.html \
  -I "include" \
  -I "UI/include" \
  -I "UDC/include" \
  -I "pl_mpeg/include" \
  -I ~/Documents/SDL3_emscripten/include/ \
  -L ~/Documents/SDL3_emscripten/lib/ \
  -O3 \
  -lSDL3 -lSDL3_image -lSDL3_mixer -lSDL3_ttf \
  -lFLAC -lgme -logg -lopus -lopusfile -lsharpyuv -ltiff -lvorbis -lvorbisfile -lwavpack -lwebp -lwebpdemux -lwebpmux -lxmp \
  -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale \
  -sALLOW_MEMORY_GROWTH \
  -sEXPORTED_RUNTIME_METHODS="["ccall"]" \
  -sEXPORTED_FUNCTIONS="["_free", "_main"]" \
  --embed-file ./data@data/ \
  --shell-file ~/emsdk/upstream/emscripten/src/shell_minimal.html \
&& python3 -m http.server
