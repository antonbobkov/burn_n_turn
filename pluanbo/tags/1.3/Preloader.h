#ifndef PRELOADER_HEADER_02_20_10_09_09
#define PRELOADER_HEADER_02_20_10_09_09

#include <vector>
#include <map>
#include <string>
#include <string>
#include <sstream>

#include "GuiGen.h"

using namespace Gui;


struct ImageSequence: virtual public SP_Info
{
    std::vector<IndexImg> vImage;
    unsigned nActive;

    void Toggle()
    {
        if(nActive == vImage.size() - 1)
            nActive = 0;
        else
            nActive++;
    }

    IndexImg GetImage(){return vImage[nActive];}

    void Add(IndexImg nImg){vImage.push_back(nImg);}

    ImageSequence():nActive(0){}
    ImageSequence(IndexImg img1):nActive(0){Add(img1);}
    ImageSequence(IndexImg img1, IndexImg img2):nActive(0){Add(img1); Add(img2);}
    ImageSequence(IndexImg img1, IndexImg img2, IndexImg img3):nActive(0){Add(img1); Add(img2); Add(img3);}
};

template<class T>
void ForEachImage(ImageSequence& img, T t)
{
    for(unsigned i = 0; i < img.vImage.size(); ++i)
        t(img.vImage[i]);
}

struct ImageFlipper
{
    SP< GraphicalInterface<IndexImg> > pGr;
    ImageFlipper(SP< GraphicalInterface<IndexImg> > pGr_):pGr(pGr_){}
    void operator()(IndexImg& img){img = pGr->FlipImage(img);}
};

struct ImagePainter
{
    typedef std::pair<Color, Color> ColorMap;
    
    SP< GraphicalInterface<IndexImg> > pGr;
    
    std::vector<ColorMap> vTr;

    ImagePainter(SP< GraphicalInterface<IndexImg> > pGr_, Color cFrom, Color cTo):pGr(pGr_)
    {
        vTr.push_back(ColorMap(cFrom, cTo));
    }
    
    ImagePainter(SP< GraphicalInterface<IndexImg> > pGr_, const std::vector<ColorMap>& vTr_):pGr(pGr_), vTr(vTr_){}
    
    void operator()(IndexImg& img)
    {
        img = pGr->CopyImage(img);
        Image* pImg = pGr->GetImage(img);

        for(unsigned i = 0; i < vTr.size(); ++i)
            pImg->ChangeColor(vTr[i].first, vTr[i].second);
    }
};

template<class T>
std::string ToString(T t)
{
    std::string s;
    std::ostringstream ostr(s);
    ostr << t;
    return ostr.str();
}

struct FilePath
{
    bool bInLinux;
    std::string sPath;

    FilePath(bool bInLinux_ = false, std::string sPath_ = ""):bInLinux(bInLinux_), sPath(sPath_){Slash(sPath);}

    void Slash(std::string& s)
    {
        if(!bInLinux)
        {
            for(unsigned i = 0; i < s.length(); ++i)
                if(s[i] == '/')
                    s[i] = '\\';
        }
        else
        {
            for(unsigned i = 0; i < s.length(); ++i)
                if(s[i] == '\\')
                    s[i] = '/';
        }
    }
    
    void Parse(std::string& s)
    {
        Slash(s);
        s = sPath + s;
    }
};

std::ostream& operator << (std::ostream& ofs, const FilePath& fp);
std::istream& operator >> (std::istream& ifs, FilePath& fp);


class PreloaderException: public MyException
{
public:
    PreloaderException(crefString strExcName_, crefString strClsName_, crefString strFnName_)
        :MyException(strExcName_, strClsName_, strFnName_){}
};

template <class Key>
class PreloaderExceptionAccess: public PreloaderException
{
public:
    Key k;
    bool bSeq;

    PreloaderExceptionAccess(crefString strFnName_, Key k_, bool bSeq_)
        :PreloaderException("PreloaderException", "Preloader", strFnName_), k(k_), bSeq(bSeq_){}


    /*virtual*/ std::string GetErrorMessage() const 
    {
        if(bSeq)
            return "No image sequence found with id " + ToString(k);
        else
            return "No image found with id " + ToString(k);
    }
};

class PreloaderExceptionLoad: public PreloaderException
{
public:
    std::string fName;

    PreloaderExceptionLoad(crefString strFnName_, crefString fName_)
        :PreloaderException("PreloaderException", "Preloader", strFnName_), fName(fName_){}


    /*virtual*/ std::string GetErrorMessage() const 
    {
        return "Cannot load image from " + fName;
    }
};


template<class Key>
class Preloader: virtual public SP_Info 
{
    typedef std::map<Key, IndexImg> RegMapType;
    typedef std::map<Key, ImageSequence> SeqMapType;

    RegMapType mpImg;
    SeqMapType mpSeq;

    std::vector<Color> vTr;
    unsigned nScale;

    FilePath fp;

    GraphicalInterface<IndexImg>* pGr;

    ImageSequence LoadSeq(std::string fName);

public:
    Preloader(SP< GraphicalInterface<IndexImg> > pGr_, FilePath fp_ = FilePath())
        :pGr(pGr_.GetRawPointer()), nScale(1), fp(fp_){}

    IndexImg& operator[](Key k);
    ImageSequence& operator()(Key k);
    
    void AddTransparent(Color c);
    void SetScale(unsigned nScale_);

    void ApplyTransparency(IndexImg pImg);

    void AddImage(IndexImg pImg, Key k);
    void AddSequence(ImageSequence pImg, Key k);

    void Load  (std::string fName, Key k);
    void LoadT (std::string fName, Key k, Color c = Color(0,0,0,0));
    void LoadS (std::string fName, Key k, unsigned nScale = 0);
    void LoadTS(std::string fName, Key k, Color c = Color(0,0,0,0), unsigned nScale = 0);

    void LoadSeq  (std::string fName, Key k);
    void LoadSeqT (std::string fName, Key k, Color c = Color(0,0,0,0));
    void LoadSeqS (std::string fName, Key k, unsigned nScale = 0);
    void LoadSeqTS(std::string fName, Key k, Color c = Color(0,0,0,0), unsigned nScale_ = 0);

};

typedef Preloader<std::string> MyPreloader;

template<class Key>
void Preloader<Key>::AddTransparent(Color c){vTr.push_back(c);}

template<class Key>
void Preloader<Key>::SetScale(unsigned nScale_){nScale = nScale_;}

template<class Key>
void Preloader<Key>::ApplyTransparency(IndexImg pImg)
{
    Image* img = pGr->GetImage(pImg);
    for(size_t n = 0, sz = vTr.size(); n < sz; ++n)
        img->ChangeColor(vTr[n], Color(0,0,0,0));
}

template<class Key>
IndexImg& Preloader<Key>::operator[](Key k)
{
    typename RegMapType::iterator itr = mpImg.find(k);
    if(itr == mpImg.end())
        throw PreloaderExceptionAccess<Key>("[]", k, false);
    return itr->second;
}

template<class Key>
ImageSequence& Preloader<Key>::operator()(Key k)
{
    typename SeqMapType::iterator itr = mpSeq.find(k);
    if(itr == mpSeq.end())
        throw PreloaderExceptionAccess<Key>("()", k, true);
    return itr->second;
}

template<class Key>
void Preloader<Key>::AddImage(IndexImg pImg, Key k)
{
    mpImg[k] = pImg;
}

template<class Key>
void Preloader<Key>::AddSequence(ImageSequence pImg, Key k)
{
    mpSeq[k] = pImg;
}





template<class Key>
void Preloader<Key>::Load(std::string fName, Key k)
{
    fp.Parse(fName);
    try
    {
        IndexImg pImg = pGr->LoadImage(fName);
        mpImg[k] = pImg;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("Load", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("Load");
        throw;
    }
}

template<class Key>
void Preloader<Key>::LoadT(std::string fName, Key k, Color c)
{
    fp.Parse(fName);
    try
    {
        IndexImg pImg = pGr->LoadImage(fName);
        
        if(c == Color(0,0,0,0))
            ApplyTransparency(pImg);
        else
            pGr->GetImage(pImg)->ChangeColor(c, Color(0,0,0,0));

        mpImg[k] = pImg;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadT", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadT");
        throw;
    }
}
template<class Key>
void Preloader<Key>::LoadS(std::string fName, Key k, unsigned nScale_)
{
    fp.Parse(fName);
    try
    {
        IndexImg pImg = pGr->LoadImage(fName);
        
        if(nScale_ == 0)
            pImg = pGr->ScaleImage(pImg, nScale);
        else
            pImg = pGr->ScaleImage(pImg, nScale_);

        mpImg[k] = pImg;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadS", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadS");
        throw;
    }
}

template<class Key>
void Preloader<Key>::LoadTS(std::string fName, Key k, Color c, unsigned nScale_)
{
    fp.Parse(fName);
    try
    {
        IndexImg pImg = pGr->LoadImage(fName);
        
        if(c == Color(0,0,0,0))
            ApplyTransparency(pImg);
        else
            pGr->GetImage(pImg)->ChangeColor(c, Color(0,0,0,0));

        if(nScale_ == 0)
            pImg = pGr->ScaleImage(pImg, nScale);
        else
            pImg = pGr->ScaleImage(pImg, nScale_);

        mpImg[k] = pImg;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadTS", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadTS");
        throw;
    }
}

inline void Separate(std::string& strFile, std::string& strFolder)
{
    std::string strRet;
    strFolder = "";
    for(unsigned i = 0; i < strFile.size(); ++i)
    {
        strRet += strFile[i];
        if(strFile[i] == '\\' || strFile[i] == '/')
        {
            strFolder += strRet;
            strRet = "";
        }
    }
}

template<class Key>
ImageSequence Preloader<Key>::LoadSeq(std::string fName)
{
    fp.Parse(fName);
    ImageSequence imgSeq;

    std::ifstream ifs((fName).c_str());
    std::string strFolder;
    Separate(fName, strFolder);
    
    while(true)
    {
        std::string s;
        ifs >> s;
        if(ifs.fail())
            break;
        imgSeq.Add(pGr->LoadImage(strFolder + s));
    }

    if(imgSeq.vImage.empty())
        throw PreloaderExceptionLoad("LoadSeq", fName);

    return imgSeq;
}

template<class Key>
void Preloader<Key>::LoadSeq(std::string fName, Key k)
{
    try
    {
        ImageSequence imgSeq = LoadSeq(fName);
        mpSeq[k] = imgSeq;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadSeq", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadSeq");
        throw;
    }
}

template<class Key>
void Preloader<Key>::LoadSeqT(std::string fName, Key k, Color c)
{
    try
    {
        ImageSequence imgSeq = LoadSeq(fName);
        for(unsigned i = 0, sz = imgSeq.vImage.size(); i < sz; ++i)
            if(c == Color(0,0,0,0))
                ApplyTransparency(imgSeq.vImage[i]);
            else
                pGr->GetImage(imgSeq.vImage[i])->ChangeColor(c, Color(0,0,0,0));
        
        mpSeq[k] = imgSeq;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadSeqT", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadSeqT");
        throw;
    }
}

template<class Key>
void Preloader<Key>::LoadSeqS(std::string fName, Key k, unsigned nScale_)
{
    try
    {
        ImageSequence imgSeq = LoadSeq(fName);
        for(unsigned i = 0, sz = imgSeq.vImage.size(); i < sz; ++i)
            if(nScale_ == 0)
                imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale);
            else
                imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale_);
        
        mpSeq[k] = imgSeq;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadSeqS", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadSeqS");
        throw;
    }
}

template<class Key>
void Preloader<Key>::LoadSeqTS(std::string fName, Key k, Color c, unsigned nScale_)
{
    try
    {
        ImageSequence imgSeq = LoadSeq(fName);
        for(size_t i = 0, sz = imgSeq.vImage.size(); i < sz; ++i)
        {
            if(c == Color(0,0,0,0))
                ApplyTransparency(imgSeq.vImage[i]);
            else
                pGr->GetImage(imgSeq.vImage[i])->ChangeColor(c, Color(0,0,0,0));
            if(nScale_ == 0)
                imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale);
            else
                imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale_);
        }
        
        mpSeq[k] = imgSeq;
    }
    catch(GraphicalInterfaceException& exGr)
    {
        PreloaderExceptionLoad ex("LoadSeqTS", fName);
        ex.InheritException(exGr);
        throw ex;
    }
    catch(PreloaderException& exPre)
    {
        exPre.AddFnName("LoadSeqTS");
        throw;
    }
}

#endif // PRELOADER_HEADER_02_20_10_09_09
