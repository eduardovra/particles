/*****************************************************************************
 * utils.h
 * Author: <eduardovra@gmail.com>
 * **************************************************************************/

#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#include "SDL.h"

uint64_t get_ticks(void);

void sleep_ticks(const uint64_t ticks);

float get_random(float min, float max);

#endif /* _UTILS_H_INCLUDED_ */
