#ifndef GENERAL_HEADER_ALREADY_DEFINED_08_25
#define GENERAL_HEADER_ALREADY_DEFINED_08_25

/*
 * General.h - Gui namespace utilities and helpers.
 * Provides: math min/max/abs; exception hierarchy (MyException, SimpleException);
 * parsing helpers; Timer; stream/file handlers and FilePath; Record/RecordKeeper
 * persistence; Index and IndexKeeper; GuiKeyType enum.
 */

#include <set>
#include <map>
#include <vector>
#include <list>

#include <string>
#include <sstream>

#include "SmartPointer.h"

namespace Gui
{

    typedef const std::string& crefString;

    template<class T>
    inline T Gmin(T a, T b)
    {return a < b ? a : b;}

    template<class T>
    inline T Gmin(T a, T b, T c)
    {return Gmin(a, Gmin(b,c));}

    template<class T>
    inline T Gmax(T a, T b)
    {return a > b ? a : b;}

    template<class T>
    inline T Gmax(T a, T b, T c)
    {return Gmax(a, Gmax(b,c));}

    template<class T>
    inline T Gabs(T a)
    {return a >= 0 ? a : -a;}

    /* Base exception: stores name, class, function stack, and inherited
     * exception info; subclasses override GetErrorMessage() for the message. */
    class MyException
    {
        std::string strExcName;         // name of exception
        std::string strClsName;         // name of class generated exception
        std::string strFnNameTrack;     // list of function names
        std::string strInherited;       // description of inherited exceptions

        std::string GetHeader() const;

    protected:
        void ResetName(crefString istrExcName);
        virtual std::string GetErrorMessage() const=0;

    public:
        MyException(crefString strExcName_, crefString strClsName_, crefString strFnName_);
        virtual ~MyException(){}

        void AddFnName(crefString strFnName);
        void InheritException(const MyException& crefExc);

        std::string GetDescription(bool bDetailed = false) const;
    };

    /* Exception that carries a single problem string as its error message. */
    class SimpleException: public MyException
    {
        std::string strProblem;
    public:
        SimpleException(crefString strClsName_, crefString strFnName_, crefString strProblem_)
            :MyException("SimpleException", strClsName_, strFnName_), strProblem(strProblem_){}

        SimpleException(crefString strProblem_)
            :MyException("<N/A>", "<N/A>", "<N/A>"), strProblem(strProblem_){}
        
        /*virtual*/ std::string GetErrorMessage() const
        {
            return strProblem;
        }
    };


    bool ParsePosition(std::string sToken, std::istream& ifs);
    bool ParseGrabNext(std::string sToken, std::istream& ifs, std::string& sResult);
    bool ParseGrabLine(std::string sToken, std::istream& ifs, std::string& sResult);

    template<class T>
    std::string S(T t)
    {
        std::string s;
        std::ostringstream ostr(s);
        ostr << t;
        return ostr.str();
    }

    /* Period-based timer: Tick()/Check() advance or test, UntilTick()/NextTick()
     * for countdown/advance to next period. */
    struct Timer
    {
        unsigned nTimer, nPeriod;

        Timer(unsigned nPeriod_ = 1):nTimer(0), nPeriod(nPeriod_){}

        unsigned UntilTick()
		{
			return nPeriod - nTimer % nPeriod;
		}
		
		bool Tick()
        {
            if(nPeriod == 0)
                return false;
            return (++nTimer % nPeriod == 0);
        }

        bool Check()
        {
            if(nPeriod == 0)
                return false;
            return (nTimer % nPeriod == 0);
        }

		void NextTick()
		{
			nTimer = nPeriod - 1;
		}
    };

	/* Owns an std::ostream* and deletes it in the destructor; exposes GetStream(). */
	class OutStreamHandler: public SP_Info
	{
	protected:
		std::ostream* pStr;
	public:
		OutStreamHandler(std::ostream* pStr_)
			:pStr(pStr_){}

		~OutStreamHandler()
		{delete pStr;}
		
		std::ostream& GetStream(){return *pStr;}
	};

	/* Owns an std::istream* and deletes it in the destructor; exposes GetStream(). */
	class InStreamHandler: public SP_Info
	{
	protected:
		std::istream* pStr;
	public:
		InStreamHandler(std::istream* pStr_)
			:pStr(pStr_){}

		~InStreamHandler()
		{delete pStr;}
		
		std::istream& GetStream(){return *pStr;}

	};

	/* Abstract interface: open files for read or write, returning smart-pointer
	 * stream handlers. */
	class FileManager: public SP_Info
	{
	public:
		virtual SP<OutStreamHandler> WriteFile(std::string s)=0;
		virtual SP<InStreamHandler> ReadFile(std::string s)=0;
	};

	/* FileManager implementation (concrete implementation for "funny" paths). */
	class FunnyFileManager: public FileManager
	{
	public:
		/*virtual*/ SP<OutStreamHandler> WriteFile(std::string s);
		/*virtual*/ SP<InStreamHandler> ReadFile(std::string s);
	};

	/* FileManager implementation using standard filesystem paths. */
	class StdFileManager: public FileManager
	{
	public:
		/*virtual*/ SP<OutStreamHandler> WriteFile(std::string s);
		/*virtual*/ SP<InStreamHandler> ReadFile(std::string s);
	};

	/* Path representation with slash/convention handling, allowed chars, and
	 * pluggable FileManager for ReadFile/WriteFile. */
	struct FilePath
    {
        std::set<char> stAllowed;
        
        bool bInLinux;
        std::string sPath;

		SP<FileManager> pFm;

        FilePath(bool bInLinux_ = false, std::string sPath_ = "");

        void Slash(std::string& s);
        void Parse(std::string& s);
        std::string GetParse(std::string s);
        void Format(std::string& s);
        std::string GetFormatted(std::string s);

		SP<OutStreamHandler> WriteFile(std::string s);
		SP<InStreamHandler> ReadFile(std::string s);
    };

    std::ostream& operator << (std::ostream& ofs, const FilePath& fp);
    std::istream& operator >> (std::istream& ifs, FilePath& fp);

    /* Abstract base for persistent key-value style records: Read/Write to
     * streams and ReadDef/WriteDef using a default file path. */
    class RecordKeeper: public SP_Info
    {
        std::string sDefFile;
    public:

        RecordKeeper(std::string sDefFile_ = ""):sDefFile(sDefFile_){}
        virtual ~RecordKeeper(){}

        virtual void Read(std::istream& ifs) = 0;
        virtual void Write(std::ostream& ofs) = 0;

        void SetDefFile(std::string sDefFile_){sDefFile = sDefFile_;}

        void ReadDef();
        void WriteDef();
    };

    /* RecordKeeper holding a map<A,B>; serializes/deserializes via Read/Write. */
    template<class A, class B>
    class Record: public RecordKeeper
    {
        std::map<A, B> mpEntries;
    public:
        B& Get(const A& a){return mpEntries[a];}
        const B& Get(const A& a) const{return mpEntries[a];}
        
        void Put(const A& a, const B& b){mpEntries[a] = b;}

        /*virtual*/ void Read(std::istream& ifs);
        /*virtual*/ void Write(std::ostream& ofs);
    };

    /* RecordKeeper that aggregates multiple RecordKeepers and forwards
     * Read/Write to each. */
    class RecordCollection: public RecordKeeper
    {
        std::vector< SP<RecordKeeper> > vRecords;
    public:

        RecordCollection(std::string sDef_ = ""):RecordKeeper(sDef_){}

        void NewRecordKeeper(SP<RecordKeeper> pRec);

        /*virtual*/ void Read(std::istream& ifs);
        /*virtual*/ void Write(std::ostream& ofs);
    };

    void Separate(std::string& strFile, std::string& strFolder);

    class Index;

    /* Interface for objects that need to be notified when an Index is destroyed. */
    struct IndexRemover
    {
        virtual void DeleteIndex(const Index& i)=0;
    };

    /* Reference-counted index that notifies an IndexRemover on destruction. */
    class Index
    {
        unsigned nI;
        unsigned* pCounter;

        IndexRemover* pRm;
    public:
        unsigned GetIndex() const {return nI;}

        Index():nI(-1), pCounter(0), pRm(0){}
        
        Index(unsigned nI_, IndexRemover* pRm_)
            :nI(nI_), pCounter(new unsigned(1)), pRm(pRm_){}

        Index(const Index& i);
        ~Index();
        
        Index& operator = (const Index& i);

        bool operator == (const Index& i){return nI == i.nI;}
        bool operator ! () {return pCounter == 0;}
    };

    /* Pool of T objects indexed by unsigned; reuses freed slots (free list). */
    template<class T>
    class IndexKeeper
    {
        std::vector<T> vStuff;
        std::list<unsigned> lsFree;
    public:
        unsigned GetNewIndex();
        T& GetElement(unsigned n);
        const T& GetElement(unsigned n) const;
        void FreeElement(unsigned n);
    };


    template<class T>
    unsigned IndexKeeper<T>::GetNewIndex()
    {
        if(lsFree.size())
        {
            unsigned n = lsFree.front();
            lsFree.pop_front();
            return n;
        }
        
        vStuff.push_back(T());

        return unsigned(vStuff.size() - 1);
    }

    template<class T>
    T& IndexKeeper<T>::GetElement(unsigned n)
    {
        return vStuff.at(n);
    }

    template<class T>
    const T& IndexKeeper<T>::GetElement(unsigned n) const
    {
        return vStuff.at(n);
    }

    template<class T>
    void IndexKeeper<T>::FreeElement(unsigned n)
    {
        vStuff.at(n) = T();
        lsFree.push_back(n);
    }

    template<class A, class B>
    void Record<A, B>::Read(std::istream& ifs)
    {
        unsigned nSz;
        ifs >> nSz;
        for(unsigned i = 0; i < nSz; ++i)
        {
            A a;
            B b;

            ifs >> a >> b;
            mpEntries[a] = b;
        }
    }

    template<class A, class B>
    void Record<A, B>::Write(std::ostream& ofs)
    {
        ofs << mpEntries.size() << "\n";
        for(typename std::map<A, B>::iterator itr = mpEntries.begin(), etr = mpEntries.end(); itr != etr; ++itr)
            ofs << itr->first << " " << itr->second << "\n";
    }

    /* Keyboard key identifiers: ASCII-like codes plus platform-mapped keys
     * (F-keys, arrows, modifiers, etc.). */
    enum GuiKeyType
    {
        /* Usually consistent across systems (ASCII). */

        GUI_BACKSPACE = 8,
        GUI_TAB = 9,
        GUI_RETURN = 13,
        GUI_ESCAPE = 27,

        // inconsistent keys - need to be mapped manually
        
        GUI_DUMMY = 300, // to start enumeration (hope it works)

        GUI_F1, GUI_F2, GUI_F3, GUI_F4, GUI_F5, GUI_F6, GUI_F7, GUI_F8, GUI_F9, GUI_F10, GUI_F11, GUI_F12,

        GUI_UP, GUI_DOWN, GUI_LEFT, GUI_RIGHT,

        GUI_INSERT, GUI_HOME, GUI_END, GUI_PGUP, GUI_PGDOWN,
        
        GUI_NUMLOCK, GUI_CAPSLOCK, GUI_SCRLOCK,

        GUI_SHIFT, GUI_CTRL,

        GUI_DELETE
    };



}

#endif // GENERAL_HEADER_ALREADY_DEFINED_08_25