/*
 * SDL_ex2_interactive -- Physics demo with user interaction
 *
 * This program displays a window in which blue rectangles fall (or rise) under
 * gravity, bounce off the top and bottom edges, and disappear after a few
 * seconds. One rectangle is spawned at startup, and new ones are spawned
 * automatically every 1–2 seconds at random horizontal positions. The user
 * can also spawn rectangles and change behavior with the keyboard and mouse.
 *
 * USER INTERACTION
 * ----------------
 *
 * Keyboard:
 *   Escape  -- Quit the program.
 *   C       -- Clear all rectangles from the screen.
 *   Space   -- Spawn a new rectangle at the center of the window with an
 *               upward impulse (or downward when gravity is reversed).
 *   G       -- Toggle gravity direction (down <-> up). Rectangles will
 *               fall or rise accordingly.
 *
 * Mouse:
 *   Click   -- Spawn a new rectangle at the click position with random size
 *               and velocity. The initial vertical velocity opposes the
 *               current gravity direction.
 *
 * Closing the window (e.g. via the close button) also quits the program.
 */

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>

#include <SDL.h>

using namespace std;

// Gravity in pixels per second squared
const float GRAVITY = 750.0f;

bool init();
void kill();
void loop();

SDL_Window* window;
SDL_Renderer* renderer;

// Keyboard-controlled: 1 = down, -1 = up
float gravityDirection = 1.0f;

struct square {
	float x, y, w, h, xvelocity, yvelocity;
	Uint32 born, lastUpdate;
};

void spawnRandomSquare(vector<square>& squares) {
	cout << "[SDL_ex2_interactive] Spawning random square\n";
	square s;
	s.w = rand() % 50 + 25;
	s.h = rand() % 50 + 25;
	s.x = rand() % (640 - (int)s.w);
	s.y = (gravityDirection > 0) ? 0 : (480 - s.h);  // top or bottom by gravity
	s.yvelocity = (gravityDirection > 0) ? -400 : 400;
	s.xvelocity = rand() % 400 - 200;
	s.lastUpdate = SDL_GetTicks();
	s.born = SDL_GetTicks();
	squares.push_back(s);
}

int main(int argc, char** args) {

	cout << "[SDL_ex2_interactive] Starting...\n";

	if ( !init() ) {
		system("pause");
		return 1;
	}

	cout << "[SDL_ex2_interactive] Window open. "
	     << "Use keys (C, Space, G, Esc) and mouse clicks.\n";

	loop();

	kill();
	cout << "[SDL_ex2_interactive] Exiting.\n";
	return 0;
}

void loop() {

	srand((unsigned)time(NULL));

	// Physics squares
	vector<square> squares;

	// Spawn one at start
	spawnRandomSquare(squares);

	// Random spawn: next spawn in 1–2 seconds
	Uint32 lastSpawnTime = SDL_GetTicks();
	Uint32 nextSpawnDelay = 1000 + rand() % 1000;

	bool running = true;
	while(running) {

		SDL_Event e;

		// Random spawn over time
		Uint32 now = SDL_GetTicks();
		if (now - lastSpawnTime >= nextSpawnDelay) {
			spawnRandomSquare(squares);
			lastSpawnTime = now;
			nextSpawnDelay = 1000 + rand() % 1000;
		}

		SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
		SDL_RenderClear( renderer );

		// Event loop
		while ( SDL_PollEvent( &e ) != 0 ) {
			switch (e.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym) {
						case SDLK_ESCAPE:
							cout << "[SDL_ex2] Key: Escape (quit)\n";
							running = false;
							break;
						case SDLK_c:
							cout << "[SDL_ex2] Key: C (clear all)\n";
							squares.clear();
							break;
						case SDLK_SPACE:
							cout << "[SDL_ex2] Key: Space (spawn center)\n";
							{
								square s;
								s.x = 320 - 25;
								s.y = 240 - 25;
								s.w = 50;
								s.h = 50;
								s.yvelocity = -400;
								s.xvelocity = rand() % 200 - 100;
								s.lastUpdate = SDL_GetTicks();
								s.born = SDL_GetTicks();
								squares.push_back(s);
							}
							break;
						case SDLK_g:
							gravityDirection = -gravityDirection;
							cout << "[SDL_ex2] Key: G (gravity "
							     << (gravityDirection > 0 ? "down" : "up") << ")\n";
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					cout << "[SDL_ex2] Mouse click at (" << e.button.x
					     << ", " << e.button.y << ")\n";
					{
						square s;
						s.x = e.button.x;
						s.y = e.button.y;
						s.w = rand() % 50 + 25;
						s.h = rand() % 50 + 25;
						s.yvelocity = -500 * gravityDirection;
						s.xvelocity = rand() % 500 - 250;
						s.lastUpdate = SDL_GetTicks();
						s.born = SDL_GetTicks();
						squares.push_back(s);
					}
					break;
			}
		}

		// Physics loop
		for (int index = 0; index < squares.size(); index++) {
			square& s = squares[index];

			Uint32 time = SDL_GetTicks();
			float dT = (time - s.lastUpdate) / 1000.0f;

			s.yvelocity += dT * GRAVITY * gravityDirection;
			s.y += s.yvelocity * dT;
			s.x += s.xvelocity * dT;

			// Bounce off bottom
			if (s.y > 480 - s.h) {
				s.y = 480 - s.h;
				s.xvelocity *= 0.9f;
				s.yvelocity = 0;
			}
			// Bounce off top (when gravity reversed)
			if (s.y < 0) {
				s.y = 0;
				s.xvelocity *= 0.9f;
				s.yvelocity = 0;
			}

			s.lastUpdate = time;
			if (s.lastUpdate > s.born + 5000) {
				squares.erase(squares.begin() + index);
				index--;
			}
		}

		// Render loop - draw blue rectangles instead of texture
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		for (const square& s : squares) {
			SDL_Rect dest = { (int)round(s.x), (int)round(s.y),
			                  (int)round(s.w), (int)round(s.h) };
			SDL_RenderFillRect(renderer, &dest);
		}

		// Delay for a random number of ticks - frame rate is variable;
		// physics is independent of frame rate.
		SDL_Delay(rand() % 25);

		SDL_RenderPresent(renderer);
	}
}

bool init() {
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		return false;
	}

	window = SDL_CreateWindow("Example",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
		SDL_WINDOW_SHOWN);
	if ( !window ) {
		cout << "Error creating window: " << SDL_GetError()  << endl;
		return false;
	}

	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if ( !renderer ) {
		cout << "Error creating renderer: " << SDL_GetError() << endl;
		return false;
	}

	return true;
}

void kill() {
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	window = NULL;
	renderer = NULL;

	SDL_Quit();
}
