#include <math.h>

#include "engine.h"

struct object player;
struct enemy bear;

float xKick = 0.0f, yKick = 0.0f;

void overWorldStart(struct world *world)
{
	playerStart(&player, 6, 124);
}

void overWorldRoutine(struct world *world)
{
	float xMod, yMod;
	Uint32 color;

	playerRoutine(&player, world);
	//enemyRoutine(world, &bear, &player);

	xMod = xKick * world->delta;
	yMod = yKick * world->delta;

	xKick *= 1.0 - (world->delta * 0.5);
	yKick *= 1.0 - (world->delta * 0.5);

	if (hypot(xKick, yKick) < 32.0)
		xKick = yKick = 0;

	moveVector(world, &player.collider, &player.x, &player.y, xMod, yMod);
}

void overWorldEnd(struct world *world)
{
	freeSprites(&player.sprites);
	//enemyEnd(&bear);
}
