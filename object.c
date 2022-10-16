#include "engine.h"

Uint32 updateSprite(Uint32 interval, void *param)
{
	struct object *object = (struct object *)param;

	if (!object->isPaused)
	{
		object->sprite.x += object->sprite.w;

		if (object->sprite.x >= object->sheet->w)
			object->sprite.x = 0;
	}

	return (Uint32)object->interval != interval ? object->interval : interval;
}

void loadSprites(const char *path, struct object *object, int width, int height)
{
	object->sheet = SDL_LoadBMP(path);
	SDL_SetColorKey(object->sheet, SDL_TRUE, SDL_MapRGB(object->sheet->format, 0xff, 0, 0xff));

	object->x = 0.0f;
	object->y = 0.0f;

	object->nCol = object->sheet->w / width;
	object->nRow = object->sheet->h / height;

	object->xCenter = width / 2;
	object->yCenter = height / 2;

	object->interval = 1000;
	object->isPaused = 0;
	object->raise = 0;

	object->sprite.x = 0;
	object->sprite.y = 0;
	object->sprite.w = width;
	object->sprite.h = height;

	object->collider = object->sprite;

	object->timer = SDL_AddTimer(object->interval, updateSprite, (void *)object);
}

void setSprite(struct object *object, int row)
{
	object->sprite.y = object->sprite.h * row;
}

void setSpriteSpeed(struct object *object, int framesPerSecond)
{
	object->interval = 1000 / framesPerSecond;
}

void freeObject(struct object *object)
{
	SDL_FreeSurface(object->sheet);
	SDL_RemoveTimer(object->timer);
}
