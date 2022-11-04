#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "engine.h"

#define MAX_TILESETPATH	256
#define MAX_OBJECTPARAM	100

void loadWorld(const char *path, struct world *world)
{
	FILE *worldFile;
	char tileSetPath[MAX_TILESETPATH];
	int xMap, yMap,
	    xTile, yTile,
	    readData,
	    c, worldID, i,
	    mapIndex;
	SDL_Rect emptyTile;

	worldFile = fopen(path, "r");
	assert(worldFile != NULL);
#ifdef EDITOR
	world->map.path = path;
#endif

	fscanf(worldFile, "%d\n", &world->worldID);

#ifndef EDITOR
	switch (world->worldID)
	{
	case 0:
	/* overWorld.c */
		world->worldRoutine = overWorldRoutine;
		world->worldEnd = overWorldEnd;

		overWorldStart(world);

		break;
	default:

		break;
	}
#endif

	fscanf(worldFile, "%s\n", tileSetPath);
	world->tileSet.imageData = SDL_LoadBMP(tileSetPath);
	assert(world->tileSet.imageData != NULL);
	SDL_SetColorKey(world->tileSet.imageData, SDL_TRUE, SDL_MapRGB(world->tileSet.imageData->format, 0xff, 0, 0xff));
#ifdef EDITOR
	sprintf(world->tileSet.path, "%s", tileSetPath);
#endif

	fscanf(worldFile, "%d,%d\n", &world->tileSet.width, &world->tileSet.height);
	world->tileSet.nCol = world->tileSet.imageData->w / world->tileSet.width;
	world->tileSet.nRow = world->tileSet.imageData->h / world->tileSet.height;

	emptyTile.x = 0;
	emptyTile.y = 0;
	emptyTile.w = world->tileSet.width;
	emptyTile.h = world->tileSet.height;

	fscanf(worldFile, "%d,%d\n", &world->map.nCol, &world->map.nRow);
	assert(world->map.nCol > 0 && world->map.nRow > 0);

	world->map.mapData[0] = calloc(world->map.nCol, sizeof(SDL_Rect *));
	world->map.mapData[1] = calloc(world->map.nCol, sizeof(SDL_Rect *));
	world->map.mapData[2] = calloc(world->map.nCol, sizeof(SDL_Rect *));
	world->map.collision = calloc(world->map.nCol, sizeof(int *));
	assert(world->map.mapData[0] != NULL
	    && world->map.mapData[1] != NULL
	    && world->map.mapData[2] != NULL
	    && world->map.collision != NULL);

	for (xMap = 0; xMap < world->map.nCol; ++xMap)
	{
		world->map.mapData[0][xMap] = calloc(world->map.nRow, sizeof(SDL_Rect));
		world->map.mapData[1][xMap] = calloc(world->map.nRow, sizeof(SDL_Rect));
		world->map.mapData[2][xMap] = calloc(world->map.nRow, sizeof(SDL_Rect));
		world->map.collision[xMap] = calloc(world->map.nRow, sizeof(SDL_Rect));
		assert(world->map.mapData[0][xMap] != NULL
		    && world->map.mapData[1][xMap] != NULL
		    && world->map.mapData[2][xMap] != NULL
		    && world->map.collision[xMap] != NULL);

		for (yMap = 0; yMap < world->map.nRow; ++yMap)
		{
			world->map.mapData[0][xMap][yMap] = emptyTile;
			world->map.mapData[1][xMap][yMap] = emptyTile;
			world->map.mapData[2][xMap][yMap] = emptyTile;
			world->map.collision[xMap][yMap] = 0;
		}
	}

	/* Read map data */

	xMap = 0;
	yMap = 0;
	mapIndex = 0;

	readData = 1;

	while (readData)
		switch (c = getc(worldFile))
		{
		case ',':
			++xMap;

			break;
		case '\n':
			xMap = 0;
			++yMap;

			break;
		case ';':
			xMap = 0;
			yMap = 0;
			++mapIndex;

			/* The next character is assumed to be a '\n', so we skip it. */
			getc(worldFile);

			break;
		case EOF:
			readData = 0;

			break;
		default:
			ungetc(c, worldFile);

			if (mapIndex < 3)
			{
				fscanf(worldFile, "%d.%d", &xTile, &yTile);

				world->map.mapData[mapIndex][xMap][yMap].x = xTile * world->tileSet.width;
				world->map.mapData[mapIndex][xMap][yMap].y = yTile * world->tileSet.height;
				world->map.mapData[mapIndex][xMap][yMap].w = world->tileSet.width;
				world->map.mapData[mapIndex][xMap][yMap].h = world->tileSet.height;
			} else
			if (mapIndex == 3)
			{
				fscanf(worldFile, "%d", &world->map.collision[xMap][yMap]);
			} else
				readData = 0;

			break;
		}

	world->backGround = SDL_CreateRGBSurface(0, world->screenWidth, world->screenHeight, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	world->spriteGround = SDL_CreateRGBSurface(0, world->screenWidth, world->screenHeight, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	world->userInterface = SDL_CreateRGBSurface(0, world->screenWidth, world->screenHeight, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	assert(world->backGround != NULL && world->spriteGround != NULL);

	SDL_FillRect(world->backGround, NULL, 0x000000ff);
	SDL_FillRect(world->spriteGround, NULL, 0x00000000);

	world->view.x = 0;
	world->view.y = 0;
	world->view.w = world->screenWidth;
	world->view.h = world->screenHeight;

	world->spriteQueue = NULL;

	world->delta = 1;
	world->keyMap[0] = 0;
	world->keyMap[1] = 0;

	fclose(worldFile);

	printf("Successfully loaded world.\n");
}

void freeWorld(struct world *world)
{
	int xMap, yMap;

	world->worldEnd(world);

	for (xMap = 0; xMap < world->map.nCol; ++xMap)
	{
		free(world->map.mapData[0][xMap]);
		free(world->map.mapData[1][xMap]);
		free(world->map.mapData[2][xMap]);
		free(world->map.collision[xMap]);
	}

	free(world->map.mapData[0]);
	free(world->map.mapData[1]);
	free(world->map.mapData[2]);
	free(world->map.collision);

	SDL_FreeSurface(world->tileSet.imageData);
	SDL_FreeSurface(world->backGround);
	SDL_FreeSurface(world->spriteGround);
	SDL_FreeSurface(world->userInterface);
}

void queueSprite(struct world *world, struct sprites *sprites, float x, float y)
{
	static struct spriteIndex queue[MAX_SPRITEQUEUE];
	static int availableIndex;

	int raise;
	SDL_Rect render;
	struct spriteIndex *marchSprite;

	raise = (int)y;

	render.x = (int)x - sprites->xCenter;
	render.y = (int)y - sprites->yCenter;
	render.w = sprites->sprite.w;
	render.h = sprites->sprite.h;

	if (SDL_IntersectRect(&render, &world->view, &render) == SDL_FALSE)
		return;

	render.x -= world->view.x;
	render.y -= world->view.y;

	if (world->spriteQueue == NULL)
	{
		availableIndex = 0;

		queue[availableIndex].sheet = sprites->sheet;
		queue[availableIndex].sprite = &sprites->sprite;
		queue[availableIndex].render = render;
		queue[availableIndex].raise = raise;
		queue[availableIndex].next = NULL;
		
		world->spriteQueue = &queue[availableIndex];
		++availableIndex;

		return;
	}

	if (availableIndex == MAX_SPRITEQUEUE - 1)
		return;

	queue[availableIndex].sheet = sprites->sheet;
	queue[availableIndex].sprite = &sprites->sprite;
	queue[availableIndex].render = render;
	queue[availableIndex].raise = raise;
	queue[availableIndex].next = NULL;

	if (world->spriteQueue->raise > raise)
	{
		queue[availableIndex].next = world->spriteQueue;
		world->spriteQueue = &queue[availableIndex];
		++availableIndex;

		return;
	}

	for (marchSprite = world->spriteQueue;; marchSprite = marchSprite->next)
	{
		if (marchSprite->next == NULL)
			break;

		if (marchSprite->next->raise >= raise)
			break;
	}

	queue[availableIndex].next = marchSprite->next;
	marchSprite->next = &queue[availableIndex];
	++availableIndex;
}

void setView(struct world *world, int xFocus, int yFocus)
{
	int xMax, yMax;

	xMax = (world->map.nCol * world->tileSet.width) - world->view.w;
	yMax = (world->map.nRow * world->tileSet.height) - world->view.h;

	world->view.x = xFocus - (world->view.w / 2);
	world->view.y = yFocus - (world->view.h / 2);

	if (world->view.x < 0)
		world->view.x = 0;
	if (world->view.y < 0)
		world->view.y = 0;
	if (world->view.x > xMax)
		world->view.x = xMax;
	if (world->view.y > yMax)
		world->view.y = yMax;
}

#define MAX_COLLISIONBOX	20

int isCollision(struct world *world, SDL_Rect *area)
{
	int xMin, yMin,
	    xMax, yMax,
	    xMap, yMap;
	SDL_Rect collisionBox;
	SDL_Rect result;

	xMin = area->x / world->tileSet.width;
	yMin = area->y / world->tileSet.height;
	xMax = (area->x + area->w) / world->tileSet.width;
	++xMax;
	yMax = (area->y + area->h) / world->tileSet.height;
	++yMax;

	if (xMin < 0)
		xMin = 0;
	if (yMin < 0)
		yMin = 0;
	if (xMax > world->map.nCol)
		xMax = world->map.nCol;
	if (yMax > world->map.nRow)
		yMax = world->map.nRow;

	collisionBox.w = world->tileSet.width;
	collisionBox.h = world->tileSet.height;

	for (xMap = xMin; xMap < xMax; ++xMap)
		for (yMap = yMin; yMap < yMax; ++yMap)
		{
			collisionBox.x = xMap * world->tileSet.width;
			collisionBox.y = yMap * world->tileSet.height;

			if (world->map.collision[xMap][yMap] && SDL_IntersectRect(&collisionBox, area, &result))
				return 1;
		}

	return 0;
}

void moveVector(struct world *world, SDL_Rect *collider, float *x, float *y, float xMod, float yMod)
{
	SDL_Rect propose;

	propose.x = collider->x + (int)(*x + xMod);
	propose.y = collider->y + (int)*y;
	propose.w = collider->w;
	propose.h = collider->h;

	if (!isCollision(world, &propose))
		*x += xMod;
	else
		propose.x = collider->x + (int)*x;

	propose.y = collider->y + (int)(*y + yMod);

	if (!isCollision(world, &propose))
		*y += yMod;
}

void loopWorld(struct world *world, SDL_Surface *screen, SDL_Rect *renderArea)
{
	struct spriteIndex *marchSprite;
	int mapIndex, xMap, yMap;
	SDL_Rect tile, render, source;

	SDL_FillRect(world->userInterface, NULL, 0);
	world->spriteQueue = NULL;
	world->worldRoutine(world);

	SDL_FillRect(world->spriteGround, NULL, 0);

	for (marchSprite = world->spriteQueue; marchSprite != NULL; marchSprite = marchSprite->next)
		SDL_BlitSurface(marchSprite->sheet, marchSprite->sprite, world->spriteGround, &marchSprite->render);

	SDL_FillRect(world->backGround, NULL, 0x808080ff);

	for (mapIndex = 0; mapIndex < 2; ++mapIndex)
	{
		if (mapIndex == 2)
			SDL_BlitSurface(world->spriteGround, NULL, world->backGround, NULL);

		for (yMap = 0; yMap < world->map.nRow; ++yMap)
			for (xMap = 0; xMap < world->map.nCol; ++xMap)
			{
				tile.x = xMap * world->tileSet.width;
				tile.y = yMap * world->tileSet.height;
				tile.w = world->tileSet.width;
				tile.h = world->tileSet.height;

				if (SDL_IntersectRect(&world->view, &tile, &render))
				{
					render.x -= world->view.x;
					render.y -= world->view.y;

					source = world->map.mapData[mapIndex][xMap][yMap];

					if (render.w < world->tileSet.width && render.x == 0)
						source.x += world->tileSet.width - render.w;
					if (render.h < world->tileSet.height && render.y == 0)
						source.y += world->tileSet.height - render.h;

					source.w = render.w;
					source.h = render.h;

					SDL_BlitSurface(world->tileSet.imageData, &source, world->backGround, &render);
				}
		}
	}

	SDL_BlitSurface(world->spriteGround, NULL, world->backGround, NULL);
	SDL_BlitScaled(world->backGround, NULL, screen, renderArea);
	SDL_BlitScaled(world->userInterface, NULL, screen, renderArea);
}

#ifdef EDITOR

void worldEditor(struct world *world, SDL_Surface *screen, SDL_Rect *renderArea)
{
	static int xShift = 8, yShift = 6;
	static int xView = -8, yView = -6,
		   xTile = 0, yTile = 0;
	static int mode = 0;
	static SDL_Rect selectedTile;
	static SDL_Rect UILocation = { 4, 166, 192, 12 };
	static SDL_Rect check = { 48, 48, 96, 48 };
	static int mapIndex = 0;

	int xMap, yMap,
	    xMin, yMin,
	    xMax, yMax,
	    renderMapIndex,
	    mouseInArea = 0;
	SDL_Rect tileRenderArea;
	SDL_Rect selectedMapTile;
	
	if (!mode)
	{
		xView += keyIsHit(world, KEY_RIGHT) ? 1 : keyIsHit(world, KEY_LEFT) ? -1 : 0;
		yView += keyIsHit(world, KEY_DOWN) ? 1 : keyIsHit(world, KEY_UP) ? -1 : 0;

		if (xView < 0 - xShift)
			xView = 0 - xShift;
		if (xView > world->map.nCol - 1 - xShift)
			xView = world->map.nCol - 1 - xShift;

		if (yView < 0 - yShift)
			yView = 0 - yShift;
		if (yView > world->map.nRow - 1 - yShift)
			yView = world->map.nRow - 1 - yShift;
	}

	xMin = xView < 0 ? 0 : xView;
	yMin = yView < 0 ? 0 : yView;
	xMax = xView + (world->screenWidth / world->tileSet.width);
	yMax = yView + (world->screenHeight / world->tileSet.height);

	if (world->map.nCol < xMax)
		xMax = world->map.nCol;
	if (world->map.nRow < yMax)
		yMax = world->map.nRow;

	tileRenderArea.w = world->tileSet.width;
	tileRenderArea.h = world->tileSet.height;

	SDL_FillRect(world->backGround, NULL, SDL_MapRGB(world->backGround->format, 0x10, 0x10, 0x10));

	for (renderMapIndex = 0; renderMapIndex < 3; ++renderMapIndex)
		for (xMap = xMin; xMap < xMax; ++xMap)
			for (yMap = yMin; yMap < yMax; ++yMap)
			{
				tileRenderArea.x = (xMap - xView) * world->tileSet.width;
				tileRenderArea.y = (yMap - yView) * world->tileSet.height;

				if (renderMapIndex == 0)
					SDL_FillRect(world->backGround, &tileRenderArea, SDL_MapRGB(world->backGround->format, 0xff, 0x0, 0xff));

				SDL_BlitSurface(world->tileSet.imageData, &world->map.mapData[renderMapIndex][xMap][yMap], world->backGround, &tileRenderArea);
			}

	if (mapIndex == 3)
	{
		for (xMap = xMin; xMap < xMax; ++xMap)
			for (yMap = yMin; yMap < yMax; ++yMap)
			{
				tileRenderArea.x = (xMap - xView) * world->tileSet.width;
				tileRenderArea.y = (yMap - yView) * world->tileSet.height;

				if (world->map.collision[xMap][yMap])
					colorArea(world->backGround, &tileRenderArea, 0xffffff80);
			}
	}

	selectedMapTile.w = world->tileSet.width;
	selectedMapTile.h = world->tileSet.height;
	selectedMapTile.x = selectedMapTile.w * xShift;
	selectedMapTile.y = selectedMapTile.h * yShift;

	colorArea(world->backGround, &selectedMapTile, 0xffffff80);

	if (mode)
		colorArea(world->backGround, NULL, 0x00000040);

	drawText(world->backGround, &UILocation, &world->font[FONT_WHITE], "( %d, %d ) %nmapIndex: %d", xView + xShift, yView + yShift, mapIndex);

	SDL_BlitScaled(world->backGround, NULL, screen, renderArea);
	SDL_FillRect(world->spriteGround, NULL, 0);

	switch (mode)
	{
	case 1:
		if (keyIsHit(world, KEY_BACK))
		{
			mode = 0;

			break;
		}

		xTile += keyIsHit(world, KEY_RIGHT) ? 1 : keyIsHit(world, KEY_LEFT) ? -1 : 0;
		yTile += keyIsHit(world, KEY_DOWN) ? 1 : keyIsHit(world, KEY_UP) ? -1 : 0;

		if (world->xMouse <= world->tileSet.imageData->w && world->yMouse <= world->tileSet.imageData->h)
		{
			mouseInArea = 1;

			xTile = world->xMouse / world->tileSet.width;
			yTile = world->yMouse / world->tileSet.height;
		}

		if (xTile < 0)
			xTile = 0;
		if (xTile > world->tileSet.nCol - 1)
			xTile = world->tileSet.nCol - 1;

		if (yTile < 0)
			yTile = 0;
		if (yTile > world->tileSet.nRow - 1)
			yTile = world->tileSet.nRow - 1;

		SDL_BlitSurface(world->tileSet.imageData, NULL, screen, NULL);

		selectedTile.x = xTile * world->tileSet.width;
		selectedTile.y = yTile * world->tileSet.height;
		selectedTile.w = world->tileSet.width;
		selectedTile.h = world->tileSet.height;

		colorArea(screen, &selectedTile, 0xffffff40);

		if (keyIsHit(world, KEY_SPECIAL) || (mouseInArea && keyIsHit(world, MOUSE_LEFT)))
		{
			world->map.mapData[mapIndex][xView + xShift][yView + yShift] = selectedTile;

			mode = 0;
		}

		break;
	case 2:
		colorArea(world->spriteGround, &check, 0x000000ff);
		drawText(world->spriteGround, &check, &world->font[FONT_WHITE], "Are you sure? ( To %s )", world->map.path);

		if (keyIsHit(world, KEY_BACK))
			mode = 0;

		if (keyIsHit(world, KEY_SELECT))
		{

			FILE *file;
			int mapIndex;

			file = fopen(world->map.path, "w");
			assert(file != NULL);

			fprintf(file, "%d\n%s\n", world->worldID, world->tileSet.path);
			fprintf(file, "%d,%d\n", world->tileSet.width, world->tileSet.height);
			fprintf(file, "%d,%d\n", world->map.nCol, world->map.nRow);

			for (mapIndex = 0; mapIndex < 3; ++mapIndex)
			{
				for (yMap = 0; yMap < world->map.nRow; ++yMap)
					for (xMap = 0; xMap < world->map.nCol; ++xMap)
					{
						fprintf(file, "%d.%d", world->map.mapData[mapIndex][xMap][yMap].x / world->tileSet.width, world->map.mapData[mapIndex][xMap][yMap].y / world->tileSet.height);

						if (xMap == world->map.nCol - 1)
							putc('\n', file);
						else
							putc(',', file);
					}

				fprintf(file, ";\n");
			}

			for (yMap = 0; yMap < world->map.nRow; ++yMap)
				for (xMap = 0; xMap < world->map.nCol; ++xMap)
				{
					fprintf(file, "%d", world->map.collision[xMap][yMap]);

					if (xMap == world->map.nCol - 1)
						putc('\n', file);
					else
						putc(',', file);
				}

			fclose(file);

			mode = 3;
		}

		break;
	case 3:
		colorArea(world->spriteGround, &check, 0xff);
		drawText(world->spriteGround, &check, &world->font[FONT_WHITE], "Attempted save.");

		if (keyIsHit(world, KEY_SELECT) || keyIsHit(world, KEY_BACK))
			mode = 0;

		break;
	default:
		if (keyIsHit(world, KEY_BACK))
		{
			++mapIndex;

			if (mapIndex > 3)
				mapIndex = 0;
		}

		if (keyIsHit(world, KEY_SPECIAL))
		{
			if (mapIndex == 3)
				world->map.collision[xView + xShift][yView + yShift] = world->map.collision[xView + xShift][yView + yShift] ? 0 : 1;
			else
				mode = 1;
		}

		if (keyIsHit(world, KEY_SELECT))
		/* Output file */
			mode = 2;

		break;
	}

	SDL_BlitScaled(world->spriteGround, NULL, screen, renderArea);
}

#endif
