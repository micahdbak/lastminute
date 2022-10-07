#ifndef SPRITE_H
#define SPRITE_H

struct sprite
{
	SDL_Surface *sheet;
	int width, height,
	    nCol, nRow,
	    xCenter, yCenter,
	    interval;
	float x, y;
	SDL_Rect rect;
	SDL_TimerID timer;
};

Uint32 updateSprite(Uint32 interval, void *param);
void loadSprite(char *path, struct sprite *sprite, int width, int height);
void nextFrame(struct sprite *sprite);
void renderSprite(struct sprite *sprite, SDL_Surface *surface);

#endif
