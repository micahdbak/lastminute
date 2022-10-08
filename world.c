#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "world.h"

#define SIZE_TILESETPATH	256

void loadWorld(char *path, struct world *world)
{
	FILE *worldFile;
	char tileSetPath[SIZE_TILESETPATH];
	int xMap, yMap,
	    xTile, yTile,
	    readData;

	worldFile = fopen(path, "r");

	if (worldFile == NULL)
	{
		fprintf(stderr, "Could not open world file %s.\n", path);

		exit(EXIT_FAILURE);
	}

	fscanf(worldFile, "%s\n", tileSetPath);
	world->tileSet.imageData = SDL_LoadBMP(tileSetPath);

	if (world->tileSet.imageData == NULL)
	{
		fprintf(stderr, "Could not open tile set file %s.\n", tileSetPath);

		exit(EXIT_FAILURE);
	}

	fscanf(worldFile, "%d,%d\n", &world->tileSet.width, &world->tileSet.height);

	world->tileSet.nCol = world->tileSet.imageData->w / world->tileSet.width;
	world->tileSet.nRow = world->tileSet.imageData->h / world->tileSet.height;

	world->map.nCol = world->map.nRow = 0;
	fscanf(worldFile, "%d,%d\n", &world->map.nCol, &world->map.nRow);

	if (world->map.nCol == 0 || world->map.nRow == 0)
	{
		fprintf(stderr, "The world %s is not of a valid scale (%d,%d).\n", path, world->map.nCol, world->map.nRow);

		exit(EXIT_FAILURE);
	}

	world->map.mapData = calloc(world->map.nCol, sizeof(SDL_Rect *));


	if (world->map.mapData == NULL)
		exit(EXIT_FAILURE);

	for (xMap = 0; xMap < world->map.nCol; ++xMap)
	{
		world->map.mapData[xMap] = calloc(world->map.nRow, sizeof(SDL_Rect));

		if (world->map.mapData[xMap] == NULL)
			exit(EXIT_FAILURE);
	}

	xMap = 0;
	yMap = 0;

	readData = 1;

	while (readData)
	{
		fscanf(worldFile, "%d.%d", &xTile, &yTile);

		world->map.mapData[xMap][yMap].x = xTile * world->tileSet.width;
		world->map.mapData[xMap][yMap].y = yTile * world->tileSet.height;
		world->map.mapData[xMap][yMap].w = world->tileSet.width;
		world->map.mapData[xMap][yMap].h = world->tileSet.height;

		switch (getc(worldFile))
		{
		case ',':
			++xMap;

			break;
		case '\n':
			xMap = 0;
			++yMap;

			break;
		default:
			readData = 0;

			break;
		}
	}

	world->drawWidth = 256;
	world->drawHeight = 224;

	world->draw = SDL_CreateRGBSurface(0, world->drawWidth, world->drawHeight, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	SDL_FillRect(world->draw, NULL, 0x000000ff);

	if (world->draw == NULL)
		exit(EXIT_FAILURE);

	world->keyMap = 0;
}

void freeWorld(struct world *world)
{
	int xMap, yMap;

	for (xMap = 0; xMap < world->map.nCol; ++xMap)
		free(world->map.mapData[xMap]);

	free(world->map.mapData);

	SDL_FreeSurface(world->tileSet.imageData);
	SDL_FreeSurface(world->draw);
}

void loopWorld(struct world *world, SDL_Surface *screen)
{
	int xMap, yMap;
	SDL_Rect render;

	render.w = world->tileSet.width;
	render.h = world->tileSet.height;

	for (xMap = 0; xMap < world->map.nCol; ++xMap)
		for (yMap = 0; yMap < world->map.nRow; ++yMap)
		{
			render.x = xMap * world->tileSet.width;
			render.y = yMap * world->tileSet.height;

			SDL_BlitSurface(world->tileSet.imageData, &world->map.mapData[xMap][yMap], world->draw, &render);
		}

	SDL_BlitScaled(world->draw, NULL, screen, NULL);

	//SDL_BlitScaled(world->tileSet.imageData, NULL, screen, NULL);
}
