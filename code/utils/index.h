#ifndef GUI_INDEX_HEADER_ALREADY_DEFINED
#define GUI_INDEX_HEADER_ALREADY_DEFINED

/*
 * index.h - Reference-counted index and index pool for the Gui namespace.
 * Index notifies an IndexRemover when the last reference is destroyed, so
 * resources (e.g. images, sounds) can be freed. IndexKeeper is a pool of
 * objects indexed by unsigned, with a free list for reuse.
 */

#include <list>
#include <vector>

class Index;

/* Interface for objects that must be notified when an Index is destroyed.
 * Used so that when the last reference to an Index goes away, the backend
 * can free the underlying resource (e.g. delete image/sound handle). */
struct IndexRemover {
  virtual void DeleteIndex(const Index &i) = 0;
};

/* Reference-counted handle to a slot in an IndexKeeper. When the last copy
 * is destroyed, the IndexRemover is notified so the slot can be freed.
 * GetIndex() returns the slot number; operator!() is true when invalid. */
class Index {
  unsigned nI;
  unsigned *pCounter;
  IndexRemover *pRm;

public:
  /* Return the underlying slot index. */
  unsigned GetIndex() const { return nI; }

  /* Default: invalid index (no slot). */
  Index() : nI(-1), pCounter(0), pRm(0) {}

  /* Create an index for slot nI_ that will notify pRm_ when destroyed. */
  Index(unsigned nI_, IndexRemover *pRm_)
      : nI(nI_), pCounter(new unsigned(1)), pRm(pRm_) {}

  Index(const Index &i);
  ~Index();

  Index &operator=(const Index &i);

  /* True if both refer to the same slot. */
  bool operator==(const Index &i) { return nI == i.nI; }

  /* True if this index is invalid (default-constructed). */
  bool operator!() { return pCounter == 0; }
};

/* Pool of T objects indexed by unsigned. GetNewIndex() allocates a slot
 * (reusing a freed one if any) and returns its index. GetElement/FreeElement
 * access or release a slot. Used with Index so that Index(GetNewIndex(), this)
 * gives a reference-counted handle that calls DeleteIndex (and thus
 * FreeElement) when the last copy is destroyed. */
template <class T> class IndexKeeper {
  std::vector<T> vStuff;
  std::list<unsigned> lsFree;

public:
  /* Allocate a new slot (reuse from free list or grow vStuff). */
  unsigned GetNewIndex();
  /* Return the object at the given slot. */
  T &GetElement(unsigned n);
  const T &GetElement(unsigned n) const;
  /* Release the slot and put it on the free list. */
  void FreeElement(unsigned n);
};

template <class T> unsigned IndexKeeper<T>::GetNewIndex() {
  if (lsFree.size()) {
    unsigned n = lsFree.front();
    lsFree.pop_front();
    return n;
  }

  vStuff.push_back(T());

  return unsigned(vStuff.size() - 1);
}

template <class T> T &IndexKeeper<T>::GetElement(unsigned n) {
  return vStuff.at(n);
}

template <class T> const T &IndexKeeper<T>::GetElement(unsigned n) const {
  return vStuff.at(n);
}

template <class T> void IndexKeeper<T>::FreeElement(unsigned n) {
  vStuff.at(n) = T();
  lsFree.push_back(n);
}

#endif // GUI_INDEX_HEADER_ALREADY_DEFINED
