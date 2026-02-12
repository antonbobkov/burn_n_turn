/*
 * SDL_ex4_sound -- Load and play music and sound effects using SDL_mixer
 *
 * Plays background music (default: music.wav) and a sound on mouse click
 * (default: sound.wav). Space = pause/resume, Escape = stop.
 * Close the window to exit.
 */

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>

int main(int argc, char **args) {

  std::cout << "[SDL_ex4_sound] Starting...\n";

  const char *musicFile = (argc > 1) ? args[1] : "test_music.wav";
  const char *soundFile = (argc > 2) ? args[2] : "test_sound.wav";

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
    std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << "\n";
    SDL_Quit();
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("SDL_ex4_sound", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
    Mix_CloseAudio();
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();
    return 1;
  }

  Mix_Music *music = Mix_LoadMUS(musicFile);
  if (!music) {
    std::cerr << "Mix_LoadMUS failed for " << musicFile << ": "
              << Mix_GetError() << "\n";
  } else {
    if (Mix_PlayMusic(music, -1) != 0) {
      std::cerr << "Mix_PlayMusic failed: " << Mix_GetError() << "\n";
    } else {
      std::cout << "[SDL_ex4_sound] Playing music. "
                << "Space=pause, Esc=stop, click=sound.\n";
    }
  }

  Mix_Chunk *sound = Mix_LoadWAV(soundFile);
  if (!sound) {
    std::cerr << "Mix_LoadWAV failed for " << soundFile << ": "
              << Mix_GetError() << "\n";
  }

  bool running = true;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
      } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_SPACE) {
          if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
            std::cout << "[SDL_ex4_sound] Music resumed.\n";
          } else {
            Mix_PauseMusic();
            std::cout << "[SDL_ex4_sound] Music paused.\n";
          }
        } else if (e.key.keysym.sym == SDLK_ESCAPE) {
          Mix_HaltMusic();
          std::cout << "[SDL_ex4_sound] Music stopped.\n";
        }
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (sound && Mix_PlayChannel(-1, sound, 0) == -1) {
          std::cerr << "Mix_PlayChannel failed: " << Mix_GetError() << "\n";
        }
      }
    }

    SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  if (music)
    Mix_FreeMusic(music);
  if (sound)
    Mix_FreeChunk(sound);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  Mix_CloseAudio();
  SDL_Quit();

  std::cout << "[SDL_ex4_sound] Exiting.\n";
  return 0;
}
