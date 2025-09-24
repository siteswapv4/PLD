source ~/emsdk/emsdk_env.sh

emcc src/*.c src/UI/*.c -o index.html \
  -I "include" \
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
