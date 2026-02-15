#include "core.h"
#include "game_utils/draw_utils.h"
#include "dragon_constants.h"
#include "dragon_macros.h"
#include "game.h"
#include "game_utils/sound_utils.h"
#include "game_utils/image_sequence.h"
#include "utils/smart_pointer.h"
#include "wrappers/font_writer.h"
#include "wrappers/color.h"

MessageWriter *pWr = 0;

int nSlimeMax = 100;

void DrawStuff(Rectangle rBound,
               smart_pointer<GraphicalInterface<Index>> pGraph,
               smart_pointer<SoundInterface<Index>> pSnd, Preloader &pr,
               int n) {
#ifdef LOADING_SCREEN
  rBound.sz.x *= 2;
  rBound.sz.y *= 2;

  Size sz1 = pGraph->GetImage(pr["splash"])->GetSize();
  Size sz2 = pGraph->GetImage(pr["loading"])->GetSize();

  Point p1(Crd(rBound.sz.x / 2), Crd(rBound.sz.y / 2));
#ifdef SMALL_SCREEN_VERSION
        Point p2(Crd(rBound.sz.x/2, Crd(rBound.sz.y*7.0f/10);
#else
  Point p2(Crd(rBound.sz.x / 2), Crd(rBound.sz.y * 6.5f / 10));
#endif

	p1.x -= sz1.x/2;
	p1.y -= sz1.y/2;
	p2.x -= sz2.x/2;
	p2.y -= sz2.y/2;

	pGraph->DrawImage(p1, pr["splash"], false);

	pGraph->DrawImage(p2, pr["loading"], Rectangle(0,0,144*n/9,32), false);

	pGraph->DrawImage(p2, pr["loading"], Rectangle(0,0,144*n/9,32), false);

	pGraph->RefreshAll();

#ifndef PC_VERSION
	if(n%2)
		pSnd->PlaySound(pr.GetSnd("beep"));
	else pSnd->PlaySound(pr.GetSnd("boop"));
#endif

#endif // LOADING_SCREEN
}

const std::string sFullScreenPath = "fullscreen.txt";
