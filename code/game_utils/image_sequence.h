#ifndef IMAGE_SEQUENCE_HEADER
#define IMAGE_SEQUENCE_HEADER

/*
 * ImageSequence and ForEachImage: multi-frame image playback with timing.
 * Used by Preloader and drawing/animation code.
 */

#include <string>
#include <vector>

#include "../utils/index.h"
#include "../utils/timer.h"

/** Sequence of image indices with timing; Toggle / ToggleTimed for animation.
 */
class ImageSequence {
public:
  /** Advance to next frame; wraps to 0 at end. Returns true on wrap. */
  bool Toggle();
  /** Advance frame when timer expires; restarts timer for current interval. */
  bool ToggleTimed();

  Index GetImage();
  int GetTime() const;
  /** Sum of all frame intervals; 1 if empty. */
  int GetTotalTime() const;

  void Add(Index nImg, int nTime = 1);
  void INIT();

  ImageSequence();
  ImageSequence(Index img1);
  ImageSequence(Index img1, Index img2);
  ImageSequence(Index img1, Index img2, Index img3);

  /** Number of frames in the sequence. */
  int GetImageCount() const;
  /** True if the sequence has no frames. */
  bool IsEmpty() const;
  /** Image index at position i. */
  Index GetImageAt(int i) const;
  /** Replace the image index at position i. */
  void SetImageAt(int i, Index img);

  /** Current active frame index. */
  int GetActive() const;
  /** Set the current active frame index. */
  void SetActive(int n);

  /** Number of frame intervals stored. */
  int GetIntervalCount() const;
  /** Interval value at position i. */
  int GetIntervalAt(int i) const;

private:
  std::vector<Index> vImage;
  std::vector<int> vIntervals;
  int nActive;
  Timer t;
};

/** Reset sequence to first frame. */
inline ImageSequence Reset(ImageSequence imgSeq) {
  imgSeq.SetActive(0);
  return imgSeq;
}

/** Applies a functor to each image index in an ImageSequence.
 *  The functor receives Index& and may modify it in place. */
template <class T> void ForEachImage(ImageSequence &img, T t) {
  for (int i = 0; i < img.GetImageCount(); ++i) {
    Index idx = img.GetImageAt(i);
    t(idx);
    img.SetImageAt(i, idx);
  }
}

#endif // IMAGE_SEQUENCE_HEADER
