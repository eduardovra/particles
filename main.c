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

#include "utils.h"
#include "draw.h"

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
