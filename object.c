#include "engine.h"

Uint32 updateSprite(Uint32 interval, void *param)
{
	struct sprites *sprites = (struct sprites *)param;

	if (!sprites->isPaused)
	{
		sprites->sprite.x += sprites->sprite.w;

		if (sprites->sprite.x >= sprites->sheet->w)
			sprites->sprite.x = 0;
	}

	return (Uint32)sprites->interval != interval ? sprites->interval : interval;
}

void loadSprites(const char *path, struct sprites *sprites, int width, int height, int xCenter, int yCenter)
{
	sprites->sheet = SDL_LoadBMP(path);
	SDL_SetColorKey(sprites->sheet, SDL_TRUE, SDL_MapRGB(sprites->sheet->format, 0xff, 0, 0xff));

	sprites->nCol = sprites->sheet->w / width;
	sprites->nRow = sprites->sheet->h / height;

	sprites->xCenter = xCenter;
	sprites->yCenter = yCenter;

	sprites->interval = 1000;
	sprites->isPaused = 0;

	sprites->sprite.x = 0;
	sprites->sprite.y = 0;
	sprites->sprite.w = width;
	sprites->sprite.h = height;

	sprites->timer = SDL_AddTimer(sprites->interval, updateSprite, (void *)sprites);
}

void setSprite(struct sprites *sprites, int row)
{
	sprites->sprite.y = sprites->sprite.h * row;
}

void setSpritesSpeed(struct sprites *sprites, int framesPerSecond)
{
	sprites->interval = 1000 / framesPerSecond;
}

void freeSprites(struct sprites *sprites)
{
	SDL_FreeSurface(sprites->sheet);
	SDL_RemoveTimer(sprites->timer);
}

float getDistance(float x1, float y1, float x2, float y2)
{
	float leg1, leg2;

	leg1 = fabs(x2 - x1);
	leg2 = fabs(y2 - y1);

	if (leg1 == 0)
		return leg2;
	if (leg2 == 0)
		return leg1;

	return sqrt(pow(leg1, 2) + pow(leg2, 2));
}

float getDirection(float x1, float y1, float x2, float y2)
{
	float numerator,
	      referenceAngle;

	numerator = y2 - y1;

	if (x2 == x1)
		return numerator < 0.0f ? 3.0 * M_PI_2 : M_PI_2;

	referenceAngle = atan2(y2 - y1, x2 - x1);

	if (referenceAngle < 0.0f)
		return (2.0 * M_PI) + referenceAngle;
	else
		return referenceAngle;
}

void makeVector(float distance, float direction, float *x, float *y)
{
	*x = distance * cos(direction);
	*y = distance * sin(direction);
}
