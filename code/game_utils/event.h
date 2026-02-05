#ifndef EVENT_HEADER_08_27_10_06_16
#define EVENT_HEADER_08_27_10_06_16

#include "Preloader.h"
#include <iostream>

using namespace Gui;

class Event : virtual public SP_Info {
  friend void Trigger(SP<Event> pE);

  virtual void Trigger() = 0;

public:
  virtual ~Event() {}
};

void Trigger(SP<Event> pE);

class EmptyEvent : public Event {
public:
  /*virtual*/ void Trigger(){};
};

class WriteEvent : public Event {
  std::string sText;

public:
  WriteEvent(std::string sText_) : sText(sText_) {}
  /*virtual*/ void Trigger() { std::cout << sText << "\n"; };
};

template <class T> class TerminatorEvent : public Event {
  T *pObject;

public:
  TerminatorEvent(T *pObject_) : pObject(pObject_) {}
  /*virtual*/ void Trigger() { pObject->Terminate(); }
};

template <class T> TerminatorEvent<T> *NewTerminatorEvent(T *pObject_) {
  return new TerminatorEvent<T>(pObject_);
}

template <class A, class B> class SwitchEvent : public Event {
  A &objCurr;
  B &objNew;

public:
  SwitchEvent(A &objCurr_, B &objNew_) : objCurr(objCurr_), objNew(objNew_) {}
  /*virtual*/ void Trigger() { objCurr = objNew; }
};

template <class A, class B>
SwitchEvent<A, B> *NewSwitchEvent(A &objCurr_, B &objNew_) {
  return new SwitchEvent<A, B>(objCurr_, objNew_);
}

template <class A, class B> class CpSwitchEvent : public Event {
  A &objCurr;
  B objNew;

public:
  CpSwitchEvent(A &objCurr_, B objNew_) : objCurr(objCurr_), objNew(objNew_) {}
  /*virtual*/ void Trigger() { objCurr = objNew; }
};

template <class A, class B>
CpSwitchEvent<A, B> *NewCpSwitchEvent(A &objCurr_, B objNew_) {
  return new CpSwitchEvent<A, B>(objCurr_, objNew_);
}

struct SequenceOfEvents : public Event {
  std::vector<SP<Event>> vEv;
  /*virtual*/ void Trigger();
  SequenceOfEvents() {}
  SequenceOfEvents(SP<Event> pEv1, SP<Event> pEv2) {
    vEv.push_back(pEv1);
    vEv.push_back(pEv2);
  }
  SequenceOfEvents(SP<Event> pEv1, SP<Event> pEv2, SP<Event> pEv3) {
    vEv.push_back(pEv1);
    vEv.push_back(pEv2);
    vEv.push_back(pEv3);
  }
};

SequenceOfEvents *TwoEvents(SP<Event> pEv1, SP<Event> pEv2);

class MakeSoundEvent : public Event {
  SoundInterface<Index> *pSn;
  Index Snd;

public:
  MakeSoundEvent(SP<SoundInterface<Index>> pSn_, Index Snd_)
      : pSn(pSn_.GetRawPointer()), Snd(Snd_) {}
  MakeSoundEvent(SoundInterface<Index> *pSn_, Index Snd_)
      : pSn(pSn_), Snd(Snd_) {}

  /*virtual*/ void Trigger() { pSn->PlaySound(Snd); }
};

#endif // EVENT_HEADER_08_27_10_06_16