/*
 * SDL_ex3_image -- Load and display an image using SDL_image
 *
 * Opens a window and displays an image loaded from file (default: image.bmp).
 * Uses SDL_image for PNG/JPG support. Close the window to exit.
 */

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

int main(int argc, char **args) {

  std::cout << "[SDL_ex3_image] Starting...\n";

  const char *imageFile = (argc > 1) ? args[1] : "test_image.bmp";

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
  if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
    std::cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
    SDL_Quit();
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("SDL_ex3_image", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_Texture *texture = IMG_LoadTexture(renderer, imageFile);
  if (!texture) {
    std::cerr << "IMG_LoadTexture failed for " << imageFile << ": "
              << IMG_GetError() << "\n";
    std::cout << "[SDL_ex3_image] Window open (no image). "
              << "Close window to exit.\n";
  }

  bool running = true;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        running = false;
    }

    SDL_SetRenderDrawColor(renderer, 40, 44, 52, 255);
    SDL_RenderClear(renderer);

    if (texture) {
      SDL_RenderCopy(renderer, texture, NULL, NULL);
    } else {
      SDL_SetRenderDrawColor(renderer, 200, 80, 80, 255);
      SDL_Rect r = {100, 150, 440, 180};
      SDL_RenderFillRect(renderer, &r);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  if (texture)
    SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();

  std::cout << "[SDL_ex3_image] Exiting.\n";
  return 0;
}
