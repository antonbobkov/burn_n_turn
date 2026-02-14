#ifndef EVENT_HEADER_08_27_10_06_16
#define EVENT_HEADER_08_27_10_06_16

#include "Preloader.h"
#include "utils/smart_pointer.h"
#include <iostream>

using namespace Gui;

class Event : virtual public SP_Info {
  friend void Trigger(smart_pointer<Event> pE);

  virtual void Trigger() = 0;

public:
  std::string get_class_name() override { return "Event"; }
  virtual ~Event() {}
};

void Trigger(smart_pointer<Event> pE);

class EmptyEvent : public Event {
public:
  std::string get_class_name() override { return "EmptyEvent"; }
  /*virtual*/ void Trigger(){};
};

class WriteEvent : public Event {
  std::string sText;

public:
  std::string get_class_name() override { return "WriteEvent"; }
  WriteEvent(std::string sText_) : sText(sText_) {}
  /*virtual*/ void Trigger() { std::cout << sText << "\n"; };
};

template <class T> class TerminatorEvent : public Event {
  T *pObject;

public:
  std::string get_class_name() override { return "TerminatorEvent"; }
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
  std::string get_class_name() override { return "SwitchEvent"; }
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
  std::string get_class_name() override { return "CpSwitchEvent"; }
  CpSwitchEvent(A &objCurr_, B objNew_) : objCurr(objCurr_), objNew(objNew_) {}
  /*virtual*/ void Trigger() { objCurr = objNew; }
};

template <class A, class B>
CpSwitchEvent<A, B> *NewCpSwitchEvent(A &objCurr_, B objNew_) {
  return new CpSwitchEvent<A, B>(objCurr_, objNew_);
}

struct SequenceOfEvents : public Event {
  std::string get_class_name() override { return "SequenceOfEvents"; }
  std::vector<smart_pointer<Event>> vEv;
  /*virtual*/ void Trigger();
  SequenceOfEvents() {}
  SequenceOfEvents(smart_pointer<Event> pEv1, smart_pointer<Event> pEv2) {
    vEv.push_back(pEv1);
    vEv.push_back(pEv2);
  }
  SequenceOfEvents(smart_pointer<Event> pEv1, smart_pointer<Event> pEv2,
                   smart_pointer<Event> pEv3) {
    vEv.push_back(pEv1);
    vEv.push_back(pEv2);
    vEv.push_back(pEv3);
  }
};

SequenceOfEvents *TwoEvents(smart_pointer<Event> pEv1,
                            smart_pointer<Event> pEv2);

class MakeSoundEvent : public Event {
  SoundInterface<Index> *pSn;
  Index Snd;

public:
  std::string get_class_name() override { return "MakeSoundEvent"; }
  MakeSoundEvent(smart_pointer<SoundInterface<Index>> pSn_, Index Snd_)
      : pSn(pSn_.get()), Snd(Snd_) {}
  MakeSoundEvent(SoundInterface<Index> *pSn_, Index Snd_)
      : pSn(pSn_), Snd(Snd_) {}

  /*virtual*/ void Trigger() { pSn->PlaySound(Snd); }
};

#endif // EVENT_HEADER_08_27_10_06_16