/*****************************************************************************
 * draw.h
 * Author: <eduardovra@gmail.com>
 * **************************************************************************/

#ifndef _DRAW_H_INCLUDED_
#define _DRAW_H_INCLUDED_

#include "SDL.h"
#include "SDL/SDL_ttf.h"

void DrawPixel(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 color);

void drawLine(SDL_Surface *screen, Sint32 x0, Sint32 y0, Sint32 x1, Sint32 y1, Uint32 color);

void drawRect(SDL_Surface *screen, Sint32 x, Sint32 y, int width, int height, Uint32 color);

void draw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel);

void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel);

int RenderTextToSurface(char * Text, int x, int y, SDL_Surface *Dest);

#endif /* _DRAW_H_INCLUDED_ */
