#ifndef ENGINE_H
#define ENGINE_H

#define MAPSHOT

#include "SDL.h"
#include "SDL_mixer.h"

struct font;
struct object;
struct world;


/* draw.c */

extern SDL_Surface *colorArea_surface;

#define FONT_NCOLS	12
#define FONT_NROWS	6

struct font
{
	SDL_Surface *sheet;
	SDL_Rect letter[FONT_NCOLS * FONT_NROWS];
	int leading, spaceWidth;
};

struct modifyLetter
{
	char letter;
	int width;
};

#define ML(ML_char, ML_width)\
	(struct modifyLetter){ML_char, ML_width}

void loadFont(const char *path, struct font *font, int leading, int spaceWidth, int defaultWidth, int nModifed, ...);
void freeFont(struct font *font);
void drawText(SDL_Surface *surface, SDL_Rect *drawArea, struct font *font, char *text, ...);
void colorArea(SDL_Surface *surface, SDL_Rect *area, Uint32 color);


/* object.c */

struct sprites
{
	SDL_Surface *sheet;
	int nCol, nRow,
	    xCenter, yCenter,
	    interval,
	    isPaused;
	SDL_Rect sprite;
	SDL_TimerID timer;
};

void loadSprites(const char *path, struct sprites *sprites, int width, int height, int xCenter, int yCenter);
void setSprite(struct sprites *sprites, int row);
void setSpritesSpeed(struct sprites *sprites, int framesPerSecond);
void freeSprites(struct sprites *sprites);

struct object
{
	struct sprites sprites;
	SDL_Rect collider;
	float x, y;
	void (*objectRoutine)(struct object *object, struct world *world);
};

float getDistance(float x1, float y1, float x2, float y2);
float getDirection(float x1, float y1, float x2, float y2);
void makeVector(float distance, float direction, float *x, float *y);


/* world.c */

#define KEY_UP		0b0000000001
#define KEY_RIGHT	0b0000000010
#define KEY_DOWN	0b0000000100
#define KEY_LEFT	0b0000001000
#define KEY_SPECIAL	0b0000010000
#define KEY_MENU	0b0000100000
#define KEY_SELECT	0b0001000000
#define KEY_BACK	0b0010000000
#define MOUSE_LEFT	0b0100000000
#define MOUSE_RIGHT	0b1000000000

#define PLAYER_SPEED	8
#define MAX_SPRITEQUEUE	50
#define MAX_OBJECTQUEUE	50

#define FONT_WHITE	0
#define FONT_BLACK	1
#define NFONTS		2

#define MAP_BACKGROUND	0
#define MAP_ADDITIONAL	1
#define MAP_FOREGROUND	2

struct world
{
	struct font font[NFONTS];

	struct tileSet
	{
		SDL_Surface *imageData;
		int width, height,
		    nCol, nRow;
#	ifdef EDITOR
		const char path[50];
#	endif
	} tileSet;

	struct map
	{
		SDL_Rect **mapData[3];
		int **collision;
		int nCol, nRow;
#	ifdef EDITOR
		const char *path;
#	endif
	} map;

	SDL_Surface *backGround, *spriteGround, *userInterface;
	int screenWidth, screenHeight;
	SDL_Rect view;

	int worldID;

	struct spriteIndex
	{
		SDL_Surface *sheet;
		int raise;
		SDL_Rect sprite, render;
		struct spriteIndex *next;
	} *spriteQueue;

	void (*worldRoutine)(struct world *world);
	void (*worldEnd)(struct world *world);

	float delta;
	Uint32 keyMap[2];
	int xMouse, yMouse;
};

#define keyIsDown(keyIsDown_worldPtr, keyIsDown_key)\
	( ((keyIsDown_worldPtr)->keyMap[0] & keyIsDown_key) && ((keyIsDown_worldPtr)->keyMap[1] & keyIsDown_key) )
#define keyIsHit(keyIsDown_worldPtr, keyIsDown_key)\
	( !((keyIsDown_worldPtr)->keyMap[0] & keyIsDown_key) && ((keyIsDown_worldPtr)->keyMap[1] & keyIsDown_key) )

void loadWorld(const char *path, struct world *world);
void freeWorld(struct world *world);
void queueSprite(struct world *world, struct sprites *sprites, float x, float y);
void setView(struct world *world, int xFocus, int yFocus);
int isCollision(struct world *world, SDL_Rect *collider);
void moveVector(struct world *world, SDL_Rect *collider, float *x, float *y, float xMod, float yMod);
void loopWorld(struct world *world, SDL_Surface *screen, SDL_Rect *renderArea);
#ifdef EDITOR
void worldEditor(struct world *world, SDL_Surface *screen, SDL_Rect *renderArea);
#endif


/* player.c */

void playerStart(struct object *object, int xStart, int yStart);
void playerRoutine(struct object *object, struct world *world);


/* enemy.c */

#define ENEMY_WANDERSTATE	0
#define ENEMY_STALKSTATE	1
#define ENEMY_ATTACKSTATE	2

struct enemy
{
	struct sprites idleSprites, attackSprites;
	SDL_Rect collider;
	float x, y,
	      randomize,
	      speed, direction,
	      anxiety,
	      power,
	      outerRadius, attackRadius,
	      stalkSpeed;
	int state;
	SDL_TimerID wanderTimer, attackTimer;
	struct sprites *(*handleSprites)(struct enemy *enemy, struct object *player);
};

void enemyRoutine(struct world *world, struct enemy *enemy, struct object *player);
void enemyEnd(struct enemy *enemy);

void bearStart(struct enemy *bear, struct object *player, int xStart, int yStart);
void skunkStart(struct world *world, struct enemy *skunk, struct object *player, int xStart, int yStart);
void porcStart(struct enemy *porc, struct object *player, int xStart, int yStart);
void raccoonStart(struct enemy *raccoon, struct object *player, int xStart, int yStart);


/* overworld.c */

extern float xKick, yKick;

void overWorldStart(struct world *world);
void overWorldRoutine(struct world *world);
void overWorldEnd(struct world *world);


#endif
