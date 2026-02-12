#include "event.h"
#include "smart_pointer.h"

void Trigger(smart_pointer<Event> pE) {
  if (!pE.is_null())
    pE->Trigger();
}

void SequenceOfEvents::Trigger() {
  for (size_t i = 0, sz = vEv.size(); i < sz; ++i)
    ::Trigger(vEv[i]);
}

SequenceOfEvents *TwoEvents(smart_pointer<Event> pEv1,
                            smart_pointer<Event> pEv2) {
  SequenceOfEvents *pRet = new SequenceOfEvents();
  pRet->vEv.push_back(pEv1);
  pRet->vEv.push_back(pEv2);
  return pRet;
}
