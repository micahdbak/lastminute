#include <math.h>

#include "engine.h"

struct enemy;

static struct object player;

struct enemy
{
	struct object sprite, idle;
	int isAttackable,
	    isAttacking;
	float direction,
	      speed,
	      outerRadius, innerRadius, attackRadius,
	      anxiety;
	SDL_TimerID attackTimer;
};

static struct enemy bear, bird;

static struct kickBack
{
	float intensity;
	float direction;
} kickBack;

float getDistance(struct object *enemy, struct object *focus)
{
	float leg1, leg2;

	leg1 = fabs(focus->x - enemy->x);
	leg2 = fabs(focus->y - enemy->y);

	if (leg1 == 0)
		return leg2;
	if (leg2 == 0)
		return leg1;

	return sqrt(pow(leg1, 2) + pow(leg2, 2));
}

float getDirection(struct object *enemy, struct object *focus)
{
	float numerator, denominator, reference;
	int quadrant;

	numerator = focus->y - enemy->y;
	denominator = focus->x - enemy->x;

	if (numerator == 0.0)
		return denominator >= 0 ? 0 : M_PI;
	if (denominator == 0.0)
		return numerator >= 0 ? M_PI_2 : 3.0 * M_PI_2;

	reference = atan( (focus->y - enemy->y) / (focus->x - enemy->x) );

	return denominator > 0 ? reference : M_PI + reference;
}

void moveInDirection(struct object *sprite, struct world *world, float direction, float speed)
{
	float xMod, yMod;
	SDL_Rect propose;

	xMod = world->delta * speed * cos(direction);
	yMod = world->delta * speed * sin(direction);

	propose.w = sprite->collider.w;
	propose.h = sprite->collider.h;
	propose.x = sprite->collider.x + (int)(sprite->x + xMod) - sprite->xCenter;
	propose.y = sprite->collider.y + (int)sprite->y - sprite->yCenter;

	if (!isCollision(world, &propose))
		sprite->x += xMod;
	else
		propose.x = sprite->collider.x + (int)sprite->x - sprite->xCenter;

	propose.y = sprite->collider.y + (int)(sprite->y + yMod) - sprite->yCenter;

	if (!isCollision(world, &propose))
		sprite->y += yMod;
}

int enemyController(struct enemy *enemy, struct world *world, struct object *player)
{
	float distance,
	      speed = enemy->anxiety * enemy->speed,
	      xMod, yMod,
	      ratio,
	      direction;
	SDL_Rect propose;

	distance = getDistance(&enemy->sprite, player);
	enemy->isAttackable = 0;

	if (distance < enemy->outerRadius)
	{
		direction = getDirection(&enemy->sprite, player);
		enemy->direction = direction;
		speed = enemy->speed;

		if (distance < enemy->attackRadius)
			enemy->isAttackable = 1;

		if (distance < enemy->innerRadius)
			enemy->direction += 0.6 * M_PI;
	}

	if (!enemy->isAttacking)
	{
		xMod = world->delta * speed * cos(enemy->direction);
		yMod = world->delta * speed * sin(enemy->direction);

		propose.w = enemy->sprite.collider.w;
		propose.h = enemy->sprite.collider.h;

		propose.x = enemy->sprite.collider.x + (int)(enemy->sprite.x + xMod) - enemy->sprite.xCenter;
		propose.y = enemy->sprite.collider.y + (int)enemy->sprite.y - enemy->sprite.yCenter;

		if (!isCollision(world, &propose))
			enemy->sprite.x += xMod;
		else
			propose.x = enemy->sprite.collider.x + (int)enemy->sprite.x - enemy->sprite.xCenter;

		propose.y = enemy->sprite.collider.y + (int)(enemy->sprite.y + yMod) - enemy->sprite.yCenter;

		if (!isCollision(world, &propose))
			enemy->sprite.y += yMod;
	}

	if (direction < 0)
		direction = 2.0 * M_PI + direction;
	else
	if (direction > 2.0 * M_PI)
		direction -= 2.0 * M_PI;

	if ((direction > 0 && direction <= M_PI / 4.0)
	 || (direction > 7.0 * M_PI / 4.0 && direction <= 0))
		return 1;
	if (direction > M_PI / 4.0 && direction <= 3.0 * M_PI / 4.0)
		return 2;
	if (direction > 3.0 * M_PI / 4.0 && direction <= 5.0 * M_PI / 4.0)
		return 3;
	if (direction > 5.0 * M_PI / 4.0 && direction <= 7.0 * M_PI / 4.0)
		return 0;

	return 0;
}

void enemyStart(struct enemy *enemy)
{
	enemy->isAttackable = 0;
	enemy->isAttacking = 0;
	enemy->direction = 4;
	enemy->speed = 16.0f;
	enemy->outerRadius = 64.0f;
	enemy->innerRadius = 24.0f;
	enemy->attackRadius = 24.0f;
	enemy->anxiety = 0.0f;
}

Uint32 bearAttack(Uint32 interval, void *param)
{
	struct world *world = (struct world *)param;

	if (bear.isAttackable)
	{
		kickBack.direction = getDirection(&bear.sprite, &player);
		kickBack.intensity = 64.0f;

		bear.isAttacking = 1;
	} else
		bear.isAttacking = 0;

	return interval;
}

void bearStart(struct world *world, int xStart, int yStart)
{
	loadSprites("bear.bmp", &bear.sprite, 32, 64);
	setSpriteSpeed(&bear.sprite, 4);

	bear.sprite.x = (float)xStart;
	bear.sprite.y = (float)yStart;
	bear.sprite.xCenter = 16;
	bear.sprite.yCenter = 48;

	bear.sprite.collider.x = 8;
	bear.sprite.collider.y = 32;
	bear.sprite.collider.w = 16;
	bear.sprite.collider.h = 8;

	enemyStart(&bear);

	bear.attackTimer = SDL_AddTimer(1000, bearAttack, (void *)world);
}

void bearRoutine(struct world *world)
{
	enemyController(&bear, world, &player);

	if (bear.sprite.y > player.y)
		setSprite(&bear.sprite, bear.isAttacking ? 3 : 1);
	else
		setSprite(&bear.sprite, bear.isAttacking ? 2 : 0);

	queueSprite(world, &bear.sprite);
}

void birdStart(struct world *world, int xStart, int yStart)
{
	loadSprites("bird_attack.bmp", &bird.sprite, 48, 48);
	loadSprites("bird_idle.bmp", &bird.idle, 16, 16);

	bird.sprite.x = bird.idle.x = (float)xStart;
	bird.sprite.y = bird.idle.y = (float)yStart;

	bird.idle.xCenter = 8;
	bird.idle.yCenter = 8;

	enemyStart(&bird);

	//bird.anxiety = 1.0f;
	bird.outerRadius = 128.0f;
}

void birdRoutine(struct world *world)
{
	static int sprite = 0;

	int newSprite;

	newSprite = enemyController(&bird, world, &player);
	bird.idle.x = bird.sprite.x;
	bird.idle.y = bird.sprite.y;

	if (newSprite != sprite)
	{
		setSprite(&bird.sprite, newSprite);
		setSprite(&bird.idle, newSprite);
		sprite = newSprite;
	}

	queueSprite(world, &bird.idle);
}

void overWorldStart(struct world *world)
{
	playerStart(&player, "32,32");
	birdStart(world, 64, 64);
	
	kickBack.intensity = 0;
	kickBack.direction = 0;
}

void overWorldRoutine(struct world *world)
{
	moveInDirection(&player, world, kickBack.direction, kickBack.intensity);
	kickBack.intensity *= 1.0f - (world->delta * 0.75);

	playerRoutine(&player, world);
	birdRoutine(world);
}

void overWorldEnd(struct world *world)
{
	freeObject(&player);
	freeObject(&bear.sprite);
	freeObject(&bird.sprite);
	SDL_RemoveTimer(bear.attackTimer);
}
