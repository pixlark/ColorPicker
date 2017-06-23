/** COPYRIGHT (C) 2017
 ** https://pixlark.github.io/
 *
 ** main.c
 * 
 * This file contains a basic color picking program.
 *
 */

#if defined(_WIN32) || defined(_WIN64)
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <Windows.h>
#else
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#endif

#include <pixint.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 600

void error_quit(char * message) {
	fprintf(stderr, "%s SDL_Error:\n\t%s\n", message, SDL_GetError());
	exit(1);
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
			SDL_FillRect(square_surface, 0, SDL_MapRGB(
					square_surface->format, r, g, slider_value));
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
	SDL_Surface * slider_bg_surface = SDL_CreateRGBSurface(
		0, slider_width, slider_height, 32, 0, 0, 0, 0);
	SDL_FillRect(slider_bg_surface, 0,
		SDL_MapRGB(slider_bg_surface->format, 255, 255, 255));
	SDL_Rect slider_bg_rect;
	slider_bg_rect.x = 0;
	slider_bg_rect.w = slider_width;
	slider_bg_rect.y = SCREEN_WIDTH;
	slider_bg_rect.h = picked_color_size / 2;
	SDL_Surface * slider_surface = SDL_CreateRGBSurface(
		0, slider_width, slider_height, 32, 0, 0, 0, 0);
	u8 slider_value = 127;
  	SDL_FillRect(slider_surface, 0,
		SDL_MapRGB(slider_surface->format, 0, 0, slider_value));
	SDL_Rect slider_dst_rect;
	slider_dst_rect.x = 0;
	slider_dst_rect.y = SCREEN_WIDTH;
	slider_dst_rect.w = 0; // junk
	slider_dst_rect.h = 0; // values
	SDL_Rect slider_src_rect;
	slider_src_rect.x = 0; // junk
	slider_src_rect.y = 0; // values
	slider_src_rect.w = (slider_value * slider_width) / 256;
	slider_src_rect.h = picked_color_size / 2;

	/* TEXT */
	TTF_Init();
	TTF_Font * default_font = TTF_OpenFont("DejaVuSans.ttf", 16);
	if (default_font == 0) {
		fprintf(stderr, "Trouble loading font. TTF_Error:\n\t%s\n", TTF_GetError());
		exit(1);
	}
	SDL_Color font_color;
	font_color.r = 255;
	font_color.g = 255;
	font_color.b = 255;
	font_color.a = 255;
	SDL_Surface * color_text_surface =
		TTF_RenderText_Solid(default_font, "Click gradient to select color.", font_color);
	if (color_text_surface == 0) {
		fprintf(stderr, "Trouble creating surface from font. TTF_Error\n\t%s\n", TTF_GetError());
		exit(1);
	}
	SDL_Rect color_text_rect;
	color_text_rect.x = 10;
	color_text_rect.y = SCREEN_WIDTH + slider_height + 10;
	color_text_rect.w = 0; // junk
	color_text_rect.h = 0; // values

	/* COLOR */
	u8 current_r = 0;
	u8 current_g = 0;
	u8 current_b = 0;
	
	u8 running = 0xFF;
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
					if (mouse_y <= SCREEN_WIDTH) {
						/* GRADIENT CLICK */
						current_r = mouse_x / (SCREEN_WIDTH/256);
						current_g = mouse_y / (SCREEN_WIDTH/256);
						current_b = slider_value;
						SDL_FillRect(picked_color_surface, 0,
							SDL_MapRGB(picked_color_surface->format,
								current_r, current_g, current_b));
						if (picked_color_surface == 0) {
							error_quit("Picked colour surface is null.");
						}
						color_changed = 0xFF;
					} else if (
						mouse_y >= SCREEN_WIDTH &&
						mouse_y <= SCREEN_WIDTH + slider_height &&
						mouse_x <= slider_width) {
						/* SLIDER CLICK */
						slider_value = (256 * mouse_x) / slider_width;
						slider_src_rect.w = mouse_x;
						SDL_FillRect(slider_surface, 0,
							SDL_MapRGB(slider_surface->format, 0, 0, slider_value));
						if (slider_surface == 0) {
							error_quit("Slider surface is null.");
						}
					}
				} break;
			}
		}
		/* TEXT */
		char * rgb_str = malloc(sizeof(char) * 64);
		sprintf(rgb_str, "R: %d G: %d B: %d",
			current_r, current_g, current_b);
		SDL_FreeSurface(color_text_surface);
		color_text_surface =
			TTF_RenderText_Solid(default_font, rgb_str, font_color);
		free(rgb_str);
		/* DRAW */
		SDL_FillRect(
			screen_surface, 0,
			SDL_MapRGB(screen_surface->format, 0, 0, 0));
		draw_gradient(screen_surface, slider_value);
		SDL_BlitSurface(
			picked_color_surface, 0,
			screen_surface, &picked_color_rect);
		SDL_BlitSurface(
			slider_bg_surface, 0,
			screen_surface, &slider_bg_rect);
		SDL_BlitSurface( // Uninitialized value ?
			slider_surface, &slider_src_rect,
			screen_surface, &slider_dst_rect);
		SDL_BlitSurface(
			color_text_surface, 0,
			screen_surface, &color_text_rect);
		SDL_UpdateWindowSurface(window);
	}

	TTF_CloseFont(default_font);
	TTF_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
