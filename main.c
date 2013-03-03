/*****************************************************************************
 * main.c
 * Author: <eduardovra@gmail.com>
 * **************************************************************************/

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "SDL.h"

#ifdef WIN32
typedef int bool;
#define false 0
#define true 1
#else
#include <stdbool.h>
#endif

/* Defines */
#define FRAMES_PER_SECOND	30
#define NUM_OF_PARTICLES	5
#define PARTICLES_MAX_VEL	10
#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480
#define SCREEN_MARGIN		50

//#define PIXEL_MODE
#define CIRCLE_MODE

/* Typedefs */
typedef struct s_position {
	int x;
	int y;
} st_position;

typedef struct s_velocity {
	int x;
	int y;
} st_velocity;

typedef struct s_color {
	int R;
	int G;
	int B;
} st_color;

typedef struct s_particle {
	st_position pos;
	st_velocity vel;
	st_color color;
} st_particle;

/* Globals */
const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
st_particle particles[NUM_OF_PARTICLES];
bool game_is_running = true;
bool leave_trail = false;
const int screen_width = WINDOW_WIDTH - (2 * SCREEN_MARGIN);
const int screen_height = WINDOW_HEIGHT - (2 * SCREEN_MARGIN);
const int x_sup_lim = WINDOW_WIDTH - SCREEN_MARGIN;
const int y_sup_lim = WINDOW_HEIGHT - SCREEN_MARGIN;
const int x_inf_lim = SCREEN_MARGIN;
const int y_inf_lim = SCREEN_MARGIN;

void DrawPixel(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 color)
{
	switch (screen->format->BytesPerPixel) {
		case 1: { /* Assuming 8-bpp */
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
			*bufp = color;
		}
		break;

		case 2: { /* Probably 15-bpp or 16-bpp */
			Uint16 *bufp;

			bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
			*bufp = color;
		}
		break;
#if 0
		case 3: { /* Slow 24-bpp mode, usually not used */
			Uint8 *bufp;

			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
			*(bufp+screen->format->Rshift/8) = R;
			*(bufp+screen->format->Gshift/8) = G;
			*(bufp+screen->format->Bshift/8) = B;
		}
		break;
#endif
		case 4: { /* Probably 32-bpp */
			Uint32 *bufp;

			bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
			*bufp = color;
		}
		break;
	}
}

void drawLine(SDL_Surface *screen, Sint32 x0, Sint32 y0, Sint32 x1, Sint32 y1, Uint32 color)
{
	/* Bresenham's line algorithm */
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;){
		DrawPixel(screen, x0, y0, color);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void drawRect(SDL_Surface *screen, Sint32 x, Sint32 y, int width, int height, Uint32 color)
{
	/* draw the 4 arests */
	drawLine(screen, x, y, x + width, y, color);
	drawLine(screen, x, y, x, y + height, color);
	drawLine(screen, x + width, y, x + width, y + height, color);
	drawLine(screen, x, y + height, x + width, y + height, color);
}

void draw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel)
{
	// if the first pixel in the screen is represented by (0,0) (which is in sdl)
	// remember that the beginning of the circle is not in the middle of the pixel
	// but to the left-top from it:

	double error = (double)-radius;
	double x = (double)radius -0.5;
	double y = (double)0.5;
	double cx = n_cx - 0.5;
	double cy = n_cy - 0.5;

	while (x >= y)
	{
		DrawPixel(surface, (int)(cx + x), (int)(cy + y), pixel);
		DrawPixel(surface, (int)(cx + y), (int)(cy + x), pixel);

		if (x != 0)
		{
			DrawPixel(surface, (int)(cx - x), (int)(cy + y), pixel);
			DrawPixel(surface, (int)(cx + y), (int)(cy - x), pixel);
		}

		if (y != 0)
		{
			DrawPixel(surface, (int)(cx + x), (int)(cy - y), pixel);
			DrawPixel(surface, (int)(cx - y), (int)(cy + x), pixel);
		}

		if (x != 0 && y != 0)
		{
			DrawPixel(surface, (int)(cx - x), (int)(cy - y), pixel);
			DrawPixel(surface, (int)(cx - y), (int)(cy - x), pixel);
		}

		error += y;
		++y;
		error += y;

		if (error >= 0)
		{
			--x;
			error -= x;
			error -= x;
		}
	}
}

void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
	static const int BPP = 2;

	double dy;
	double r = (double)radius;

	for (dy = 1; dy <= r; dy += 1.0)
	{
		// This loop is unrolled a bit, only iterating through half of the
		// height of the circle.  The result is used to draw a scan line and
		// its mirror image below it.

		// The following formula has been simplified from our original.  We
		// are using half of the width of the circle because we are provided
		// with a center and we need left/right coordinates.

		double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
		int x = cx - dx;

		// Grab a pointer to the left-most pixel for each half of the circle
		Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
		Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;

		for (; x <= cx + dx; x++)
		{
			*(Uint32 *)target_pixel_a = pixel;
			*(Uint32 *)target_pixel_b = pixel;
			target_pixel_a += BPP;
			target_pixel_b += BPP;
		}
	}
}

#if 0
uint64_t timespec_to_miliseconds(const struct timespec * ts)
{
	return (ts->tv_sec * 1000) + (ts->tv_nsec / 1000000);
}

struct timespec miliseconds_to_timespec(const uint64_t ticks)
{
	struct timespec ts;
	ts.tv_sec = ticks / 1000;
	ts.tv_nsec = (ticks % 1000) * 1000000;
	return ts;
}
#endif

uint64_t get_ticks(void)
{
#if 0
	struct timespec ts;
	assert( clock_gettime(CLOCK_MONOTONIC, &ts) == 0 );
	return timespec_to_miliseconds(&ts);
#endif
	return SDL_GetTicks();
}

void sleep_ticks(const uint64_t ticks)
{
#if 0
	struct timespec ts = miliseconds_to_timespec(ticks);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
#endif
	SDL_Delay(ticks);
}

/* return a random value between [min,max] */
int get_random(int min, int max)
{
	int range = ( max - min ) + 1;
	return ((rand() / (RAND_MAX + 1.0)) * range) + min;
}

void update(int delta)
{
	int i;

	for (i = 0; i < NUM_OF_PARTICLES; i++) {
		/* move */
		particles[i].pos.x += particles[i].vel.x;
		particles[i].pos.y += particles[i].vel.y;

		/* collision detection and response*/
		if (particles[i].pos.x <= x_inf_lim) {
			particles[i].pos.x = x_inf_lim + 1;
			particles[i].vel.x *= -1;
		}
		if (particles[i].pos.x >= x_sup_lim) {
			particles[i].pos.x = x_sup_lim - 1;
			particles[i].vel.x *= -1;
		}
		if (particles[i].pos.y <= y_inf_lim){
			particles[i].pos.y = y_inf_lim + 1;
			particles[i].vel.y *= -1;
		}
		if (particles[i].pos.y >= y_sup_lim){
			particles[i].pos.y = y_sup_lim - 1;
			particles[i].vel.y *= -1;
		}
	}
}

void draw(SDL_Surface * screen)
{
	Uint32 color = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0x00);
	int i;

	/* clear screen */
	if ( leave_trail == false )
		SDL_FillRect(screen, NULL, 0);

	if ( SDL_MUSTLOCK(screen) ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			return;
		}
	}
	/* draw surrounding rectangle */
	drawRect(screen, SCREEN_MARGIN, SCREEN_MARGIN,
			screen_width, screen_height, color);

	/* draw all paticles */
	for(i = 0; i < NUM_OF_PARTICLES; i++) {
		Uint32 par_color = SDL_MapRGB(screen->format,
				particles[i].color.R, particles[i].color.G, particles[i].color.B);
#if defined (PIXEL_MODE)
		DrawPixel(screen, particles[i].pos.x, particles[i].pos.y, par_color);
#elif defined (CIRCLE_MODE)
		fill_circle(screen, particles[i].pos.x, particles[i].pos.y, 15, 0xff000000 + par_color);
		draw_circle(screen, particles[i].pos.x, particles[i].pos.y, 15, 0xffffffff);
#endif
	}
	if ( SDL_MUSTLOCK(screen) ) {
		SDL_UnlockSurface(screen);
	}

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void input(void)
{
	SDL_Event event;

	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_ACTIVEEVENT:
			case SDL_MOUSEMOTION:
				break;
			case SDL_QUIT:
				game_is_running = false;
				break;
			default:
				fprintf(stderr, "Unkown event %u\n", event.type);
				break;
		}
	}
}

void init_particles(st_particle * particles, ssize_t num_of_particles)
{
	int i;

	for (i = 0; i < NUM_OF_PARTICLES; i++) {
		/* position */
		particles[i].pos.x = get_random(x_inf_lim, x_sup_lim);
		particles[i].pos.y = get_random(y_inf_lim, y_sup_lim);
		/* velocity */
		particles[i].vel.x = get_random(-PARTICLES_MAX_VEL, PARTICLES_MAX_VEL);
		particles[i].vel.y = get_random(-PARTICLES_MAX_VEL, PARTICLES_MAX_VEL);
		/* color */
		particles[i].color.R = get_random(0, 0xFF);
		particles[i].color.G = get_random(0, 0xFF);
		particles[i].color.B = get_random(0, 0xFF);
	}
}

int main(int argc, char *argv[])
{
	SDL_Surface * screen;
	uint64_t next_game_tick, sleep_time, ticks_now;

	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	/* set the title bar */
	SDL_WM_SetCaption("Particles", "Particles");

	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_SWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set %dx%d video: %s\n",
				WINDOW_WIDTH, WINDOW_HEIGHT, SDL_GetError());
		exit(1);
	}

	/* seed pseudo random numbers generator */
	srand(time(NULL));

	/* init particles */
	init_particles(particles, NUM_OF_PARTICLES);

	next_game_tick = get_ticks();

	while ( game_is_running )
	{
		input();
		update(SKIP_TICKS);
		draw(screen);
		
		next_game_tick += SKIP_TICKS;
		ticks_now = get_ticks();
		sleep_time = ( next_game_tick > ticks_now ) ?
				next_game_tick - ticks_now : 0;
		if ( sleep_time >= 0 ) {
			sleep_ticks( sleep_time );
		}
		else {
			// Shit, we are running behind!
			game_is_running = false;
		}
	}

	atexit(SDL_Quit);

	return 0;
}
