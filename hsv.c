/** COPYRIGHT (C) 2017
 ** https://pixlark.github.io/
 *
 ** main.c
 * 
 * This file contains a basic color picking program for the HSV color
 * space.
 *
 */

#if defined(_WIN32) || defined(_WIN64)
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <Windows.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif

#include <pixcolor.h>
#include <pixint.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 600

enum interaction {
	NONE,
	GRADIENT,
	SLIDER
};

void error_quit(char * message) {
	fprintf(stderr, "%s SDL_Error:\n\t%s\n", message, SDL_GetError());
	exit(1);
}

void strip_dir(char * dir) {
	for (u16 i = strlen(dir) - 1; i >= 0; i--) {
		if (dir[i] == '/') {
			dir[i + 1] = '\0';
			return;
		}
	}
}

void draw_gradient(SDL_Surface * screen_surface, u8 slider_value) {
	/* 1. Create a surface of cell_size
	 * 2. Fill it with desired colour
	 * 3. SDL_Rect for the location
	 * 4. Blit surface to screen
	 */
	u16 cell_size = SCREEN_WIDTH/256;
	SDL_Surface * square_surface =
		SDL_CreateRGBSurface(0, cell_size, cell_size, 32, 0, 0, 0, 0);
	u8 r = 0;
	do {
		u8 g = 0;
		do {
			HSVColor print_hsv; print_hsv.h = slider_value; print_hsv.s = r; print_hsv.v = g;
			RGBColor print_rgb = HSVtoRGB(print_hsv);
			SDL_FillRect(square_surface, 0,
				SDL_MapRGB(square_surface->format, print_rgb.r, print_rgb.g, print_rgb.b));
			SDL_Rect this_rect;
			this_rect.x = r*cell_size;
			this_rect.w = cell_size;
			this_rect.y = g*cell_size;
			this_rect.h = cell_size;
			SDL_BlitSurface(square_surface, 0, screen_surface, &this_rect);
		} while (g++ != 255);
	} while (r++ != 255);
	SDL_FreeSurface(square_surface);
}

#if defined(_WIN32) || defined(_WIN64)
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow) {
#else
int main(int argc, char * argv[]) {
#endif
	strip_dir(*argv);
	SDL_Window * window;
	SDL_Surface * screen_surface;

	/* INITIALIZE */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		error_quit("Trouble initializing.");
	}
	window = SDL_CreateWindow(
		"Color Picker",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	screen_surface = SDL_GetWindowSurface(window);
	if (screen_surface == 0) {
		error_quit("WTF WTF WTF");
	}
	
	/* PICKED COLOR SQUARE */
	u16 picked_color_size = SCREEN_HEIGHT - SCREEN_WIDTH;
	SDL_Surface * picked_color_surface = SDL_CreateRGBSurface(
		0, picked_color_size, picked_color_size, 32, 0, 0, 0, 0);
	SDL_Rect picked_color_rect;
	picked_color_rect.x = SCREEN_WIDTH - picked_color_size;
	picked_color_rect.w = picked_color_size;
	picked_color_rect.y = SCREEN_HEIGHT - picked_color_size;
	picked_color_rect.h = picked_color_size;

	/* COLOR SLIDER */
	u16 slider_width = SCREEN_WIDTH - picked_color_size;
	u16 slider_height = picked_color_size / 2;
	char * bmp_name = "hue.bmp";
	char * bmp_path = malloc(sizeof(char) * (strlen(*argv) + strlen(bmp_name) - 1));
	strcpy(bmp_path, *argv);
	strcat(bmp_path, bmp_name);
	SDL_Surface * slider_bg_surface = SDL_LoadBMP(bmp_path);
	SDL_Rect slider_bg_rect;
	slider_bg_rect.x = 0;
	slider_bg_rect.w = slider_width;
	slider_bg_rect.y = SCREEN_WIDTH;
	slider_bg_rect.h = picked_color_size / 2;
	SDL_Surface * slider_bar_surface = SDL_CreateRGBSurface(
		0, 3, slider_height, 32, 0, 0, 0, 0);
	SDL_FillRect(slider_bar_surface, 0, SDL_MapRGB(slider_bar_surface->format, 255, 255, 255));
	u8 slider_value = 127;
	SDL_Rect slider_bar_rect;
	slider_bar_rect.x = ((slider_value * slider_width) / 255) - 1;
	slider_bar_rect.y = SCREEN_WIDTH;

	/* TEXT */
	TTF_Init();
	char * font_name = "DejaVuSans.ttf";
	char * font_path = malloc(sizeof(char) * (strlen(*argv) + strlen(font_name) - 1));
	strcpy(font_path, *argv);
	strcat(font_path, font_name);
	TTF_Font * default_font = TTF_OpenFont(font_path, 16);
	if (default_font == 0) {
		fprintf(stderr, "Trouble loading font. TTF_Error:\n\t%s\n", TTF_GetError());
		exit(1);
	}
	SDL_Color font_color;
	font_color.r = 255;
	font_color.g = 255;
	font_color.b = 255;
	font_color.a = 255;
	SDL_Surface * rgb_color_text_surface =
		TTF_RenderText_Solid(default_font, "...", font_color);
	if (rgb_color_text_surface == 0) {
		fprintf(stderr, "Trouble creating surface from font. TTF_Error\n\t%s\n", TTF_GetError());
		exit(1);
	}
	SDL_Rect rgb_color_text_rect;
	rgb_color_text_rect.x = 10;
	rgb_color_text_rect.y = SCREEN_WIDTH + slider_height + 10;
	rgb_color_text_rect.w = 0; // junk
	rgb_color_text_rect.h = 0; // values
	SDL_Surface * hsv_color_text_surface =
		TTF_RenderText_Solid(default_font, "...", font_color);
	if (hsv_color_text_surface == 0) {
		fprintf(stderr, "Trouble creating surface from font. TTF_Error\n\t%s\n", TTF_GetError());
		exit(1);
	}
	SDL_Rect hsv_color_text_rect;
	hsv_color_text_rect.x = 10 + (slider_width / 2);
	hsv_color_text_rect.y = SCREEN_WIDTH + slider_height + 10;
	hsv_color_text_rect.w = 0; // junk
	hsv_color_text_rect.h = 0; // values

	/* COLOR */
	RGBColor current_rgb; current_rgb.r = 0; current_rgb.g = 0; current_rgb.b = 0;
	HSVColor current_hsv; current_hsv.h = 0; current_hsv.s = 0; current_hsv.v = 0;
	
	u8 running = 0xFF;
	u8 current_interaction = NONE;
	SDL_Event event;
	while (running) {
		u8 color_changed = 0x00;
		/* PROCESS EVENTS */
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT: {
					running = 0x00;
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					int mouse_x = 0;
					int mouse_y = 0;
					SDL_GetMouseState(&mouse_x, &mouse_y);
					if (mouse_y < SCREEN_WIDTH) {
						current_interaction = GRADIENT;
					}
					if (mouse_y > SCREEN_WIDTH &&
						mouse_y < SCREEN_WIDTH + slider_height &&
						mouse_x < slider_width) {
						current_interaction = SLIDER;
					}
				} break;
				case SDL_MOUSEBUTTONUP: {
					current_interaction = NONE;
				} break;
			}
		}
		/* MOUSE */
		if (current_interaction != NONE) {
			int mouse_x = 0;
			int mouse_y = 0;
			SDL_GetMouseState(&mouse_x, &mouse_y);
			if (current_interaction == GRADIENT) {
				current_hsv.s = mouse_x / (SCREEN_WIDTH/256);
				current_hsv.v = mouse_y / (SCREEN_WIDTH/256);
				current_hsv.h = slider_value;
				current_rgb = HSVtoRGB(current_hsv);
				SDL_FillRect(picked_color_surface, 0,
							 SDL_MapRGB(picked_color_surface->format,
										current_rgb.r, current_rgb.g, current_rgb.b));
				if (picked_color_surface == 0) {
					error_quit("Picked colour surface is null.");
				}
				color_changed = 0xFF;
			} else if (current_interaction == SLIDER) {
				if (mouse_x < 0) {
					slider_value = 0;
				} else if (mouse_x > slider_width) {
					slider_value = 255;
				} else {
					slider_value = (255 * mouse_x) / slider_width;
				}
				slider_bar_rect.x = ((slider_value * slider_width) / 255) - 1;
			}
		}
		/* TEXT */
		char * rgb_str = malloc(sizeof(char) * 64);
		sprintf(rgb_str, "R: %d G: %d B: %d",
			current_rgb.r, current_rgb.g, current_rgb.b);
		SDL_FreeSurface(rgb_color_text_surface);
		rgb_color_text_surface =
			TTF_RenderText_Solid(default_font, rgb_str, font_color);
		free(rgb_str);
		char * hsv_str = malloc(sizeof(char) * 64);
		sprintf(hsv_str, "H: %d S: %d V: %d",
			current_hsv.h, current_hsv.s, current_hsv.v);
		SDL_FreeSurface(hsv_color_text_surface);
		hsv_color_text_surface =
			TTF_RenderText_Solid(default_font, hsv_str, font_color);
		free(hsv_str);
		/* DRAW */
		SDL_FillRect(
			screen_surface, 0,
			SDL_MapRGB(screen_surface->format, 0, 0, 0));
		draw_gradient(screen_surface, slider_value);
		SDL_BlitSurface(
			slider_bg_surface, 0,
			screen_surface, &slider_bg_rect);
		SDL_BlitSurface(
			slider_bar_surface, 0,
			screen_surface, &slider_bar_rect);
		SDL_BlitSurface(
			picked_color_surface, 0,
			screen_surface, &picked_color_rect);
		SDL_BlitSurface(
			rgb_color_text_surface, 0,
			screen_surface, &rgb_color_text_rect);
		SDL_BlitSurface(
			hsv_color_text_surface, 0,
			screen_surface, &hsv_color_text_rect);
		SDL_UpdateWindowSurface(window);
	}

	TTF_CloseFont(default_font);
	TTF_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
