r:
	gcc rgb.c -lSDL2 -lSDL2_ttf -o build/linux/rgbpicker
	gcc hsv.c -lSDL2 -lSDL2_ttf -o build/linux/hsvpicker
d:
	gcc rgb.c -g -lSDL2 -lSDL2_ttf -o build/linux/rgbpicker
	gcc hsv.c -g -lSDL2 -lSDL2_ttf -o build/linux/hsvpicker
