/*
 * gui_sdl_example - Load and display an image using GuiSdl (GuiGen + SDL).
 *
 * Opens a window via SdlGraphicalInterface, loads an image (default
 * test_image.bmp), and runs a main loop drawing the image or a fallback
 * rectangle. Close window to exit.
 */

#include "GuiSdl.h"
#include "utils/smart_pointer.h"

#include "wrappers/color.h"
#include <iostream>

int main(int argc, char **args) {
  std::cout << "[gui_sdl_example] Starting...\n";

  const char *imageFile = (argc > 1) ? args[1] : "test_image.bmp";

  smart_pointer<SdlGraphicalInterface> pGr;
  try {
    pGr = make_smart(new SdlGraphicalInterface(Size(640, 480)));
  } catch (GraphicalInterfaceException &e) {
    std::cerr << "SdlGraphicalInterface init failed: " << e.GetDescription(true)
              << "\n";
    return 1;
  }

  pGr->SetTitle("gui_sdl_example");

  SdlImage *pImg = 0;
  try {
    pImg = pGr->LoadImage(imageFile);
  } catch (GraphicalInterfaceException &e) {
    std::cerr << "LoadImage failed for " << imageFile << ": "
              << e.GetDescription(true) << "\n";
    std::cout << "[gui_sdl_example] Window open (no image). "
              << "Close window to exit.\n";
  }

  bool running = true;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        running = false;
    }

    /* Clear with background color (matching SDL_ex3_image). */
    pGr->DrawRectangle(Rectangle(Size(640, 480)), Color(40, 44, 52), false);
    if (pImg) {
      pGr->DrawImage(Point(0, 0), pImg, pImg->GetRectangle(), true);
    } else {
      /* Fallback rectangle when image failed to load. */
      pGr->DrawRectangle(Rectangle(100, 150, 540, 330), Color(200, 80, 80),
                         true);
    }
    pGr->RefreshAll();
    SDL_Delay(16);
  }

  if (pImg)
    pGr->DeleteImage(pImg);

  pGr = smart_pointer<SdlGraphicalInterface>();
  /* Destructor calls SDL_Quit(). */

  std::cout << "[gui_sdl_example] Exiting.\n";
  return 0;
}
