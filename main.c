#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "engine.h"

SDL_Surface *colorArea_surface;

void cleanUp(void)
{
	Mix_CloseAudio();
	SDL_Quit();
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
	    isDown;
	struct world world;

	Mix_Music *music;

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

	loadFont("font.bmp", &world.font[REGULAR], 10, 4, 7, 0);
	loadWorld("over.world", &world);

	world.keyMap[0] = 0;
	world.keyMap[1] = 0;

	music = Mix_LoadMUS("music.ogg");
	Mix_PlayMusic(music, -1);

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

#			define modifyKey(modifyKey_key)\
				if (world.keyMap[1] & modifyKey_key)\
				{\
					if (!isDown)\
						world.keyMap[1] -= modifyKey_key;\
				} else\
				if (isDown)\
					world.keyMap[1] += modifyKey_key;

				isDown = event.type == SDL_KEYDOWN ? 1 : 0;

				switch (event.key.keysym.sym)
				{
				case SDLK_UP:      modifyKey(KEY_UP);       break;
				case SDLK_RIGHT:   modifyKey(KEY_RIGHT);    break;
				case SDLK_DOWN:    modifyKey(KEY_DOWN);     break;
				case SDLK_LEFT:    modifyKey(KEY_LEFT);     break;
				case SDLK_SPACE:   modifyKey(KEY_SPECIAL);  break;
				case SDLK_TAB:     modifyKey(KEY_MENU);     break;
				case SDLK_RETURN:  modifyKey(KEY_SELECT);   break;
				case SDLK_LSHIFT:  modifyKey(KEY_BACK);     break;
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

		SDL_FillRect(screen, NULL, 0);

#	ifdef EDITOR
		worldEditor(&world, screen, &renderArea);
#	else
		loopWorld(&world, screen, &renderArea);
#	endif

		world.keyMap[0] = world.keyMap[1];

		SDL_UpdateWindowSurface(window);
	}

	freeFont(&world.font[REGULAR]);
	freeWorld(&world);

	SDL_FreeSurface(colorArea_surface);

	Mix_FreeMusic(music);

	exit(EXIT_SUCCESS);
}
