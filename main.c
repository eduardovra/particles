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
#define NUM_OF_PARTICLES	1
#define PARTICLES_MAX_VEL	0.5
#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		640
#define SCREEN_MARGIN		200

//#define PIXEL_MODE
#define CIRCLE_MODE

/* Macros */
#define NUM_OF_PLANES	( sizeof(__planes) / sizeof(__planes[0]) )

/* Typedefs */
typedef struct s_position {
	float x;
	float y;
} st_position;

typedef struct s_velocity {
	float x;
	float y;
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

typedef struct s_point {
	float x;
	float y;
} st_point;

typedef struct s_line {
	st_point p1;
	st_point p2;
} st_line;

typedef struct s_plane {
	float a;	/*! normal vector x */
	float b;	/*! normal vector y */
	float c;	/*! normal vector distance to the origin */
	st_line line;
} st_plane;

/* Globals */
const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
st_particle particles[NUM_OF_PARTICLES];
bool game_is_running = true;
bool leave_trail = false;
bool rotate_planes = true;
const int screen_width = WINDOW_WIDTH - (2 * SCREEN_MARGIN);
const int screen_height = WINDOW_HEIGHT - (2 * SCREEN_MARGIN);
const int x_sup_lim = WINDOW_WIDTH - SCREEN_MARGIN;
const int y_sup_lim = WINDOW_HEIGHT - SCREEN_MARGIN;
const int x_inf_lim = SCREEN_MARGIN;
const int y_inf_lim = SCREEN_MARGIN;
const float rotation_step = (M_PI / 180.0) * 10;

st_plane __planes[] = {
	{ .a = 1, .b = 0, .c = 1, .line.p1.x = 1, .line.p1.y = 1, .line.p2.x = 1, .line.p2.y = -1 },
	{ .a = 0, .b = -1, .c = 1, .line.p1.x = -1, .line.p1.y = -1, .line.p2.x = 1, .line.p2.y = -1 },
	{ .a = -1, .b = 0, .c = 1, .line.p1.x = -1, .line.p1.y = 1, .line.p2.x = -1, .line.p2.y = -1 },
	{ .a = 0, .b = 1, .c = 1, .line.p1.x = -1, .line.p1.y = 1, .line.p2.x = 1, .line.p2.y = 1 },
};

Sint32 screen_x(float _x) {
	Sint32 x = _x * (WINDOW_WIDTH - SCREEN_MARGIN) * 0.5;
	x = x + WINDOW_WIDTH * 0.5;
	return x;
}

Sint32 screen_y(float _y) {
	Sint32 y = _y * (WINDOW_HEIGHT - SCREEN_MARGIN) * 0.5;
	y = y + WINDOW_HEIGHT * 0.5;
	return y;
}

void rotate_point(st_point * point, const float angle)
{
	float x = point->x, y = point->y;

	point->x = (x * cos(angle)) - (y * sin(angle));
	point->y = (x * sin(angle)) + (y * cos(angle));
}

void updatePlanes(const float delta)
{
	int i;
	const float angle = rotation_step * delta;

	for (i = 0; i < NUM_OF_PLANES; i++) {
		__planes[i].a = (__planes[i].a * cos(angle)) - (__planes[i].b * sin(angle));
		__planes[i].b = (__planes[i].a * sin(angle)) + (__planes[i].b * cos(angle));

		rotate_point(&__planes[i].line.p1, angle);
		rotate_point(&__planes[i].line.p2, angle);
	}
}

void update(const float delta)
{
	int i, j;

	/* apply plane rotation */
	if ( rotate_planes )
		updatePlanes(delta);

	for (i = 0; i < NUM_OF_PARTICLES; i++) {
		/* move */
		particles[i].pos.x += particles[i].vel.x * delta;
		particles[i].pos.y += particles[i].vel.y * delta;

		/* collision detection and response*/
		for (j = 0; j < NUM_OF_PLANES; j++) {
			float distance = (particles[i].pos.x * __planes[j].a) + (particles[i].pos.y * __planes[j].b) + __planes[j].c;
			float norm_vel = (particles[i].vel.x * __planes[j].a) + (particles[i].vel.y * __planes[j].b);

			if ( (distance < 0) && (norm_vel < 0) ) {
				fprintf(stderr, "particle %d collided on plane %d\n", i, j);
				particles[i].vel.x -= 2 * __planes[j].a * ((particles[i].vel.x * __planes[j].a) + (particles[i].vel.y * __planes[j].b));
				particles[i].vel.y -= 2 * __planes[j].b * ((particles[i].vel.x * __planes[j].a) + (particles[i].vel.y * __planes[j].b));
			}
		}
	}
}

void drawPlanes(SDL_Surface * screen)
{
	/* draw 4 lines of a square */
	const Uint32 color = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0x00);
	int i, size = sizeof(__planes) / sizeof(__planes[0]);

	for (i = 0; i < size; i++) {
		drawLine(screen,
				screen_x(__planes[i].line.p1.x), screen_y(__planes[i].line.p1.y),
				screen_x(__planes[i].line.p2.x), screen_y(__planes[i].line.p2.y),
				color);
	}
}

void draw(SDL_Surface * screen)
{
	//Uint32 color = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0x00);
	int i;

	/* clear screen */
	if ( leave_trail == false )
		SDL_FillRect(screen, NULL, 0);

	if ( SDL_MUSTLOCK(screen) ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			return;
		}
	}
	/* draw border planes */
	drawPlanes(screen);

	/* draw all paticles */
	for(i = 0; i < NUM_OF_PARTICLES; i++) {
		Uint32 par_color = SDL_MapRGB(screen->format,
				particles[i].color.R, particles[i].color.G, particles[i].color.B);
#if defined (PIXEL_MODE)
		DrawPixel(screen, screen_x(particles[i].pos.x), screen_y(particles[i].pos.y), par_color);
#elif defined (CIRCLE_MODE)
		fill_circle(screen, screen_x(particles[i].pos.x), screen_y(particles[i].pos.y), 12, 0xff000000 + par_color);
		draw_circle(screen, screen_x(particles[i].pos.x), screen_y(particles[i].pos.y), 12, 0xffffffff);
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
		particles[i].pos.x = get_random_float(-1, 1);
		particles[i].pos.y = get_random_float(-1, 1);
		/* velocity */
		particles[i].vel.x = get_random_float(-PARTICLES_MAX_VEL, PARTICLES_MAX_VEL);
		particles[i].vel.y = get_random_float(-PARTICLES_MAX_VEL, PARTICLES_MAX_VEL);
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
		update(SKIP_TICKS / 1000.0);
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
