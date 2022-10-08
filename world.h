#ifndef WORLD_H
#define WORLD_H

#include "SDL.h"

#define KEY_UP		0b00000001
#define KEY_RIGHT	0b00000010
#define KEY_DOWN	0b00000100
#define KEY_LEFT	0b00001000
#define KEY_SPECIAL	0b00010000
#define KEY_MENU	0b00100000
#define KEY_SELECT	0b01000000
#define KEY_BACK	0b10000000

struct world
{
	struct tileSet
	{
		SDL_Surface *imageData;
		int width, height,
		    nCol, nRow;
	} tileSet;
	struct map
	{
		SDL_Rect **mapData;
		int nCol, nRow;
	} map;
	SDL_Surface *draw;
	int drawWidth, drawHeight;
	float delta;
	Uint32 keyMap;
};

void loadWorld(char *path, struct world *world);
void freeWorld(struct world *world);
void loopWorld(struct world *world, SDL_Surface *screen);

#endif
