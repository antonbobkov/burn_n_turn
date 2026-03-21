#ifndef EVENT_HEADER_08_27_10_06_16
#define EVENT_HEADER_08_27_10_06_16

#include "Preloader.h"

class Event {
  friend void Trigger(Event* pE);

  virtual void Trigger() = 0;

public:
  virtual ~Event() {}
};

void Trigger(Event* pE);

template <class A, class B> class SwitchEvent : public Event {
  A &objCurr;
  B &objNew;

public:
  SwitchEvent(A &objCurr_, B &objNew_) : objCurr(objCurr_), objNew(objNew_) {}
  void Trigger() override { objCurr = objNew; }
};

#endif // EVENT_HEADER_08_27_10_06_16
