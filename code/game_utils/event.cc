#include "event.h"
#include "utils/smart_pointer.h"

void Trigger(smart_pointer<Event> pE) {
  if (!pE.is_null())
    pE->Trigger();
}

void Trigger(Event* pE) {
  if (pE)
    pE->Trigger();
}

void SequenceOfEvents::Trigger() {
  for (size_t i = 0, sz = vEv.size(); i < sz; ++i)
    ::Trigger(vEv[i]);
}
