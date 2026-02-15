#ifndef IMAGE_SEQUENCE_HEADER
#define IMAGE_SEQUENCE_HEADER

/*
 * ImageSequence and ForEachImage: multi-frame image playback with timing.
 * Used by Preloader and drawing/animation code.
 */

#include <string>
#include <vector>

#include "utils/index.h"
#include "utils/smart_pointer.h"
#include "utils/timer.h"

/** Sequence of image indices with timing; Toggle / ToggleTimed for animation.
 */
struct ImageSequence : virtual public SP_Info {
  std::string get_class_name() override { return "ImageSequence"; }
  std::vector<Index> vImage;
  std::vector<unsigned> vIntervals;
  unsigned nActive;

  Timer t;

  /** Advance to next frame; wraps to 0 at end. Returns true on wrap. */
  bool Toggle();
  /** Advance frame when timer expires; restarts timer for current interval. */
  bool ToggleTimed();

  Index GetImage();
  unsigned GetTime() const;
  /** Sum of all frame intervals; 1 if empty. */
  unsigned GetTotalTime() const;

  void Add(Index nImg, unsigned nTime = 1);
  void INIT();

  ImageSequence();
  ImageSequence(Index img1);
  ImageSequence(Index img1, Index img2);
  ImageSequence(Index img1, Index img2, Index img3);
};

/** Reset sequence to first frame. */
inline ImageSequence Reset(ImageSequence imgSeq) {
  imgSeq.nActive = 0;
  return imgSeq;
}

/** Applies a functor to each image index in an ImageSequence. */
template <class T> void ForEachImage(ImageSequence &img, T t) {
  for (unsigned i = 0; i < img.vImage.size(); ++i)
    t(img.vImage[i]);
}

#endif // IMAGE_SEQUENCE_HEADER
