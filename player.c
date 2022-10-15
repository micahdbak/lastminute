#include <math.h>

#include "engine.h"

static float diagonalMultiplier;
static Mix_Chunk *footstep;

void playerStart(struct object *object, char *param)
{
	int xStart, yStart;

	diagonalMultiplier = 1.0 / sqrt(2.0);

	loadSprites("player.bmp", object, 16, 16);
	object->xCenter = 8, object->yCenter = 12;

	sscanf(param, "%d,%d", &xStart, &yStart);

	object->x = (float)xStart;
	object->y = (float)yStart;

	footstep = Mix_LoadWAV("footStep.wav");
}

void playerRoutine(struct object *object, struct world *world)
{
	static int currentSprite = 0;

	int xDir, yDir, newSprite, isRunning;
	float speed,
	      xMod, yMod;
	SDL_Rect propose;

	if (keyIsHit(world, KEY_BACK))
		Mix_PlayChannel(-1, footstep, 0);

	xDir = (keyIsDown(world, KEY_RIGHT) ? 1 : keyIsDown(world, KEY_LEFT) ? -1 : 0);
	yDir = (keyIsDown(world, KEY_DOWN) ? 1 : keyIsDown(world, KEY_UP) ? -1 : 0);

	newSprite = (yDir * 3) + xDir + 4;
	/*  	up	no	down
	left	0	3	6
	no	1	4	7
	right	2	5	8 */

	isRunning = keyIsDown(world, KEY_BACK);

	if (newSprite != 4 && newSprite != currentSprite)
	{
		currentSprite = newSprite;
		setSprite(object, newSprite);
	}

	if (xDir == 0 && yDir == 0)
	{
		object->isPaused = 1;
		object->sprite.x = 0;
	} else {
		object->isPaused = 0;

		if (isRunning)
		{
			speed = 64.0f;
			setSpriteSpeed(object, 8);
		} else {
			speed = 32.0f;
			setSpriteSpeed(object, 4);
		}

		if (xDir != 0 && yDir != 0)
			speed *= diagonalMultiplier;

		xMod = world->delta * speed * xDir;
		yMod = world->delta * speed * yDir;

		propose.x = (int)(object->x + xMod) - object->xCenter;
		propose.y = (int)(object->y + yMod) - object->yCenter;
		propose.w = object->sprite.w;
		propose.h = object->sprite.h;

		/*
		if (!isCollision(world, &propose))
		{
		*/
			object->x += xMod;
			object->y += yMod;
		/*
		}
		*/

		setView(world, (int)object->x, (int)object->y);
	}

	queueSprite(world, object);
}