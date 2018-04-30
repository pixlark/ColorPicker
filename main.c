// RGB/HSV conversions by David H from https://stackoverflow.com/questions/3018313/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Embed DejaVuSans in executable through header file bigass array
#include "dejavu.h"

// A buttload of magic numbers for positioning stuff on the
// screen. It's ugly, but it makes things clear.
#define SCREEN_WIDTH        (512)
#define SCREEN_HEIGHT       (600)
#define MAIN_GRADIENT_SIZE  (SCREEN_WIDTH)
#define SAMPLE_BOX_SIZE     (SCREEN_HEIGHT - SCREEN_WIDTH)
#define HUE_GRADIENT_WIDTH  (SCREEN_WIDTH  - SAMPLE_BOX_SIZE)
#define HUE_GRADIENT_HEIGHT (SAMPLE_BOX_SIZE / 2)
#define HUE_SLIDER_HEIGHT   (HUE_GRADIENT_HEIGHT)
#define HUE_SLIDER_WIDTH    (3)
#define TEXT_PADDING        (10)

// Start of David H's conversion code
typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} RGBColor;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} HSVColor;

HSVColor rgb_to_hsv(RGBColor in)
{
    HSVColor         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


RGBColor hsv_to_rgb(HSVColor in)
{
    double      hh, p, q, t, ff;
    long        i;
    RGBColor         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}
// End of David H's conversion code

static TTF_Font * default_font;

typedef struct {
	uint32_t buttons;
	int x;
	int y;
} MouseState;

enum UIState {
	UI_NONE,
	UI_SLIDER_CHANGE,
	UI_GRADIENT_CHANGE,
};

int clamp(int lower, int higher, int num)
{
	if (num < lower) {
		return lower;
	} else if (num > higher) {
		return higher;
	} else {
		return num;
	}
}

bool point_in_rect(SDL_Rect rect, int x, int y)
{
	return
		x > rect.x && x < rect.x + rect.w &&
		y > rect.y && y < rect.y + rect.h;
}

SDL_Color from_RGBColor(RGBColor rgb_color)
{
	SDL_Color color = {
		rgb_color.r * 255,
		rgb_color.g * 255,
		rgb_color.b * 255,
		0xff
	};
	return color;
}

void set_pixel(SDL_Surface * surface, SDL_Color color, int x, int y)
{
	((char*)surface->pixels)[(x + y * surface->w) * 4    ] = color.r;
	((char*)surface->pixels)[(x + y * surface->w) * 4 + 1] = color.g;
	((char*)surface->pixels)[(x + y * surface->w) * 4 + 2] = color.b;
	((char*)surface->pixels)[(x + y * surface->w) * 4 + 3] = color.a;
}

void draw_surface_as_texture(SDL_Renderer * renderer, SDL_Surface * surface, int x, int y)
{
	SDL_Texture * render_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect render_rect = {x, y, surface->w, surface->h};
	SDL_RenderCopy(renderer, render_texture, NULL, &render_rect);
	SDL_DestroyTexture(render_texture);
}

void draw_gradient(SDL_Renderer * renderer, double hue)
{
	SDL_Surface * surface = SDL_CreateRGBSurface(
		0, MAIN_GRADIENT_SIZE, MAIN_GRADIENT_SIZE, 32,
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			HSVColor hsv_color = {
				hue,
				(double) x / surface->w,
				1.0 - ((double) y / surface->h),
			};
			set_pixel(surface, from_RGBColor(hsv_to_rgb(hsv_color)), x, y);
		}
	}
	
	draw_surface_as_texture(renderer, surface, 0, 0);
	SDL_FreeSurface(surface);
}

void draw_hue_gradient(SDL_Renderer * renderer)
{
	SDL_Surface * surface = SDL_CreateRGBSurface(
		0, HUE_GRADIENT_WIDTH, HUE_GRADIENT_HEIGHT, 32,
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			HSVColor hsv_color = {
				(double) x / surface->w * 360,
				1.0, 1.0,
			};
			set_pixel(surface, from_RGBColor(hsv_to_rgb(hsv_color)), x, y);
		}
	}

	draw_surface_as_texture(renderer, surface, 0, MAIN_GRADIENT_SIZE);
	SDL_FreeSurface(surface);	
}

void draw_hue_slider(SDL_Renderer * renderer, double hue)
{
	SDL_Rect draw_rect = {
		((hue / 360.0) * HUE_GRADIENT_WIDTH) - (HUE_SLIDER_WIDTH / 2),
		MAIN_GRADIENT_SIZE,
		HUE_SLIDER_WIDTH,
		HUE_SLIDER_HEIGHT,
	};
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xaa);
	SDL_RenderFillRect(renderer, &draw_rect);
}

void draw_sample_box(SDL_Renderer * renderer, SDL_Color color)
{
	SDL_Rect draw_rect = {
		HUE_GRADIENT_WIDTH,
		MAIN_GRADIENT_SIZE,
		SAMPLE_BOX_SIZE,
		SAMPLE_BOX_SIZE,
	};
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xff);
	SDL_RenderFillRect(renderer, &draw_rect);
}

void draw_text(SDL_Renderer * renderer, char * text, int x, int y)
{
	SDL_Color draw_color = {0xff, 0xff, 0xff, 0xff};
	SDL_Surface * surface = TTF_RenderText_Solid(default_font, text, draw_color);
	SDL_Rect draw_rect = {x, y, surface->w, surface->h};
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_RenderCopy(renderer, texture, NULL, &draw_rect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void draw_info_text(SDL_Renderer * renderer, HSVColor hsv_color)
{
	RGBColor rgb_color = hsv_to_rgb(hsv_color);
	char rgb_buffer[128];
	sprintf(rgb_buffer, "R: %d G: %d B: %d",
		(int) (rgb_color.r * 256),
		(int) (rgb_color.g * 256),
		(int) (rgb_color.b * 256));
	draw_text(renderer, rgb_buffer,
		TEXT_PADDING,
		MAIN_GRADIENT_SIZE + HUE_GRADIENT_HEIGHT + TEXT_PADDING);
	
	char hsv_buffer[128];
	sprintf(hsv_buffer, "H: %d S: %d V: %d",
		(int)  hsv_color.h,
		(int) (hsv_color.s * 100),
		(int) (hsv_color.v * 100));
	draw_text(renderer, hsv_buffer,
		TEXT_PADDING + (HUE_GRADIENT_WIDTH / 2),
		MAIN_GRADIENT_SIZE + HUE_GRADIENT_HEIGHT + TEXT_PADDING);
}

void draw_position_indicator(SDL_Renderer * renderer, HSVColor color)
{
	char heavy = 0xaa;
	char light = 0x88;
	char circle[5 * 5] = {
		0x00 , light, heavy, light, 0x00 ,
		light, heavy, heavy, heavy, light,
		heavy, heavy, heavy, heavy, heavy,
		light, heavy, heavy, heavy, light,
		0x00 , light, heavy, light, 0x00 ,
	};
	SDL_Surface * surface = SDL_CreateRGBSurface(
		0, 5, 5, 32,
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			SDL_Color color = {0xff, 0xff, 0xff, circle[x + y * 5]};
			set_pixel(surface, color, x, y);
		}
	}
	draw_surface_as_texture(renderer, surface,
		       color.s  * MAIN_GRADIENT_SIZE - 3,
		(1.0 - color.v) * MAIN_GRADIENT_SIZE - 3);
	SDL_FreeSurface(surface);
}

/* Elements that get rendered:
 *   [x] main gradient
 *   [x] hue gradient
 *   [x] slider bar on hue gradient
 *   [x] sampling box
 *   [x] position indicator
 *   [x] info text
 */

enum UIState get_click_state(MouseState m)
{
	SDL_Rect main_gradient = {
		0, 0, MAIN_GRADIENT_SIZE, MAIN_GRADIENT_SIZE,
	};
	SDL_Rect slider_gradient = {
		0, MAIN_GRADIENT_SIZE,
		HUE_GRADIENT_WIDTH, HUE_GRADIENT_HEIGHT,
	};
	if (point_in_rect(main_gradient, m.x, m.y)) {
		return UI_GRADIENT_CHANGE;
	} else if (point_in_rect(slider_gradient, m.x, m.y)) {
		return UI_SLIDER_CHANGE;
	} else {
		return UI_NONE;
	}
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	SDL_Window * window = SDL_CreateWindow(
		"Color Picker",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_RWops * dejavu_mem = SDL_RWFromMem(DejaVuSans_ttf, DejaVuSans_ttf_len);
	if (!dejavu_mem) {
		printf("Couldn't open font file.\n");
		return 1;
	}
	default_font = TTF_OpenFontRW(dejavu_mem, 0, 16);
	if (!default_font) {
		printf("Font memory couldn't be processed as TTF_Font*:\n\t%s\n", TTF_GetError());
		return 1;
	}
	
	HSVColor current_color = {180.0, 1.0, 1.0};

	enum UIState ui_state = UI_NONE;
	
	SDL_Event event;
	bool running = true;
	while (running) {
		MouseState m;
		m.buttons = SDL_GetMouseState(&m.x, &m.y);
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
				case SDL_BUTTON_LEFT:
					ui_state = get_click_state(m);
					SDL_CaptureMouse(true);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				ui_state = UI_NONE;
				SDL_CaptureMouse(false);
				break;
			}
			
		}
		// input
		// hue slider
		switch (ui_state) {
		case UI_GRADIENT_CHANGE: {
			current_color.s = ((double) clamp(0, MAIN_GRADIENT_SIZE, m.x) / MAIN_GRADIENT_SIZE);
			current_color.v = 1.0 - ((double) clamp(0, MAIN_GRADIENT_SIZE, m.y) / MAIN_GRADIENT_SIZE);
		}   break;
		case UI_SLIDER_CHANGE:
			current_color.h = ((double) clamp(0, HUE_GRADIENT_WIDTH, m.x) / HUE_GRADIENT_WIDTH) * 360.0;
			break;
		}
		
		// drawing
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
		SDL_RenderClear(renderer);
		draw_gradient(renderer, current_color.h);
		draw_hue_gradient(renderer);
		draw_hue_slider(renderer, current_color.h);
		draw_info_text(renderer, current_color);
		draw_sample_box(renderer, from_RGBColor(hsv_to_rgb(current_color)));
		draw_position_indicator(renderer, current_color);
		SDL_RenderPresent(renderer);
		SDL_Delay(8);
	}
	
	return 0;
}
