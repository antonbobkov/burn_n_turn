/*
 * SDL_ex1_basic -- Minimal SDL2 window example
 *
 * This program opens a 640x480 window, fills it with cyan (0x00, 0xFF, 0xFF),
 * and keeps the window open until the user closes it (e.g. by clicking the
 * window close button). It demonstrates basic SDL initialization, window
 * creation, surface-based drawing with SDL_FillRect, and an event loop
 * that only handles SDL_QUIT. No keyboard or mouse input is processed beyond
 * closing the window.
 */

#include <SDL.h>
#include <stdio.h>
#include <iostream>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main( int argc, char* args[] )
{
	std::cout << "[SDL_ex1_basic] Starting...\n";

	//The window we'll be rendering to
	SDL_Window* window = NULL;
	
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("SDL Tutorial",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			SDL_FillRect(screenSurface, NULL,
				SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0xFF));
			
			//Update the surface
			SDL_UpdateWindowSurface( window );

			std::cout << "[SDL_ex1_basic] Window open. Close the window to exit.\n";

            //Hack to get window to stay up
            SDL_Event e;
            bool quit = false;
            while (!quit) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) quit = true;
                }
            }
		}
	}

	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	std::cout << "[SDL_ex1_basic] Exiting.\n";
	return 0;
}
