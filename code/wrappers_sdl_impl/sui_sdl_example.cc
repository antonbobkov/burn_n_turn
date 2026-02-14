/*
 * sui_sdl_example - Load and play music and sound effects using SuiSdl (SuiGen
 * + SDL_mixer).
 *
 * Plays background music (default: test_music.wav) and a sound on mouse click
 * (default: test_sound.wav). Space = pause/resume, Escape = stop.
 * Close the window to exit.
 */

#include "SuiSdl.h"
#include "utils/smart_pointer.h"

#include "utils/exception.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>

int main(int argc, char **args) {
  std::cout << "[sui_sdl_example] Starting...\n";

  const char *musicFile = (argc > 1) ? args[1] : "test_music.wav";
  const char *soundFile = (argc > 2) ? args[2] : "test_sound.wav";

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  smart_pointer<SdlSoundInterface> pSnd =
      make_smart(new SdlSoundInterface());
SimpleSoundInterface<Mix_Chunk *> soundIf(pSnd);

  SDL_Window *window =
      SDL_CreateWindow("sui_sdl_example", SDL_WINDOWPOS_UNDEFINED,
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
      std::cout << "[sui_sdl_example] Playing music. "
                << "Space=pause, Esc=stop, click=sound.\n";
    }
  }

Index sndIndex;
  bool soundLoaded = false;
  try {
    sndIndex = soundIf.LoadSound(soundFile);
    soundLoaded = true;
  } catch (SimpleException &e) {
    std::cerr << "LoadSound failed for " << soundFile << ": "
              << e.GetDescription() << "\n";
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
            std::cout << "[sui_sdl_example] Music resumed.\n";
          } else {
            Mix_PauseMusic();
            std::cout << "[sui_sdl_example] Music paused.\n";
          }
        } else if (e.key.keysym.sym == SDLK_ESCAPE) {
          Mix_HaltMusic();
          std::cout << "[sui_sdl_example] Music stopped.\n";
        }
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (soundLoaded)
          soundIf.PlaySound(sndIndex, -1, false);
      }
    }

    SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  if (music)
    Mix_FreeMusic(music);
  if (soundLoaded)
    soundIf.DeleteSound(sndIndex);
  pSnd = smart_pointer<SdlSoundInterface>();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  Mix_CloseAudio();
  SDL_Quit();

  std::cout << "[sui_sdl_example] Exiting.\n";
  return 0;
}
