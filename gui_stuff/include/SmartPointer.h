#ifndef SMART_POINTER_HEADER_02_20_10_11_54
#define SMART_POINTER_HEADER_02_20_10_11_54

#include <string>
#include <set>
#include <list>

class SP_Info;

template<class T> class SP;

class SSP_Base;
template<class T> class SSP;

struct Inspector;

typedef std::set<SSP_Base*> SmartSet;
typedef SmartSet::iterator SmartSetIter;

inline void CHECK_DELETION(SP_Info*);
inline void DELETE_REGULAR_POINTER(SP_Info*);
inline void DELETE_PERMANENT_POINTER(SP_Info*, SSP_Base*);

extern int nGlobalSuperMegaCounter;

class SP_Info
{
    template<class T> friend class SP;
    template<class T> friend class SSP;
    friend class SSP_Base;
    friend struct Inspector;

    friend void CHECK_DELETION(SP_Info*);
    friend void DELETE_REGULAR_POINTER(SP_Info*);
    friend void DELETE_PERMANENT_POINTER(SP_Info*, SSP_Base*);

    unsigned _SP_INFO_COUNTER;

    int _SP_INFO_MARK;
    
    SmartSet _SP_INFO_POINT_TO_ME;
    SmartSet _SP_INFO_POINT_FROM_ME;
    
public:
    std::string _SP_INFO_STRING;

    SP_Info(const SP_Info& inf):_SP_INFO_COUNTER(0), _SP_INFO_MARK(0){++nGlobalSuperMegaCounter;}
    SP_Info():_SP_INFO_COUNTER(0), _SP_INFO_MARK(0){++nGlobalSuperMegaCounter;}
    SP_Info(std::string sStr_):_SP_INFO_STRING(sStr_), _SP_INFO_COUNTER(0), _SP_INFO_MARK(0){++nGlobalSuperMegaCounter;}
    
    virtual ~SP_Info(){--nGlobalSuperMegaCounter;}
};

inline void CHECK_DELETION(SP_Info* pThis)
{
    if(pThis->_SP_INFO_COUNTER == 0 && pThis->_SP_INFO_POINT_TO_ME.empty())
        delete pThis;
}

inline void DELETE_REGULAR_POINTER(SP_Info* pThis)
{
    --(pThis->_SP_INFO_COUNTER);
    CHECK_DELETION(pThis);
}

inline void DELETE_PERMANENT_POINTER(SP_Info* pThis, SSP_Base* pPermPnt)
{
    pThis->_SP_INFO_POINT_TO_ME.erase(pPermPnt);
    CHECK_DELETION(pThis);
}


template<class T>
class SP
{
    template<class N> friend class SSP;
    template<class N> friend class SP;

    T* pPointTo;

    void CONSTRUCT(T* pPointTo_)
    {
        pPointTo = pPointTo_;
        
        if(pPointTo)
            ++pPointTo->_SP_INFO_COUNTER;
    }

    SP<T>& EQUAL(T* pPointTo_)
    {
        if(pPointTo)
            DELETE_REGULAR_POINTER(pPointTo);
        
        pPointTo = pPointTo_;
        
        if(pPointTo)
            ++pPointTo->_SP_INFO_COUNTER;

        return *this;
    }

public:

    SP():pPointTo(0){}
    
    SP(T* pPointTo_)        {CONSTRUCT(pPointTo_);}
    SP(const SP<T>& pSmPnt) {CONSTRUCT(pSmPnt.pPointTo);}
    
    template<class N>
    SP(const SP<N>& pSmPnt) {CONSTRUCT(pSmPnt.pPointTo);}

    template<class N>
    SP(const SSP<N>& pPrmPnt);

    SP<T>& operator = (T* pPointTo_)        {return EQUAL(pPointTo_);}
    SP<T>& operator = (const SP<T>& pSmPnt) {return EQUAL(pSmPnt.pPointTo);}
    
    template<class N>
    SP<T>& operator = (const SP<N>& pSmPnt) {return EQUAL(pSmPnt.pPointTo);}

    template<class N>
    SP& operator = (const SSP<N>& pPrmPnt);

    ~SP()
    {
        if(pPointTo)
            DELETE_REGULAR_POINTER(pPointTo);
    }

    T* operator->() const {return pPointTo;}
    //const T* operator->() const {return pPointTo;}
    
    T& operator*() const {return *pPointTo;}
    //const T& operator*() const{return *pPointTo;}

    bool operator == (const SP<T>& pSmPnt)
    {
        return pPointTo == pSmPnt.pPointTo;
    }
    
    bool operator != (const SP<T>& pSmPnt)
    {
        return pPointTo != pSmPnt.pPointTo;
    }

    T* GetRawPointer()
    {
        return pPointTo;
    }
};

class SSP_Base
{
    friend struct Inspector;

    SP_Info* pHost;

    virtual void CleanNullify()=0;
    virtual SP_Info* GetPointeeHead() const=0;

    SSP_Base(const SSP_Base&){}

protected:
    SP_Info* GetHost() const {return pHost;}

public:

    
    SSP_Base(SP_Info* pHost_):pHost(pHost_)
    {
        pHost->_SP_INFO_POINT_FROM_ME.insert(this);
    }
    
    virtual ~SSP_Base()
    {
        pHost->_SP_INFO_POINT_FROM_ME.erase(this);
    }
};

template<class T>
class SSP: public SSP_Base
{
    template<class N> friend class SP;
    template<class N> friend class SSP;

    T* pPointTo;

    /*virtual*/ void CleanNullify(){pPointTo = 0;}
    /*virtual*/ SP_Info* GetPointeeHead() const {return pPointTo;}

    void CONSTRUCT(T* pPointTo_)
    {
        pPointTo = pPointTo_;

        if(pPointTo)
            pPointTo->_SP_INFO_POINT_TO_ME.insert(this);
    }

    SSP<T>& EQUAL(T* pPointTo_)
    {
        if(pPointTo)
            DELETE_PERMANENT_POINTER(pPointTo, this);
        
        pPointTo = pPointTo_;
        
        if(pPointTo)
            pPointTo->_SP_INFO_POINT_TO_ME.insert(this);

        return *this;
    }

protected:
    SSP(const SSP<T>& pPermPnt)
        :SSP_Base(pPermPnt.GetHost()) {CONSTRUCT(pPermPnt.pPointTo);}

public:

//    SSP(SP_Info* pHost_): SSP_Base(pHost_), pPointTo(0){}
    
    SSP(SP_Info* pHost_, T* pPointTo_)
        :SSP_Base(pHost_) {CONSTRUCT(pPointTo_);}
    
    SSP(SP_Info* pHost_, const SSP<T>& pPermPnt)
        :SSP_Base(pHost_) {CONSTRUCT(pPermPnt.pPointTo);}

    template<class N>
    SSP(SP_Info* pHost_, const SSP<N>& pPermPnt)
        :SSP_Base(pHost_) {CONSTRUCT(pPermPnt.pPointTo);}

    template<class N>
    SSP(SP_Info* pHost_, const SP<N>& pSmrtPnt)
        :SSP_Base(pHost_) {CONSTRUCT(pSmrtPnt.pPointTo);}

    SSP<T>& operator = (T* pPointTo_) {return EQUAL(pPointTo_);}
    SSP<T>& operator = (const SSP<T>& pPermPnt) {return EQUAL(pPermPnt.pPointTo);}
    
    template<class N>
    SSP<T>& operator = (const SSP<N>& pPermPnt) {return EQUAL(pPermPnt.pPointTo);}
    
    template<class N>
    SSP<T>& operator = (const SP<N>& pSmrtPnt) {return EQUAL(pSmrtPnt.pPointTo);}
    
    ~SSP()
    {
        if(pPointTo)
            DELETE_PERMANENT_POINTER(pPointTo, this);
    }

    T* operator->(){return pPointTo;}
    const T* operator->() const {return pPointTo;}
    
    T& operator*() {return *pPointTo;}
    const T& operator*() const{return *pPointTo;}

    bool operator == (const SP<T>& pSmPnt)
    {
        return pPointTo == pSmPnt.pPointTo;
    }
    
    bool operator != (const SP<T>& pSmPnt)
    {
        return pPointTo != pSmPnt.pPointTo;
    }

    T* GetRawPointer(){return pPointTo;}
};

template<class T> template <class N>
SP<T>::SP(const SSP<N>& pPrmPnt){CONSTRUCT(pPrmPnt.pPointTo);}

template<class T> template <class N>
SP<T>& SP<T>::operator = (const SSP<N>& pPrmPnt){return EQUAL(pPrmPnt.pPointTo);}


// SSP that can be copied
// used for storage in arrays
template<class T>
class ASSP: public SSP<T>
{
    template<class N> friend class ASSP;

public:
    template<class N>
    ASSP(const ASSP<N>& pAsp):SSP<T>(pAsp.GetHost(), pAsp){}
    
    template<class N>
    ASSP(SP_Info* pHost, N n):SSP<T>(pHost, n){}

    template<class N>
    ASSP<T>& operator = (const N& n)
    {
        SSP<T>* pThis = this;
        *pThis = n;

        return *this;
    }
};

struct Inspector
{
    static int& GetMark(SP_Info* pHd){return pHd->_SP_INFO_MARK;}
    static unsigned& GetCounter(SP_Info* pHd){return pHd->_SP_INFO_COUNTER;}
    static SmartSet& GetToSet(SP_Info* pHd){return pHd->_SP_INFO_POINT_TO_ME;}
    static SmartSet& GetFromSet(SP_Info* pHd){return pHd->_SP_INFO_POINT_FROM_ME;}
    static void DeleteCheck(SP_Info* pHd){CHECK_DELETION(pHd);}

    static SP_Info* GetHost(SSP_Base* pSmPnt){return pSmPnt->GetHost();}
    static SP_Info* GetPointee(SSP_Base* pSmPnt){return pSmPnt->GetPointeeHead();}
    static void Nullify(SSP_Base* pSmPnt){return pSmPnt->CleanNullify();}
};

template<class Iter>
void CleanIslandSeeded(Iter itrBegin, Iter itrEnd)
{
    std::list<SP_Info*> lstSeeders;
    std::list<SP_Info*> lstToCheck;
    std::list<SP_Info*> lstAll;
    std::list<SP_Info*> lstToDelete;

    for(; itrBegin != itrEnd; ++itrBegin)
    {
        Inspector::GetMark(*itrBegin) = 1;
        
        if(Inspector::GetCounter(*itrBegin) != 0)
            lstSeeders.push_back(*itrBegin);
        
        lstToCheck.push_back(*itrBegin);
        lstAll.push_back(*itrBegin);
    }

    while(!lstToCheck.empty())
    {
        for(int i = 0; i < 2; ++i)
        {
            SmartSet* pSt = 0;

            if(i == 0)
                pSt = &(Inspector::GetToSet(lstToCheck.front()));
            else
                pSt = &(Inspector::GetFromSet(lstToCheck.front()));
            
            SmartSet& st = *pSt;
            
            for(SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr)
            {
                SP_Info* pHd = 0;
                
                if(i == 0)
                    pHd = Inspector::GetHost(*itr);
                else
                    pHd = Inspector::GetPointee(*itr);
                
                if(pHd == 0)
                    continue;
                
                int& _SP_INFO_MARK = Inspector::GetMark(pHd);
                
                if(_SP_INFO_MARK != 0)
                    continue;

                _SP_INFO_MARK = 1;

                if(Inspector::GetCounter(pHd) != 0)
                    lstSeeders.push_back(pHd);
                
                lstToCheck.push_back(pHd);
                lstAll.push_back(pHd);
            }
        }

        lstToCheck.pop_front();
    }

    for(std::list<SP_Info*>::iterator itr = lstSeeders.begin(), etr = lstSeeders.end(); itr != etr; ++itr)
        Inspector::GetMark(*itr) = 2;

    while(!lstSeeders.empty())
    {
        SmartSet& st = Inspector::GetFromSet(lstSeeders.front());
        lstSeeders.pop_front();
        
        for(SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr)
        {
            SP_Info* pHd = Inspector::GetPointee(*itr);
            
            if(pHd == 0)
                continue;
            
            int& _SP_INFO_MARK = Inspector::GetMark(pHd);

            if(_SP_INFO_MARK == 2)
                continue;
            
            _SP_INFO_MARK = 2;
            lstSeeders.push_back(pHd);
        }
    }

    for(std::list<SP_Info*>::iterator itr = lstAll.begin(), etr = lstAll.end(); itr != etr; ++itr)
        if(Inspector::GetMark(*itr) == 1)
            lstToDelete.push_back(*itr);
    
    for(std::list<SP_Info*>::iterator itr = lstAll.begin(), etr = lstAll.end(); itr != etr; ++itr)
        Inspector::GetMark(*itr) = 0;

    for(std::list<SP_Info*>::iterator itr = lstToDelete.begin(), etr = lstToDelete.end(); itr != etr; ++itr)
    {
        SmartSet& st = Inspector::GetToSet(*itr);
        
        for(SmartSetIter itr = st.begin(), etr = st.end(); itr != etr; ++itr)
        {
            Inspector::Nullify(*itr);
        }

        st.clear();
    }

    for(std::list<SP_Info*>::iterator itr = lstToDelete.begin(), etr = lstToDelete.end(); itr != etr; ++itr)
    {
        Inspector::DeleteCheck(*itr);
    }
}

void CleanIslandSeeded(SP_Info* pHd);

void CleanIsland(SP_Info* pHd);

#endif // SMART_POINTER_HEADER_02_20_10_11_54
