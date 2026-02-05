#include "Preloader.h"

namespace Gui {

bool ImageSequence::Toggle() {
  if (vImage.empty())
    return true;

  if (nActive == vImage.size() - 1) {
    nActive = 0;
    return true;
  } else {
    nActive++;
  }
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