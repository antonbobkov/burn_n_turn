#ifndef SMART_POINTER_H
#define SMART_POINTER_H

/*
 * smart_pointer.h - Reference-counted and "permanent" smart pointers.
 * smart_pointer: reference-counted pointer (increments SP_Info counter).
 * SSP/ASSP: non-owning "permanent" pointers that keep pointee alive; SSP_Base
 * tracks them. Inspector and CleanIslandSeeded support cycle/island cleanup.
 */

#include <list>
#include <set>
#include <string>

class SP_Info;

template <class T> class smart_pointer;

class SSP_Base;
template <class T> class SSP;

struct Inspector;

typedef std::set<SSP_Base *> SmartSet;
typedef SmartSet::iterator SmartSetIter;

inline void CHECK_DELETION(SP_Info *);
inline void DELETE_REGULAR_POINTER(SP_Info *);
inline void DELETE_PERMANENT_POINTER(SP_Info *, SSP_Base *);

extern int nGlobalSuperMegaCounter;

/* Mix-in base for reference-counted objects: holds ref count, mark, and sets of
 * SSP_Base pointers that reference this object. smart_pointer/SSP use it. */
class SP_Info {
  template <class T> friend class smart_pointer;
  template <class T> friend class SSP;
  friend class SSP_Base;
  friend struct Inspector;

  friend void CHECK_DELETION(SP_Info *);
  friend void DELETE_REGULAR_POINTER(SP_Info *);
  friend void DELETE_PERMANENT_POINTER(SP_Info *, SSP_Base *);

  unsigned _SP_INFO_COUNTER;

  int _SP_INFO_MARK;

  SmartSet _SP_INFO_POINT_TO_ME;
  SmartSet _SP_INFO_POINT_FROM_ME;

public:
  std::string _SP_INFO_STRING;

  SP_Info(const SP_Info &inf) : _SP_INFO_COUNTER(0), _SP_INFO_MARK(0) {
    ++nGlobalSuperMegaCounter;
  }
  SP_Info() : _SP_INFO_COUNTER(0), _SP_INFO_MARK(0) {
    ++nGlobalSuperMegaCounter;
  }
  SP_Info(std::string sStr_)
      : _SP_INFO_STRING(sStr_), _SP_INFO_COUNTER(0), _SP_INFO_MARK(0) {
    ++nGlobalSuperMegaCounter;
  }

  virtual ~SP_Info() { --nGlobalSuperMegaCounter; }
};

inline void CHECK_DELETION(SP_Info *pThis) {
  if (pThis->_SP_INFO_COUNTER == 0 && pThis->_SP_INFO_POINT_TO_ME.empty())
    delete pThis;
}

inline void DELETE_REGULAR_POINTER(SP_Info *pThis) {
  --(pThis->_SP_INFO_COUNTER);
  CHECK_DELETION(pThis);
}

inline void DELETE_PERMANENT_POINTER(SP_Info *pThis, SSP_Base *pPermPnt) {
  pThis->_SP_INFO_POINT_TO_ME.erase(pPermPnt);
  CHECK_DELETION(pThis);
}

/* Reference-counted smart pointer: copies increment the pointee's SP_Info
 * counter; destruction decrements it and deletes when count and SSP set are 0.
 */
template <class T> class smart_pointer {
  template <class N> friend class SSP;
  template <class N> friend class smart_pointer;
  template <class N> friend smart_pointer<N> make_smart(N *pPointTo_);

  T *pPointTo;
  SP_Info *pPointToSPInfo;

  void CONSTRUCT(T *pPointTo_, SP_Info *pInfo_) {
    pPointTo = pPointTo_;
    pPointToSPInfo = pInfo_;

    if (pPointToSPInfo)
      ++pPointToSPInfo->_SP_INFO_COUNTER;
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

  template <class N> smart_pointer(const SSP<N> &pPrmPnt);

  smart_pointer<T> &operator=(const smart_pointer<T> &pSmPnt) {
    return ASSIGN_TO(pSmPnt.pPointTo, pSmPnt.pPointToSPInfo);
  }

  template <class N>
  smart_pointer<T> &operator=(const smart_pointer<N> &pSmPnt) {
    return ASSIGN_TO(pSmPnt.pPointTo, pSmPnt.pPointToSPInfo);
  }

  template <class N> smart_pointer<T> &operator=(const SSP<N> &pPrmPnt);

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

  bool is_null() const { return pPointTo == 0; }
};

template <class T> smart_pointer<T> make_smart(T *pPointTo_) {
  return smart_pointer<T>(pPointTo_, static_cast<SP_Info *>(pPointTo_));
}

/* Base for "permanent" (non-owning) smart pointers: registered in the pointee's
 * SP_Info so the pointee stays alive while any SSP points to it. */
class SSP_Base {
  friend struct Inspector;

  SP_Info *pHost;

  virtual void CleanNullify() = 0;
  virtual SP_Info *GetPointeeHead() const = 0;

  SSP_Base(const SSP_Base &) {}

protected:
  SP_Info *GetHost() const { return pHost; }

public:
  SSP_Base(SP_Info *pHost_) : pHost(pHost_) {
    pHost->_SP_INFO_POINT_FROM_ME.insert(this);
  }

  virtual ~SSP_Base() { pHost->_SP_INFO_POINT_FROM_ME.erase(this); }
};

/* Non-owning smart pointer that keeps the pointee alive via SP_Info's
 * POINT_TO_ME set. Does not participate in ref count; pointee deleted when
 * ref count is 0 and no SSPs point to it. */
template <class T> class SSP : public SSP_Base {
  template <class N> friend class smart_pointer;
  template <class N> friend class SSP;

  T *pPointTo;

  /*virtual*/ void CleanNullify() { pPointTo = 0; }
  /*virtual*/ SP_Info *GetPointeeHead() const { return pPointTo; }

  void CONSTRUCT(T *pPointTo_) {
    pPointTo = pPointTo_;

    if (pPointTo)
      pPointTo->_SP_INFO_POINT_TO_ME.insert(this);
  }

  SSP<T> &ASSIGN_TO(T *pPointTo_) {
    if (pPointTo)
      DELETE_PERMANENT_POINTER(pPointTo, this);

    pPointTo = pPointTo_;

    if (pPointTo)
      pPointTo->_SP_INFO_POINT_TO_ME.insert(this);

    return *this;
  }

protected:
  SSP(const SSP<T> &pPermPnt) : SSP_Base(pPermPnt.GetHost()) {
    CONSTRUCT(pPermPnt.pPointTo);
  }

public:
  //    SSP(SP_Info* pHost_): SSP_Base(pHost_), pPointTo(0){}

  SSP(SP_Info *pHost_, T *pPointTo_) : SSP_Base(pHost_) {
    CONSTRUCT(pPointTo_);
  }

  SSP(SP_Info *pHost_, const SSP<T> &pPermPnt) : SSP_Base(pHost_) {
    CONSTRUCT(pPermPnt.pPointTo);
  }

  template <class N>
  SSP(SP_Info *pHost_, const SSP<N> &pPermPnt) : SSP_Base(pHost_) {
    CONSTRUCT(pPermPnt.pPointTo);
  }

  template <class N>
  SSP(SP_Info *pHost_, const smart_pointer<N> &pSmrtPnt) : SSP_Base(pHost_) {
    CONSTRUCT(pSmrtPnt.pPointTo);
  }

  SSP<T> &operator=(T *pPointTo_) { return ASSIGN_TO(pPointTo_); }
  SSP<T> &operator=(const SSP<T> &pPermPnt) {
    return ASSIGN_TO(pPermPnt.pPointTo);
  }

  template <class N> SSP<T> &operator=(const SSP<N> &pPermPnt) {
    return ASSIGN_TO(pPermPnt.pPointTo);
  }

  template <class N> SSP<T> &operator=(const smart_pointer<N> &pSmrtPnt) {
    return ASSIGN_TO(pSmrtPnt.pPointTo);
  }

  ~SSP() {
    if (pPointTo)
      DELETE_PERMANENT_POINTER(pPointTo, this);
  }

  T *operator->() { return pPointTo; }
  const T *operator->() const { return pPointTo; }

  T &operator*() { return *pPointTo; }
  const T &operator*() const { return *pPointTo; }

  bool operator==(const smart_pointer<T> &pSmPnt) const {
    return pPointTo == pSmPnt.pPointTo;
  }

  bool operator!=(const smart_pointer<T> &pSmPnt) const {
    return pPointTo != pSmPnt.pPointTo;
  }

  T *get() { return pPointTo; }

  bool is_null() const { return pPointTo == 0; }
};

template <class T>
template <class N>
smart_pointer<T>::smart_pointer(const SSP<N> &pPrmPnt) {
  CONSTRUCT(pPrmPnt.pPointTo, static_cast<SP_Info *>(pPrmPnt.pPointTo));
}

template <class T>
template <class N>
smart_pointer<T> &smart_pointer<T>::operator=(const SSP<N> &pPrmPnt) {
  return ASSIGN_TO(pPrmPnt.pPointTo, static_cast<SP_Info *>(pPrmPnt.pPointTo));
}

/* Copyable SSP for use in containers (e.g. arrays); otherwise same as SSP. */
template <class T> class ASSP : public SSP<T> {
  template <class N> friend class ASSP;

public:
  template <class N> ASSP(const ASSP<N> &pAsp) : SSP<T>(pAsp.GetHost(), pAsp) {}

  template <class N> ASSP(SP_Info *pHost, N n) : SSP<T>(pHost, n) {}

  template <class N> ASSP<T> &operator=(const N &n) {
    SSP<T> *pThis = this;
    *pThis = n;

    return *this;
  }
};

/* Friend accessor for SP_Info/SSP_Base internals; used by CleanIslandSeeded
 * to traverse and break cycles and delete unreachable objects. */
struct Inspector {
  static int &GetMark(SP_Info *pHd) { return pHd->_SP_INFO_MARK; }
  static unsigned &GetCounter(SP_Info *pHd) { return pHd->_SP_INFO_COUNTER; }
  static SmartSet &GetToSet(SP_Info *pHd) { return pHd->_SP_INFO_POINT_TO_ME; }
  static SmartSet &GetFromSet(SP_Info *pHd) {
    return pHd->_SP_INFO_POINT_FROM_ME;
  }
  static void DeleteCheck(SP_Info *pHd) { CHECK_DELETION(pHd); }

  static SP_Info *GetHost(SSP_Base *pSmPnt) { return pSmPnt->GetHost(); }
  static SP_Info *GetPointee(SSP_Base *pSmPnt) {
    return pSmPnt->GetPointeeHead();
  }
  static void Nullify(SSP_Base *pSmPnt) { return pSmPnt->CleanNullify(); }
};

/* Marks reachable SP_Info nodes from [itrBegin, itrEnd], then deletes
 * unreachable nodes and nullifies SSPs pointing to them (breaks islands). */
template <class Iter> void CleanIslandSeeded(Iter itrBegin, Iter itrEnd) {
  std::list<SP_Info *> lstSeeders;
  std::list<SP_Info *> lstToCheck;
  std::list<SP_Info *> lstAll;
  std::list<SP_Info *> lstToDelete;

  for (; itrBegin != itrEnd; ++itrBegin) {
    Inspector::GetMark(*itrBegin) = 1;

    if (Inspector::GetCounter(*itrBegin) != 0)
      lstSeeders.push_back(*itrBegin);

    lstToCheck.push_back(*itrBegin);
    lstAll.push_back(*itrBegin);
  }

  while (!lstToCheck.empty()) {
    for (int i = 0; i < 2; ++i) {
      SmartSet *pSt = 0;

      if (i == 0)
        pSt = &(Inspector::GetToSet(lstToCheck.front()));
      else
        pSt = &(Inspector::GetFromSet(lstToCheck.front()));

      SmartSet &st = *pSt;

      for (SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr) {
        SP_Info *pHd = 0;

        if (i == 0)
          pHd = Inspector::GetHost(*itr);
        else
          pHd = Inspector::GetPointee(*itr);

        if (pHd == 0)
          continue;

        int &_SP_INFO_MARK = Inspector::GetMark(pHd);

        if (_SP_INFO_MARK != 0)
          continue;

        _SP_INFO_MARK = 1;

        if (Inspector::GetCounter(pHd) != 0)
          lstSeeders.push_back(pHd);

        lstToCheck.push_back(pHd);
        lstAll.push_back(pHd);
      }
    }

    lstToCheck.pop_front();
  }

  for (std::list<SP_Info *>::iterator itr = lstSeeders.begin(),
                                      etr = lstSeeders.end();
       itr != etr; ++itr)
    Inspector::GetMark(*itr) = 2;

  while (!lstSeeders.empty()) {
    SmartSet &st = Inspector::GetFromSet(lstSeeders.front());
    lstSeeders.pop_front();

    for (SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr) {
      SP_Info *pHd = Inspector::GetPointee(*itr);

      if (pHd == 0)
        continue;

      int &_SP_INFO_MARK = Inspector::GetMark(pHd);

      if (_SP_INFO_MARK == 2)
        continue;

      _SP_INFO_MARK = 2;
      lstSeeders.push_back(pHd);
    }
  }

  for (std::list<SP_Info *>::iterator itr = lstAll.begin(), etr = lstAll.end();
       itr != etr; ++itr)
    if (Inspector::GetMark(*itr) == 1)
      lstToDelete.push_back(*itr);

  for (std::list<SP_Info *>::iterator itr = lstAll.begin(), etr = lstAll.end();
       itr != etr; ++itr)
    Inspector::GetMark(*itr) = 0;

  for (std::list<SP_Info *>::iterator itr = lstToDelete.begin(),
                                      etr = lstToDelete.end();
       itr != etr; ++itr) {
    SmartSet &st = Inspector::GetToSet(*itr);

    for (SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr) {
      Inspector::Nullify(*itr);
    }

    st.clear();
  }

  for (std::list<SP_Info *>::iterator itr = lstToDelete.begin(),
                                      etr = lstToDelete.end();
       itr != etr; ++itr) {
    Inspector::DeleteCheck(*itr);
  }
}

/* Clean island starting from single node pHd. */
void CleanIslandSeeded(SP_Info *pHd);

/* Clean island (convenience overload). */
void CleanIsland(SP_Info *pHd);

#endif // SMART_POINTER_H
