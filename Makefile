ifdef debug
	debug_opt=-g
endif

win_SDL2_path    =G:\.minlib\SDL2-2.0.7\x86_64-w64-mingw32
win_SDL2_ttf_path=G:\.minlib\SDL2_ttf-2.0.14\x86_64-w64-mingw32

win_freetype_path=G:\.minlib\freetype-2.9\objs\.libs
win_zlib_path    =G:\.minlib\zlib-1.2.11

win_sdl2-config_path="G:\.minlib\SDL2-2.0.7\x86_64-w64-mingw32\bin\sdl2-config"

win:
	@echo "Building for Windows..."
	gcc $(debug_opt) \
		main.c \
		-static \
		-I$(win_SDL2_path)\include \
		-I$(win_SDL2_ttf_path)\include \
		-L$(win_SDL2_path)\lib \
		-L$(win_SDL2_ttf_path)\lib \
		-L$(win_freetype_path) \
		-L$(win_zlib_path) \
		$(shell bash $(win_sdl2-config_path) --static-libs) \
		-lSDL2_ttf -lfreetype -lz \
		-o picker

nix:
	@echo "Building for Linux..."
	gcc $(debug_opt) \
		main.c \
		-lSDL2 -lSDL2_ttf \
		-o picker
