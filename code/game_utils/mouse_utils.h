#ifndef TOWER_DEFENSE_MOUSE_UTILS_H
#define TOWER_DEFENSE_MOUSE_UTILS_H

/* MouseCursor, PositionTracker, TrackballTracker: cursor image/position,
 * position tracking for camera/flight, trackball-style steering.
 * Uses geometry.h for Point, fPoint. Defines MouseTracker. */

#include "Preloader.h"
#include "image_sequence.h"
#include "../utils/index.h"
#include "../wrappers/geometry.h"


#include <list>

template <class T> class GraphicalInterface;

/* Tracks mouse position and relative movement: OnMouse(Point),
 * GetRelMovement(). */
class MouseTracker {
public:
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

private:
  Point pLastRead;
  Point pCurPos;
  bool bInitialized;
};

/** Tracks mouse for trackball-style steering (angle and fire). */
class TrackballTracker {
public:
  TrackballTracker();

  void Update();

  bool IsTrigger();

  Point GetMovement();
  fPoint GetAvMovement();
  int GetLengthSq(Point p);
  int GetDerivative();

private:
  MouseTracker mtr;
  std::list<Point> lsMouse;
  int nMaxLength;
  bool trigFlag;
  int threshold;
};

/** Tracks mouse position, last down, press state and counter for input. */
class PositionTracker {
public:
  PositionTracker() : bPressed(false) {}

  void On();
  int Off();
  void Update();

  void UpdateMouse(Point pMouse_);
  void UpdateLastDownPosition(Point pMouse_);
  fPoint GetDirection(fPoint fDragonPos) { return pMouse - fDragonPos; }
  fPoint GetFlightDirection(fPoint fDragonPos);

  bool IsPressed() const { return bPressed; }

private:
  Point pMouse;
  Point pLastDownPosition;
  bool bPressed;
  int nCounter;
};

/** Cursor image and position; Draw/Update for rendering and click state. */
class MouseCursor {
public:
  MouseCursor(ImageSequence imgCursor_, Point pCursorPos_)
      : bPressed(false), imgCursor(imgCursor_), pCursorPos(pCursorPos_) {}

  void DrawCursor(GraphicalInterface<Index> *pGr);
  void SetCursorPos(Point pPos);
  void SetPressed(bool b) { bPressed = b; }

private:
  bool bPressed;
  ImageSequence imgCursor;
  Point pCursorPos;
};

#endif
