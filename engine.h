#ifndef ENGINE_H
#define ENGINE_H

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

struct object
{
	SDL_Surface *sheet;
	float x, y;
	int nCol, nRow,
	    xCenter, yCenter,
	    interval,
	    isPaused,
	    raise;
	SDL_Rect sprite;
	SDL_Rect collider;
	SDL_TimerID timer;
	void (*objectRoutine)(struct object *object, struct world *world);
};

void loadSprites(const char *path, struct object *object, int width, int height);
void setSprite(struct object *object, int row);
void setSpriteSpeed(struct object *object, int framesPerSecond);
void freeObject(struct object *object);


/* world.c */

#define KEY_UP		0b00000001
#define KEY_RIGHT	0b00000010
#define KEY_DOWN	0b00000100
#define KEY_LEFT	0b00001000
#define KEY_SPECIAL	0b00010000
#define KEY_MENU	0b00100000
#define KEY_SELECT	0b01000000
#define KEY_BACK	0b10000000

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
	int xView, yView;

	int worldID;

	struct spriteIndex
	{
		SDL_Surface *sheet;
		int raise;
		SDL_Rect *sprite;
		SDL_Rect render;
		struct spriteIndex *next;
	} *spriteQueue;

	void (*worldRoutine)(struct world *world);
	void (*worldEnd)(struct world *world);

	float delta;
	Uint32 keyMap[2];
};

void loadWorld(const char *path, struct world *world);
void freeWorld(struct world *world);

void queueSprite(struct world *world, struct object *object);

#define keyIsDown(keyIsDown_worldPtr, keyIsDown_key)\
	( ((keyIsDown_worldPtr)->keyMap[0] & keyIsDown_key) && ((keyIsDown_worldPtr)->keyMap[1] & keyIsDown_key) )
#define keyIsHit(keyIsDown_worldPtr, keyIsDown_key)\
	( !((keyIsDown_worldPtr)->keyMap[0] & keyIsDown_key) && ((keyIsDown_worldPtr)->keyMap[1] & keyIsDown_key) )

void setView(struct world *world, int xFocus, int yFocus);
int isCollision(struct world *world, SDL_Rect *area);
void loopWorld(struct world *world, SDL_Surface *screen, SDL_Rect *renderArea);

#ifdef EDITOR
void worldEditor(struct world *world, SDL_Surface *screen, SDL_Rect *renderArea);
#endif


/* overworld.c */

void overWorldStart(struct world *world);
void overWorldRoutine(struct world *world);
void overWorldEnd(struct world *world);


/* player.c */

extern struct playerData playerData;

void playerStart(struct object *object, char *param);
void playerRoutine(struct object *object, struct world *world);

#endif
