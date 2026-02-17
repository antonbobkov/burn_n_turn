#ifndef SMART_POINTER_H
#define SMART_POINTER_H

/*
 * smart_pointer.h - Reference-counted smart pointers.
 * smart_pointer: reference-counted pointer (increments SP_Info counter).
 */

#include <map>
#include <string>

class SP_Info;

template <class T> class smart_pointer;

inline void CHECK_DELETION(SP_Info *);
inline void DELETE_REGULAR_POINTER(SP_Info *);

extern int nGlobalSuperMegaCounter;

extern std::map<std::string, int> g_smart_pointer_count;

/* Mix-in base for reference-counted objects: holds ref count. smart_pointer
 * uses it. */
class SP_Info {
  template <class T> friend class smart_pointer;

  friend void CHECK_DELETION(SP_Info *);
  friend void DELETE_REGULAR_POINTER(SP_Info *);

  unsigned _SP_INFO_COUNTER;

public:
  std::string _SP_INFO_STRING;

  SP_Info(const SP_Info &inf) : _SP_INFO_COUNTER(0) {
    ++nGlobalSuperMegaCounter;
  }
  SP_Info() : _SP_INFO_COUNTER(0) { ++nGlobalSuperMegaCounter; }
  SP_Info(std::string sStr_) : _SP_INFO_STRING(sStr_), _SP_INFO_COUNTER(0) {
    ++nGlobalSuperMegaCounter;
  }

  virtual std::string get_class_name() { return ""; }

  unsigned get_counter() const { return _SP_INFO_COUNTER; }

  virtual ~SP_Info() { --nGlobalSuperMegaCounter; }
};

inline void CHECK_DELETION(SP_Info *pThis) {
  if (pThis->_SP_INFO_COUNTER == 0)
    delete pThis;
}

inline void DELETE_REGULAR_POINTER(SP_Info *pThis) {
  --g_smart_pointer_count[pThis->get_class_name()];
  --(pThis->_SP_INFO_COUNTER);
  CHECK_DELETION(pThis);
}

/* Reference-counted smart pointer: copies increment the pointee's SP_Info
 * counter; destruction decrements it and deletes when count is 0. */
template <class T> class smart_pointer {
  template <class N> friend class smart_pointer;
  template <class N> friend smart_pointer<N> make_smart(N *pPointTo_);

  T *pPointTo;
  SP_Info *pPointToSPInfo;

  void CONSTRUCT(T *pPointTo_, SP_Info *pInfo_) {
    pPointTo = pPointTo_;
    pPointToSPInfo = pInfo_;

    if (pPointToSPInfo) {
      ++g_smart_pointer_count[pPointToSPInfo->get_class_name()];
      ++pPointToSPInfo->_SP_INFO_COUNTER;
    }
  }

  smart_pointer<T> &ASSIGN_TO(T *pPointTo_, SP_Info *pInfo_) {
    if (pPointToSPInfo == pInfo_)
      return *this;

    if (pPointToSPInfo)
      DELETE_REGULAR_POINTER(pPointToSPInfo);

    CONSTRUCT(pPointTo_, pInfo_);

    return *this;
  }

  /* Only make_smart calls this */
  smart_pointer(T *pPointTo_, SP_Info *pInfo_) { CONSTRUCT(pPointTo_, pInfo_); }

public:
  smart_pointer() : pPointTo(0), pPointToSPInfo(0) {}

  smart_pointer(const smart_pointer<T> &pSmPnt) {
    CONSTRUCT(pSmPnt.pPointTo, pSmPnt.pPointToSPInfo);
  }

  template <class N> smart_pointer(const smart_pointer<N> &pSmPnt) {
    CONSTRUCT(pSmPnt.pPointTo, pSmPnt.pPointToSPInfo);
  }

  smart_pointer<T> &operator=(const smart_pointer<T> &pSmPnt) {
    return ASSIGN_TO(pSmPnt.pPointTo, pSmPnt.pPointToSPInfo);
  }

  template <class N>
  smart_pointer<T> &operator=(const smart_pointer<N> &pSmPnt) {
    return ASSIGN_TO(pSmPnt.pPointTo, pSmPnt.pPointToSPInfo);
  }

  ~smart_pointer() {
    if (pPointToSPInfo)
      DELETE_REGULAR_POINTER(pPointToSPInfo);
  }

  T *operator->() const { return pPointTo; }

  T &operator*() const { return *pPointTo; }

  bool operator==(const smart_pointer<T> &pSmPnt) const {
    return pPointTo == pSmPnt.pPointTo;
  }

  bool operator!=(const smart_pointer<T> &pSmPnt) const {
    return pPointTo != pSmPnt.pPointTo;
  }

  T *get() { return pPointTo; }
  T *get() const { return pPointTo; }

  bool is_null() const { return pPointTo == 0; }
};

template <class T> smart_pointer<T> make_smart(T *pPointTo_) {
  return smart_pointer<T>(pPointTo_, static_cast<SP_Info *>(pPointTo_));
}

#endif // SMART_POINTER_H
