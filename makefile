P=game
OBJECTS=
LDLIBS=
CFLAGS=`pkg-config --cflags sdl2` -g -Wall -std=gnu11 -O3
# CFLAGS=`sdl2-config --cflags` -g -Wall -std=gnu11 -O3
LDLIBS=`sdl2-config --libs`

$(P): $(OBJECTS)

# index.js:
# 	emcc game.c -s WASM=1 -s USE_SDL=2 -sUSE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 --preload-file resources -o index.js

wasm:
	EMSDK_QUIET=1 source "/Users/flav/source/otherstuff/languages/c/wasm/emsdk/emsdk_env.sh"
	emcc game.c -s WASM=1 -s USE_SDL=2 -sUSE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -o index.js
