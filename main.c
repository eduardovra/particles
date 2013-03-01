#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "SDL.h"

/* Defines */
#define FRAMES_PER_SECOND	25
#define NUM_OF_PARTICLES	5
#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480

/* Typedefs */
typedef struct s_position {
	int x;
	int y;
} st_position;

typedef struct s_velocity {
	int x;
	int y;
} st_velocity;

/* Globals */
const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
bool game_is_running = true;
static st_position pos[NUM_OF_PARTICLES];
static st_velocity vel[NUM_OF_PARTICLES];

void DrawPixel(SDL_Surface *screen, Uint8 R, Uint8 G, Uint8 B, Sint32 x, Sint32 y)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
            return;
        }
    }
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

        case 3: { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *(bufp+screen->format->Rshift/8) = R;
            *(bufp+screen->format->Gshift/8) = G;
            *(bufp+screen->format->Bshift/8) = B;
        }
        break;

        case 4: { /* Probably 32-bpp */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
    if ( SDL_MUSTLOCK(screen) ) {
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, x, y, 1, 1);
}

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

uint64_t get_ticks(void)
{
	struct timespec ts;
	if ( clock_gettime(CLOCK_MONOTONIC, &ts) != 0 ) {
		abort();
	}
	return timespec_to_miliseconds(&ts);
}

void sleep_ticks(const uint64_t ticks)
{
	struct timespec ts = miliseconds_to_timespec(ticks);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

int get_random(int max)
{
	double random_number = rand() / (double) RAND_MAX;
	return random_number * max;
}

void update(int delta)
{
	static bool initialized = false;
	int i;
	
	if ( initialized == false ) {
		for (i = 0; i < NUM_OF_PARTICLES; i++) {
			pos[i].x = get_random(WINDOW_WIDTH);
			pos[i].y = get_random(WINDOW_HEIGHT);
			vel[i].x = get_random(20) - 10;
			vel[i].y = get_random(20) - 10;
		}
		initialized = true;
	}

	/* move */
	for (i = 0; i < NUM_OF_PARTICLES; i++) {
		pos[i].x += vel[i].x;
		pos[i].y += vel[i].y;

		if (pos[i].x < 0)
			pos[i].x = 0;
		if (pos[i].y < 0)
			pos[i].y = 0;

		/* collision detection and response*/
		if ((pos[i].x >= WINDOW_WIDTH) || pos[i].x <= 0)
			vel[i].x *= -1;
		if ((pos[i].y >= WINDOW_HEIGHT) || pos[i].y <= 0)
			vel[i].y *= -1;
	}
}

void draw(SDL_Surface * screen)
{
	int i;
#if 0
	/* clean screen */
	SDL_FillRect(screen, NULL, 0x000000);
	SDL_Flip(screen);
#endif
	/* draw all paticles */
	for(i = 0; i < NUM_OF_PARTICLES; i++) {
		DrawPixel(screen, 0xFF, 0xFF, 0xFF, pos[i].x, pos[i].y);
	}
}

void input(void)
{
	SDL_Event event;

	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {
			case SDL_QUIT:
				game_is_running = false;
				break;
			default:
				fprintf(stderr, "Unkown event %u\n", event.type);
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	SDL_Surface * screen;
	uint64_t next_game_tick, sleep_time;

	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_SWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
		exit(1);
	}

	next_game_tick = get_ticks();

	while ( game_is_running )
	{
		update(SKIP_TICKS);
		draw(screen);
		
		next_game_tick += SKIP_TICKS;
		sleep_time = next_game_tick - get_ticks();
		if ( sleep_time >= 0 ) {
			sleep_ticks( sleep_time );
		}
		else {
			// Shit, we are running behind!
			game_is_running = false;
		}

		input();
	}

	atexit(SDL_Quit);

	return 0;
}

