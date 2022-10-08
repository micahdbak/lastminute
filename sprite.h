#ifndef SPRITE_H
#define SPRITE_H

struct sprite
{
	SDL_Surface *sheet;
	int width, height,
	    nCol, nRow,
	    xCenter, yCenter,
	    interval,
	    isPaused;
	float x, y;
	SDL_Rect rect;
	SDL_TimerID timer;
};

void loadSprite(char *path, struct sprite *sprite, int width, int height);
void freeSprite(struct sprite *sprite);
void nextFrame(struct sprite *sprite);
void setSpriteRow(struct sprite *sprite, int row);
void setSpriteSpeed(struct sprite *sprite, int framesPerSecond);
void renderSprite(struct sprite *sprite, SDL_Surface *surface);

#endif
