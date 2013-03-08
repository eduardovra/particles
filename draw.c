/*****************************************************************************
 * draw.c
 * Author: <eduardovra@gmail.com>
 * **************************************************************************/

#include <math.h>
#include "draw.h"

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
