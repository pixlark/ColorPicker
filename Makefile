ifdef debug
	debug_opt=-g
endif

release:
	@echo "Building for Linux..."
	gcc $(debug_opt) \
		main.c \
		-lSDL2 -lSDL2_ttf \
		-o picker
