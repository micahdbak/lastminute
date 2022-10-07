#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>

#include "sprite.h"

#define SWIDTH	640
#define SHEIGHT	480

SDL_Window *window;
SDL_Surface *screen;

void cleanUp(void)
{
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	SDL_Event event;
	int newTicks, prevTicks = 0;
	float ticks;
	double delta;
	int quit = 0;
	struct sprite randomGuy;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
	{
		fprintf(stderr, "Couldn't Initialize SDL: %s\n", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	atexit(cleanUp);

	window = SDL_CreateWindow("Ventura De Espanol En Canada", 0, 0, SWIDTH, SHEIGHT, SDL_WINDOW_RESIZABLE);
	screen = SDL_GetWindowSurface(window);

	if (window == NULL || screen == NULL)
	{
		fprintf(stderr, "Couldn't open a window: %s\n", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	printf("Success!\n");

	loadSprite("randomGuy.bmp", &randomGuy, 16, 16);

	randomGuy.x = 16.0f;
	randomGuy.y = 16.0f;

	while (!quit)
	{
		newTicks = SDL_GetTicks();
		delta = (float)(newTicks - prevTicks) / 1000.0f;
		prevTicks = newTicks;

		while (SDL_PollEvent(&event))
			switch (event.type)
			{
			case SDL_QUIT:
				quit = 1;

				break;
			default:

				break;
			}

		randomGuy.x += delta * 8.0f;

		if (randomGuy.x > 480.0f)
			randomGuy.x = 16.0f;

		SDL_FillRect(screen, NULL, 0xff0000ff);
		renderSprite(&randomGuy, screen);
		SDL_UpdateWindowSurface(window);
	}

	exit(EXIT_SUCCESS);
}
