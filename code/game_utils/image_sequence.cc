#include "image_sequence.h"

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

int ImageSequence::GetTime() const {
  if (vIntervals.empty())
    return 1;
  return vIntervals[nActive];
}

int ImageSequence::GetTotalTime() const {
  int nRet = 0;
  for (int i = 0; i < (int)vIntervals.size(); ++i)
    nRet += vIntervals[i];
  if (nRet == 0)
    return 1;
  return nRet;
}

void ImageSequence::Add(Index nImg, int nTime) {
  vImage.push_back(nImg);
  vIntervals.push_back(nTime);
}

bool ImageSequence::Toggle() {
  if (vImage.empty())
    return true;
  if (nActive == (int)vImage.size() - 1) {
    nActive = 0;
    return true;
  }
  nActive++;
  return false;
}

int ImageSequence::GetImageCount() const { return (int)vImage.size(); }

bool ImageSequence::IsEmpty() const { return vImage.empty(); }

Index ImageSequence::GetImageAt(int i) const { return vImage[i]; }

void ImageSequence::SetImageAt(int i, Index img) { vImage[i] = img; }

int ImageSequence::GetActive() const { return nActive; }

void ImageSequence::SetActive(int n) { nActive = n; }

int ImageSequence::GetIntervalCount() const { return (int)vIntervals.size(); }

int ImageSequence::GetIntervalAt(int i) const { return vIntervals[i]; }

bool ImageSequence::ToggleTimed() {
  if (!t.IsActive())
    t = Timer(vIntervals.at(nActive));
  if (t.Tick()) {
    bool b = Toggle();
    t = Timer(vIntervals.at(nActive));
    return b;
  }
  return false;
}
