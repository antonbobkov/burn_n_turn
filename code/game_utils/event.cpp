#include "event.h"

void Trigger(SP<Event> pE) {
  if (pE != 0)
    pE->Trigger();
}

void SequenceOfEvents::Trigger() {
  for (size_t i = 0, sz = vEv.size(); i < sz; ++i)
    ::Trigger(vEv[i]);
}

SequenceOfEvents *TwoEvents(SP<Event> pEv1, SP<Event> pEv2) {
  SequenceOfEvents *pRet = new SequenceOfEvents();
  pRet->vEv.push_back(pEv1);
  pRet->vEv.push_back(pEv2);
  return pRet;
}
