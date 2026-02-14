#ifndef TOWER_DEFENSE_MOUSE_UTILS_H
#define TOWER_DEFENSE_MOUSE_UTILS_H

/* MouseCursor, PositionTracker, TrackballTracker: cursor image/position,
 * position tracking for camera/flight, trackball-style steering.
 * Uses geometry.h for Point, fPoint. Defines MouseTracker. */

#include "Preloader.h"
#include "wrappers/geometry.h"
#include "utils/index.h"

#include <list>

namespace Gui {

template <class T>
class GraphicalInterface;

/* Tracks mouse position and relative movement: OnMouse(Point),
 * GetRelMovement(). */
struct MouseTracker {
  Point pLastRead;
  Point pCurPos;

  bool bInitialized;

  MouseTracker() : bInitialized(false) {}

  void OnMouse(Point p) {
    pCurPos = p;
    if (!bInitialized) {
      bInitialized = true;
      pLastRead = p;
    }
  }

  Point GetRelMovement() {
    if (!bInitialized)
      return Point(0, 0);

    Point pRet = pCurPos - pLastRead;
    pLastRead = pCurPos;
    return pRet;
  }
};

} // namespace Gui

using Gui::fPoint;
using Gui::MouseTracker;
using Gui::Point;

/** Tracks mouse for trackball-style steering (angle and fire). */
struct TrackballTracker {
  MouseTracker mtr;

  std::list<Point> lsMouse;
  int nMaxLength;
  bool trigFlag;
  int threshold;
  TrackballTracker();

  void Update();

  bool IsTrigger();

  Point GetMovement();
  fPoint GetAvMovement();
  int GetLengthSq(Point p);
  int GetDerivative();
};

/** Tracks mouse position, last down, press state and counter for input. */
struct PositionTracker {
  Point pMouse;
  Point pLastDownPosition;
  bool bPressed;
  int nCounter;

  PositionTracker() : bPressed(false) {}

  void On();
  int Off();
  void Update();

  void UpdateMouse(Point pMouse_);
  void UpdateLastDownPosition(Point pMouse_);
  fPoint GetDirection(fPoint fDragonPos) { return pMouse - fDragonPos; }
  fPoint GetFlightDirection(fPoint fDragonPos);
};

/** Cursor image and position; Draw/Update for rendering and click state. */
struct MouseCursor {
  bool bPressed;
  Gui::ImageSequence imgCursor;
  Point pCursorPos;

  MouseCursor(Gui::ImageSequence imgCursor_, Point pCursorPos_)
      : imgCursor(imgCursor_), pCursorPos(pCursorPos_), bPressed(false) {}

  void DrawCursor(Gui::GraphicalInterface<Gui::Index> *pGr);
  void SetCursorPos(Point pPos);
};

#endif
