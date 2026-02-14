/*
 * index.cpp - Index copy constructor, destructor, and assignment.
 * Reference-counting and notification of IndexRemover when last reference
 * is destroyed.
 */

#include "index.h"

Index::Index(const Index &i) : nI(i.nI), pCounter(0), pRm(i.pRm) {
  if (i.pCounter)
    pCounter = &++*i.pCounter;
}

Index::~Index() {
  if (pCounter && --*pCounter == 0) {
    pRm->DeleteIndex(*this);
    delete pCounter;
  }
}

Index &Index::operator=(const Index &i) {
  if (pCounter && --*pCounter == 0) {
    pRm->DeleteIndex(*this);
    delete pCounter;
  }

  nI = i.nI;

  if (i.pCounter)
    ++*i.pCounter;

  pCounter = i.pCounter;
  pRm = i.pRm;

  return *this;
}
