#include "SDL.h"

#include "sprite.h"

Uint32 updateSprite(Uint32 interval, void *param)
{
	struct sprite *sprite = (struct sprite *)param;

	nextFrame(sprite);

	if ((Uint32)sprite->interval != interval)
		return sprite->interval;

	return interval;
}

void loadSprite(char *path, struct sprite *sprite, int width, int height)
{
	sprite->sheet = SDL_LoadBMP(path);

	sprite->width = width;
	sprite->height = height;

	sprite->nCol = sprite->sheet->w / width;
	sprite->nRow = sprite->sheet->h / height;

	sprite->xCenter = width / 2;
	sprite->yCenter = height / 2;

	sprite->rect.x = 0;
	sprite->rect.y = 0;
	sprite->rect.w = width;
	sprite->rect.h = height;

	sprite->x = 0.0f;
	sprite->y = 0.0f;

	sprite->interval = 1000;

	sprite->timer = SDL_AddTimer(sprite->interval, updateSprite, (void *)sprite);
}

void nextFrame(struct sprite *sprite)
{
	sprite->rect.x += sprite->width;

	if (sprite->rect.x >= sprite->sheet->w)
		sprite->rect.x = 0;
}

void renderSprite(struct sprite *sprite, SDL_Surface *surface)
{
	static SDL_Rect render;

	render.x = (int)sprite->x - sprite->xCenter;
	render.y = (int)sprite->y - sprite->yCenter;
	render.w = 4 * sprite->width;
	render.h = 4 * sprite->height;

	SDL_BlitScaled(sprite->sheet, &sprite->rect, surface, &render);
}
