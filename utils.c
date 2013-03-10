/*****************************************************************************
 * utils.h
 * Author: <eduardovra@gmail.com>
 * **************************************************************************/

#include "utils.h"

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
float get_random(float min, float max)
{
	float range = ( max - min ) + 1;
	return ((rand() / (RAND_MAX + 1.0)) * range) + min;
}
