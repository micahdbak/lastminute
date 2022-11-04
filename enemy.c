#include <math.h>
#include <stdlib.h>

#include "engine.h"

/* of each enemy, not total. */
#define MAX_ENEMY	10

#define BEAR_OUTERRADIUS	64.0f
#define BEAR_ATTACKRADIUS	16.0f
#define BEAR_STALKSPEED		32.0F

#define SKUNK_EPILEPSYSPEED	100

Uint32 enemyWander(Uint32 interval, void *param)
{
	struct enemy *enemy = (struct enemy *)param;

	if (enemy->state == ENEMY_WANDERSTATE)
	{
		enemy->speed = 16.0f * enemy->anxiety;

		/* Move in a random direction */
		enemy->direction = (float)(rand() % 360) / 180.0f * (float)M_PI;
	}

	return interval;
}

void enemyRoutine(struct world *world, struct enemy *enemy, struct object *player)
{
	float distance,
	      xMod, yMod;
	struct sprites *sprites;

	distance = getDistance(enemy->x, enemy->y, player->x, player->y);

	if (distance < enemy->outerRadius)
	{
		setSpritesSpeed(&enemy->idleSprites, 8);

		if (distance < enemy->attackRadius)
		{
			enemy->state = ENEMY_ATTACKSTATE;
		} else {
			enemy->state = ENEMY_STALKSTATE;
			enemy->speed = enemy->stalkSpeed;

			enemy->direction = getDirection(enemy->x, enemy->y, player->x, player->y);
		}
	} else {
		setSpritesSpeed(&enemy->idleSprites, 4);
		enemy->state = ENEMY_WANDERSTATE;
	}

	xMod = world->delta * enemy->speed * cos(enemy->direction);
	yMod = world->delta * enemy->speed * sin(enemy->direction);

	moveVector(world, &enemy->collider, &enemy->x, &enemy->y, xMod, yMod);
	sprites = enemy->handleSprites(enemy, player);

	queueSprite(world, sprites, enemy->x, enemy->y);
}

void enemyEnd(struct enemy *enemy)
{
	freeSprites(&enemy->idleSprites);
	freeSprites(&enemy->attackSprites);

	SDL_RemoveTimer(enemy->wanderTimer);
	SDL_RemoveTimer(enemy->attackTimer);
}

struct bearParam
{
	struct enemy *bear;
	struct object *player;
};

Uint32 bearAttack(Uint32 interval, void *param)
{
	struct bearParam *bearParam = (struct bearParam *)param;
	float direction,
	      xMod, yMod;

	if (bearParam->bear->state == ENEMY_ATTACKSTATE)
	{
		direction = getDirection(bearParam->bear->x, bearParam->bear->y, bearParam->player->x, bearParam->player->y);
		makeVector(bearParam->bear->power, direction, &xMod, &yMod);

		xKick += xMod;
		yKick += yMod;
	}

	return interval;
}

struct sprites *bearSprites(struct enemy *bear, struct object *player)
{
	struct sprites *sprites;
	
	sprites = bear->state == ENEMY_ATTACKSTATE ? &bear->attackSprites : &bear->idleSprites;
	setSprite(sprites, bear->y > player->y ? 1 : 0);

	return sprites;
}

void bearStart(struct enemy *bear, struct object *player, int xStart, int yStart)
{
	static struct bearParam bearParam[MAX_ENEMY];
	static int availableIndex = 0;

	loadSprites("bearIdle.bmp", &bear->idleSprites, 32, 32, 16, 32);

	bear->collider.x = -8;
	bear->collider.y = -8;
	bear->collider.w = 16;
	bear->collider.h = 8;

	setSpritesSpeed(&bear->idleSprites, 4);

	loadSprites("bearAttack.bmp", &bear->attackSprites, 32, 64, 16, 48);

	setSpritesSpeed(&bear->attackSprites, 8);

	bear->x = (float)xStart;
	bear->y = (float)yStart;

	bear->randomize = ((float)(rand() % 180) / 180.0f * M_PI) - (M_PI_2);
	bear->speed = 16.0f;
	bear->direction = 0.0f;
	bear->anxiety = 1.0f;
	bear->power = 128.0f;
	bear->outerRadius = 64.0f;
	bear->attackRadius = 16.0f;
	bear->stalkSpeed = 32.0f;

	bear->state = ENEMY_WANDERSTATE;
	
	bearParam[availableIndex].bear = bear;
	bearParam[availableIndex].player = player;

	bear->wanderTimer = SDL_AddTimer(4000 / bear->anxiety, enemyWander, (void *)bear);
	bear->attackTimer = SDL_AddTimer(1000, bearAttack, (void *)&bearParam[availableIndex]);

	++availableIndex;

	bear->handleSprites = bearSprites;
}

struct skunkParam
{
	struct world *world;
	struct enemy *skunk;
	struct object *player;
};

Uint32 skunkAttack(Uint32 interval, void *param)
{
	static int isFading = 0,
	           fade = 0;
	static Uint32 colors[8] = { 0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xffff00ff, 0xff00ffff, 0x00ffffff, 0xffffffff, 0x000000ff };

	struct skunkParam *skunkParam = (struct skunkParam *)param;
	Uint32 color;
	float direction,
	      xMod, yMod;

	if (isFading)
	{
		color = colors[rand() % 8] - fade;
		SDL_FillRect(skunkParam->world->userInterface, NULL, color);

		fade += 0x20;
		
		if (fade >= 0xff)
		{
			SDL_FillRect(skunkParam->world->userInterface, NULL, 0);
			isFading = 0;
		}
		
		return SKUNK_EPILEPSYSPEED;
	} else
	if (skunkParam->skunk->state == ENEMY_ATTACKSTATE)
	{
		isFading = 1;
		fade = 0;

		direction = getDirection(skunkParam->skunk->x, skunkParam->skunk->y, skunkParam->player->x, skunkParam->player->y);
		makeVector(skunkParam->skunk->power, direction, &xMod, &yMod);

		xKick += xMod;
		yKick += yMod;

		return SKUNK_EPILEPSYSPEED;
	}

	return 1000;
}

struct sprites *skunkSprites(struct enemy *skunk, struct object *player)
{
	return &skunk->idleSprites;
}

void skunkStart(struct world *world, struct enemy *skunk, struct object *player, int xStart, int yStart)
{
	static struct skunkParam skunkParam[MAX_ENEMY];
	static int availableIndex = 0;

	loadSprites("skunk.bmp", &skunk->idleSprites, 16, 24, 8, 24);
	loadSprites("skunk.bmp", &skunk->attackSprites, 16, 24, 8, 24);

	skunk->collider.x = -8;
	skunk->collider.y = -8;
	skunk->collider.w = 16;
	skunk->collider.h = 8;

	setSpritesSpeed(&skunk->idleSprites, 4);

	skunk->x = (float)xStart;
	skunk->y = (float)yStart;

	skunk->randomize = ((float)(rand() % 180) / 180.0f * M_PI) - (M_PI_2);
	skunk->speed = 32.0f;
	skunk->direction = 0.0f;
	skunk->anxiety = 3.0f;
	skunk->power = 64.0f;
	skunk->outerRadius = 64.0f;
	skunk->attackRadius = 8.0f;
	skunk->stalkSpeed = 48.0f;

	skunk->state = ENEMY_WANDERSTATE;

	skunkParam[availableIndex].world = world;
	skunkParam[availableIndex].skunk = skunk;
	skunkParam[availableIndex].player = player;

	skunk->wanderTimer = SDL_AddTimer(4000 / skunk->anxiety, enemyWander, (void *)skunk);
	skunk->attackTimer = SDL_AddTimer(1000, skunkAttack, (void *)&skunkParam[availableIndex]);

	++availableIndex;

	skunk->handleSprites = skunkSprites;
}

struct porcParam
{
	struct enemy *porc;
	struct object *player;
};

Uint32 porcAttack(Uint32 interval, void *param)
{
	struct porcParam *porcParam = (struct porcParam *)param;
	float direction,
	      xMod, yMod;

	if (porcParam->porc->state == ENEMY_ATTACKSTATE)
	{
		direction = getDirection(porcParam->porc->x, porcParam->porc->y, porcParam->player->x, porcParam->player->y);
		makeVector(porcParam->porc->power, direction, &xMod, &yMod);

		xKick += xMod;
		yKick += yMod;

		porcParam->porc->attackSprites.sprite.x = 0;
	}

	return interval;
}

struct sprites *porcSprites(struct enemy *porc, struct object *player)
{
	float direction;
	int sprite;

	if (porc->state != ENEMY_ATTACKSTATE)
	{
		direction = porc->direction;//getDirection(porc->x, porc->y, player->x, player->y);

		if (direction < (M_PI / 4.0)
		 || direction >= (7.0 / 4.0 * M_PI))
			sprite = 3;
		else
		if (direction >= (M_PI / 4.0)
		 && direction < (3.0 / 4.0 * M_PI))
			sprite = 0;
		else
		if (direction >= (3.0 / 4.0 * M_PI)
		 && direction < (5.0 / 4.0 * M_PI))
			sprite = 2;
		else
			sprite = 1;

		setSprite(&porc->idleSprites, sprite);

		return &porc->idleSprites;
	} else
		return &porc->attackSprites;
}

void porcStart(struct enemy *porc, struct object *player, int xStart, int yStart)
{
	static struct porcParam porcParam[MAX_ENEMY];
	static int availableIndex = 0;

	loadSprites("porcIdle.bmp", &porc->idleSprites, 16, 16, 8, 12);
	loadSprites("porcAttack.bmp", &porc->attackSprites, 48, 48, 24, 28);

	porc->collider.x = -8;
	porc->collider.y = -8;
	porc->collider.w = 16;
	porc->collider.h = 16;

	setSpritesSpeed(&porc->idleSprites, 4);
	setSpritesSpeed(&porc->attackSprites, 4);

	porc->x = (float)xStart;
	porc->y = (float)yStart;

	porc->randomize = ((float)(rand() % 180) / 180.0f * M_PI) - (M_PI_2);
	porc->speed = 16.0f;
	porc->direction = 0.0f;
	porc->anxiety = 0.5f;
	porc->power = 96.0f;
	porc->outerRadius = 64.0f;
	porc->attackRadius = 32.0f;
	porc->stalkSpeed = 24.0f;

	porc->state = ENEMY_WANDERSTATE;

	porcParam[availableIndex].porc = porc;
	porcParam[availableIndex].player = player;

	porc->wanderTimer = SDL_AddTimer(4000 / porc->anxiety, enemyWander, (void *)porc);
	porc->attackTimer = SDL_AddTimer(1000, porcAttack, (void *)&porcParam[availableIndex]);

	++availableIndex;

	porc->handleSprites = porcSprites;
}

/*
struct raccoonParam
{
	struct enemy *raccoon;
	struct object *player;
};

Uint32 raccoonAttack(Uint32 interval, void *param)
{
	struct raccoonParam *raccoonParam = (struct raccoonParam *)param;
	
	return interval;
}

struct sprites *raccoonSprites(struct enemy *raccoon, struct object *player)
{
	return &raccoon->idleSprites;
}

void raccoonStart(struct enemy *raccoon, struct object *player, int xStart, int yStart)
{
	static struct raccoonParam raccoonParam[MAX_ENEMY];
	static int availableIndex = 0;

	printf("Creating raccoon...\n");

	loadSprites("raccoon.bmp", &raccoon->idleSprites, 16, 16, 8, 12);
	loadSprites("raccoon.bmp", &raccoon->attackSprites, 16, 16, 8, 12);

	raccoon->collider.x = -4;
	raccoon->collider.y = -4;
	raccoon->collider.w = 8;
	raccoon->collider.h = 8;

	setSpritesSpeed(&raccoon->idleSprites, 4);

	raccoon->x = (float)xStart;
	raccoon->y = (float)yStart;

	raccoon->randomize = ((float)(rand() % 180) / 180.0f * M_PI) - (M_PI_2);
	raccoon->speed = 32.0f;
	raccoon->direction = 0.0f;
	raccoon->anxiety = 8.0f;
	raccoon->power = 128.0f;
	raccoon->outerRadius = 128.0f;
	raccoon->attackRadius = 32.0f;
	raccoon->stalkSpeed = 64.0f;

	raccoon->state = ENEMY_WANDERSTATE;

	raccoonParam[availableIndex].raccoon = raccoon;
	raccoonParam[availableIndex].player = player;

	raccoon->wanderTimer = SDL_AddTimer(4000 / raccoon->anxiety, enemyWander, (void *)raccoon);
	raccoon->attackTimer = SDL_AddTimer(1000, raccoonAttack, (void *)&raccoonParam[availableIndex]);

	++availableIndex;

	raccoon->handleSprites = raccoonSprites;

	printf("Created raccoon.\n");
}
*/
