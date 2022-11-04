#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "engine.h"

SDL_Surface *colorArea_surface;

void cleanUp(void)
{
	Mix_CloseAudio();
	SDL_Quit();
}

void modifyKey(struct world *world, int isDown, int key)
{
	if (world->keyMap[1] & key)
	{
		if (!isDown)
			world->keyMap[1] -= key;
	} else
	if (isDown)
		world->keyMap[1] += key;
}

int main(int argc, char *argv[])
{
	SDL_Window *window;
	SDL_Surface *screen;

	int windowWidth = 1024, windowHeight = 768;
	float screenRatio, windowRatio;
	SDL_Rect renderArea;

	SDL_Event event;
	int newTicks, prevTicks = 0,
	    quit = 0,
	    isDown,
	    mouseState;
	struct world world;

	srand(time(0));

	assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO) == 0);
	assert(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) != -1);
	atexit(cleanUp);

	window = SDL_CreateWindow("Explorer", 0, 0, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
	assert(window != NULL);
	screen = SDL_GetWindowSurface(window);
	assert(screen != NULL);

	colorArea_surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	renderArea.x = 0;
	renderArea.y = 0;
	renderArea.w = windowWidth;
	renderArea.h = windowHeight;

	world.screenWidth = 256;
	world.screenHeight = 192;

	screenRatio = (float)world.screenWidth / (float)world.screenHeight;

	loadFont("whiteFont.bmp", &world.font[FONT_WHITE], 12, 3, 6, 18,
	         ML('C', 5), ML('I', 5), ML('J', 5), ML('L', 5), ML('M', 8), ML('N', 7), ML('T', 7), ML('W', 8),
		 ML('c', 5), ML('f', 5), ML('i', 3), ML('j', 4), ML('l', 4), ML('m', 9), ML('r', 5), ML('s', 5), ML('t', 5), ML('w', 8));
	loadWorld("over.world", &world);

	world.keyMap[0] = 0;
	world.keyMap[1] = 0;

	while (!quit)
	{
		while (SDL_PollEvent(&event))
			switch (event.type)
			{
			case SDL_QUIT:
				quit = 1;

				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED
				 || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					screen = SDL_GetWindowSurface(window);
					SDL_GetWindowSize(window, &windowWidth, &windowHeight);

					windowRatio = (float)windowWidth / (float)windowHeight;

					if (windowRatio == screenRatio)
					{
						renderArea.w = windowWidth;
						renderArea.h = windowHeight;
						renderArea.x = 0;
						renderArea.y = 0;
					} else
					if (windowRatio > screenRatio)
					{
						renderArea.w = (int)((float)windowHeight * screenRatio);
						renderArea.h = windowHeight;
						renderArea.x = (windowWidth - renderArea.w) / 2;
						renderArea.y = 0;
					} else {
					/* screenRatio > windowRatio */
						renderArea.w = windowWidth;
						renderArea.h = (int)((float)windowWidth / screenRatio);
						renderArea.x = 0;
						renderArea.y = (windowHeight - renderArea.h) / 2;
					}

					SDL_FillRect(screen, NULL, 0);
				}

				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				isDown = event.type == SDL_KEYDOWN ? 1 : 0;

				switch (event.key.keysym.sym)
				{
				case SDLK_UP:      modifyKey(&world, isDown, KEY_UP);       break;
				case SDLK_RIGHT:   modifyKey(&world, isDown, KEY_RIGHT);    break;
				case SDLK_DOWN:    modifyKey(&world, isDown, KEY_DOWN);     break;
				case SDLK_LEFT:    modifyKey(&world, isDown, KEY_LEFT);     break;
				case SDLK_SPACE:   modifyKey(&world, isDown, KEY_SPECIAL);  break;
				case SDLK_TAB:     modifyKey(&world, isDown, KEY_MENU);     break;
				case SDLK_RETURN:  modifyKey(&world, isDown, KEY_SELECT);   break;
				case SDLK_LSHIFT:  modifyKey(&world, isDown, KEY_BACK);     break;
				default:

					break;
				}

				break;
			default:

				break;
			}

		newTicks = SDL_GetTicks();
		world.delta = (float)(newTicks - prevTicks) / 1000.0f;
		prevTicks = newTicks;

		mouseState = SDL_GetMouseState(&world.xMouse, &world.yMouse);

		modifyKey(&world, mouseState & SDL_BUTTON_LMASK, MOUSE_LEFT);
		modifyKey(&world, mouseState & SDL_BUTTON_RMASK, MOUSE_RIGHT);

		SDL_FillRect(screen, NULL, 0);

#	ifdef EDITOR
		worldEditor(&world, screen, &renderArea);
#	else
		loopWorld(&world, screen, &renderArea);
#	endif
		world.keyMap[0] = world.keyMap[1];

		SDL_UpdateWindowSurface(window);
	}

	freeFont(&world.font[FONT_WHITE]);

#	ifndef EDITOR
	freeWorld(&world);
#	endif

	SDL_FreeSurface(colorArea_surface);

	exit(EXIT_SUCCESS);
}
