#include "event.h"

void Trigger(Event* pE) {
  if (pE)
    pE->Trigger();
}
