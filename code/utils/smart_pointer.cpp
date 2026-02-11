#include "smart_pointer.h"

#include <vector>

int nGlobalSuperMegaCounter = 0;

void CleanIslandSeeded(SP_Info *pHd) {
  std::vector<SP_Info *> v;
  v.push_back(pHd);

  CleanIslandSeeded(v.begin(), v.end());
}

void CleanIsland(SP_Info *pHd) {
  std::list<SP_Info *> lstToCheck;
  std::list<SP_Info *> lstToAll;

  lstToCheck.push_back(pHd);
  lstToAll.push_back(pHd);

  Inspector::GetMark(pHd) = 1;

  bool bUseful = false;

  while (!lstToCheck.empty()) {
    SmartSet &st = Inspector::GetToSet(lstToCheck.front());
    lstToCheck.pop_front();

    for (SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr) {
      SP_Info *pHd = Inspector::GetHost(*itr);

      if (Inspector::GetCounter(pHd) != 0) {
        bUseful = true;
        lstToCheck.clear();
        break;
      }

      int &_SP_INFO_MARK = Inspector::GetMark(pHd);

      if (_SP_INFO_MARK == 0) {
        _SP_INFO_MARK = 1;

        lstToCheck.push_back(pHd);
        lstToAll.push_back(pHd);
      }
    }
  }

  if (bUseful) {
    for (std::list<SP_Info *>::iterator itr = lstToAll.begin(),
                                        etr = lstToAll.end();
         itr != etr; ++itr)
      Inspector::GetMark(*itr) = 0;
    return;
  }

  for (std::list<SP_Info *>::iterator itr = lstToAll.begin(),
                                      etr = lstToAll.end();
       itr != etr; ++itr) {
    SmartSet &st = Inspector::GetToSet(*itr);

    for (SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr) {
      Inspector::Nullify(*itr);
    }

    st.clear();
  }

  for (std::list<SP_Info *>::iterator itr = lstToAll.begin(),
                                      etr = lstToAll.end();
       itr != etr; ++itr) {
    Inspector::DeleteCheck(*itr);
  }
}
