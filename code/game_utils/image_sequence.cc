#include "game_utils/image_sequence.h"

namespace Gui {

void ImageSequence::INIT() {
  nActive = 0;
  t = Timer(0);
}

ImageSequence::ImageSequence() { INIT(); }

ImageSequence::ImageSequence(Index img1) {
  INIT();
  Add(img1);
}

ImageSequence::ImageSequence(Index img1, Index img2) {
  INIT();
  Add(img1);
  Add(img2);
}

ImageSequence::ImageSequence(Index img1, Index img2, Index img3) {
  INIT();
  Add(img1);
  Add(img2);
  Add(img3);
}

Index ImageSequence::GetImage() { return vImage[nActive]; }

unsigned ImageSequence::GetTime() const {
  if (vIntervals.empty())
    return 1;
  return vIntervals[nActive];
}

unsigned ImageSequence::GetTotalTime() const {
  int nRet = 0;
  for (unsigned i = 0; i < vIntervals.size(); ++i)
    nRet += vIntervals[i];
  if (nRet == 0)
    return 1;
  return nRet;
}

void ImageSequence::Add(Index nImg, unsigned nTime) {
  vImage.push_back(nImg);
  vIntervals.push_back(nTime);
}

bool ImageSequence::Toggle() {
  if (vImage.empty())
    return true;
  if (nActive == vImage.size() - 1) {
    nActive = 0;
    return true;
  }
  nActive++;
  return false;
}

bool ImageSequence::ToggleTimed() {
  if (t.nPeriod == 0)
    t = Timer(vIntervals.at(nActive));
  if (t.Tick()) {
    bool b = Toggle();
    t = Timer(vIntervals.at(nActive));
    return b;
  }
  return false;
}

} // namespace Gui
