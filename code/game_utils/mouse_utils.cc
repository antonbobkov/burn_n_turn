#include "mouse_utils.h"

#include <list>
#include <vector>

Point TrackballTracker::GetMovement() { return *(++lsMouse.begin()); }

int TrackballTracker::GetLengthSq(Point p) { return p.x * p.x + p.y * p.y; }

TrackballTracker::TrackballTracker() {
  nMaxLength = 3;
  for (int i = 0; i < nMaxLength; i++)
    lsMouse.push_back(Point(0, 0));
  trigFlag = false;
  threshold = 25;
}

void TrackballTracker::Update() {
  Point p = mtr.GetRelMovement();

  lsMouse.push_front(p);
  lsMouse.pop_back();
}

bool TrackballTracker::IsTrigger() {
  std::vector<int> v;
  for (std::list<Point>::iterator itr = lsMouse.begin(); v.size() < 3; ++itr)
    v.push_back(int(fPoint(*itr).Length()));

  return (v[1] > v[0]) && (v[1] > v[2]);
}

fPoint TrackballTracker::GetAvMovement() {
  fPoint p;
  for (std::list<Point>::iterator itr = lsMouse.begin(), etr = lsMouse.end();
       itr != etr; ++itr)
    p += *itr;

  p.x /= lsMouse.size();
  p.y /= lsMouse.size();

  return p;
}

int TrackballTracker::GetDerivative() {
  if (nMaxLength > 1)
    return GetLengthSq(lsMouse.front()) - GetLengthSq(*++lsMouse.begin());
  else
    return 0;
}

void PositionTracker::On() {
  bPressed = true;
  nCounter = 0;
}

int PositionTracker::Off() {
  bPressed = false;
  return nCounter;
}

void PositionTracker::Update() {
  if (bPressed)
    ++nCounter;
}

void PositionTracker::UpdateMouse(Point pMouse_) {
  pMouse = pMouse_;
  pMouse.x /= 2;
  pMouse.y /= 2;
}

void PositionTracker::UpdateLastDownPosition(Point pMouse_) {
  pLastDownPosition = pMouse_;
  pLastDownPosition.x /= 2;
  pLastDownPosition.y /= 2;
}

fPoint PositionTracker::GetFlightDirection(fPoint fDragonPos) {
  return pLastDownPosition - fDragonPos;
}

void MouseCursor::DrawCursor(GraphicalInterface<Index> *pGr) {
  Index img;
  if (bPressed)
    img = imgCursor.vImage.at(1);
  else
    img = imgCursor.vImage.at(0);

  Size sz = pGr->GetImage(img)->GetSize();
  Point p = pCursorPos;

  p.x -= sz.x / 2;
  p.y -= sz.y / 2;

  pGr->DrawImage(p, img, false);
}

void MouseCursor::SetCursorPos(Point pPos) { pCursorPos = pPos; }
