#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "SDL.h"

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

void update(void)
{
	
}

void draw(void)
{
	
}

int main(int argc, char *argv[])
{
	SDL_Surface *screen;
	uint64_t next_game_tick, sleep_time;
	bool game_is_running = true;
	
	const int FRAMES_PER_SECOND = 25;
	const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
	
	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
		exit(1);
	}

	next_game_tick = get_ticks();

	while ( game_is_running )
	{
		update();
		draw();
		
		next_game_tick += SKIP_TICKS;
		sleep_time = next_game_tick - get_ticks();
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

