#include <math.h>

#include "engine.h"

static float diagonalMultiplier;

void playerStart(struct object *object, char *param)
{
	int xStart, yStart;

	diagonalMultiplier = 1.0 / sqrt(2.0);

	loadSprites("player.bmp", &object->sprites, 16, 16, 8, 16);

	object->collider.x = -6;
	object->collider.y = -4;
	object->collider.w = 12;
	object->collider.h = 4;

	sscanf(param, "%d,%d", &xStart, &yStart);

	object->x = (float)xStart;
	object->y = (float)yStart;
}

void playerRoutine(struct object *object, struct world *world)
{
	static int currentSprite = 0;

	int xDir, yDir,
	    newSprite,
	    isRunning;
	float speed,
	      xMod, yMod;
	SDL_Rect textArea = { 4, 4, 256, 0 };

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
		setSprite(&object->sprites, newSprite);
	}

	if (xDir == 0 && yDir == 0)
	{
		object->sprites.isPaused = 1;
		object->sprites.sprite.x = 0;
	} else {
		object->sprites.isPaused = 0;

		if (isRunning)
		{
			speed = 64.0f;
			setSpritesSpeed(&object->sprites, 8);
		} else {
			speed = 32.0f;
			setSpritesSpeed(&object->sprites, 4);
		}

		if (xDir != 0 && yDir != 0)
			speed *= diagonalMultiplier;

		xMod = world->delta * speed * xDir;
		yMod = world->delta * speed * yDir;

		moveVector(world, &object->collider,
		           &object->x, &object->y,
			   xMod, yMod);
	}
	
	setView(world, (int)object->x, (int)object->y);

	queueSprite(world, &object->sprites, object->x, object->y);
}
