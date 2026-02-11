/*
 * test_sdl - Catch2 tests for SDL image/sound loading and wrapper interfaces.
 * Run from a directory containing test_image.bmp and test_sound.wav.
 */

#include "GuiSdl.h"
#include "SuiSdl.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <catch2/catch.hpp>

TEST_CASE("SDL image and sound load", "[sdl]") {
  const char *imageFile = "test_image.bmp";
  const char *soundFile = "test_sound.wav";

  REQUIRE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) >= 0);

  SDL_Window *win = SDL_CreateWindow("", 0, 0, 1, 1, 0);
  REQUIRE(win != nullptr);

  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);
  REQUIRE(ren != nullptr);

  SDL_Texture *tex = IMG_LoadTexture(ren, imageFile);
  REQUIRE(tex != nullptr);

  REQUIRE(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) >= 0);

  Mix_Chunk *chunk = Mix_LoadWAV(soundFile);
  REQUIRE(chunk != nullptr);

  // Cleanup
  SDL_DestroyTexture(tex);
  Mix_FreeChunk(chunk);
  Mix_CloseAudio();
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

TEST_CASE("SdlGraphicalInterface::LoadImage", "[sdl][gui]") {
  smart_pointer<Gui::SdlGraphicalInterface> pGr =
      make_smart(new Gui::SdlGraphicalInterface(Gui::Size(640, 480)));
  Gui::SdlImage *pImg = pGr->LoadImage("test_image.bmp");
  REQUIRE(pImg != nullptr);
  pGr->DeleteImage(pImg);
}

TEST_CASE("SdlSoundInterface::LoadSound", "[sdl][sui]") {
  REQUIRE(SDL_Init(SDL_INIT_AUDIO) >= 0);
  smart_pointer<Gui::SdlSoundInterface> pSnd =
      make_smart(new Gui::SdlSoundInterface());
  Mix_Chunk *chunk = pSnd->LoadSound("test_sound.wav");
  REQUIRE(chunk != nullptr);
  pSnd->DeleteSound(chunk);
  Mix_CloseAudio();
  SDL_Quit();
}
