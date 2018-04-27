#ifndef PRELOADER_HEADER_02_20_10_09_09
#define PRELOADER_HEADER_02_20_10_09_09

#include <vector>
#include <map>
#include <string>
#include <sstream>

#include "General.h"
#include "GuiGen.h"
#include "SuiGen.h"

namespace Gui
{


    struct SoundSequence
    {
        std::vector<Index> vSounds;
        std::vector<unsigned> vIntervals;
        unsigned nActive;

        bool Toggle()
        {
            if(nActive == vSounds.size() - 1)
            {
                nActive = 0;
                return true;
            }
            else
                nActive++;
            return false;
        }
        
        Index GetSound(){return vSounds[nActive];}
        unsigned GetTime()const{if(vIntervals.empty()) return 1; return vIntervals[nActive];}

        void Add(Index iSnd, unsigned nTime = 1){vSounds.push_back(iSnd); vIntervals.push_back(nTime);}

        SoundSequence():nActive(0){}

    };

    struct ImageSequence: virtual public SP_Info
    {
        std::vector<Index> vImage;
        std::vector<unsigned> vIntervals;
        unsigned nActive;

        Timer t;

        bool Toggle();
        bool ToggleTimed();

        Index GetImage(){return vImage[nActive];}
        unsigned GetTime()const{if(vIntervals.empty()) return 1; return vIntervals[nActive];}
		unsigned GetTotalTime()const
		{
			int nRet = 0;
			for(unsigned i = 0; i < vIntervals.size(); ++i)
				nRet += vIntervals[i];
			if(nRet == 0)
				return 1;
			return nRet;
		}

        void Add(Index nImg, unsigned nTime = 1)
        {vImage.push_back(nImg); vIntervals.push_back(nTime);}

        void INIT(){nActive = 0; t = Timer(0);}
        
        ImageSequence(){INIT();}
        ImageSequence(Index img1){INIT(); Add(img1);}
        ImageSequence(Index img1, Index img2){INIT(); Add(img1); Add(img2);}
        ImageSequence(Index img1, Index img2, Index img3){INIT(); Add(img1); Add(img2); Add(img3);}
    };

    template<class T>
    void ForEachImage(ImageSequence& img, T t)
    {
        for(unsigned i = 0; i < img.vImage.size(); ++i)
            t(img.vImage[i]);
    }

    struct ImageFlipper
    {
        SP< GraphicalInterface<Index> > pGr;
        ImageFlipper(SP< GraphicalInterface<Index> > pGr_):pGr(pGr_){}
        void operator()(Index& img){img = pGr->FlipImage(img);}
    };

    struct ImagePainter
    {
        typedef std::pair<Color, Color> ColorMap;
        
        SP< GraphicalInterface<Index> > pGr;
        
        std::vector<ColorMap> vTr;

        ImagePainter(SP< GraphicalInterface<Index> > pGr_, Color cFrom, Color cTo):pGr(pGr_)
        {
            vTr.push_back(ColorMap(cFrom, cTo));
        }
        
        ImagePainter(SP< GraphicalInterface<Index> > pGr_, const std::vector<ColorMap>& vTr_):pGr(pGr_), vTr(vTr_){}
        
        void operator()(Index& img)
        {
            img = pGr->CopyImage(img);
            Image* pImg = pGr->GetImage(img);

            for(unsigned i = 0; i < vTr.size(); ++i)
                pImg->ChangeColor(vTr[i].first, vTr[i].second);
        }
    };

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
        bool bSnd;

        PreloaderExceptionAccess(crefString strFnName_, Key k_, bool bSeq_, bool bSnd_ = false)
            :PreloaderException("PreloaderException", "Preloader", strFnName_), k(k_), bSeq(bSeq_), bSnd(bSnd_){}


        /*virtual*/ std::string GetErrorMessage() const 
        {
            if(bSeq && !bSnd)
                return "No image sequence found with id " + S(k);
            else if(!bSeq && !bSnd)
                return "No image found with id " + S(k);
            else if(bSeq && bSnd)
                return "No sound sequence found with id " + S(k);
            else
                return "No sound found with id " + S(k);
        }
    };

    class PreloaderExceptionLoad: public PreloaderException
    {
    public:
        std::string fName;
        bool bSound;

        PreloaderExceptionLoad(crefString strFnName_, crefString fName_, bool bSound_ = false)
            :PreloaderException("PreloaderException", "Preloader", strFnName_), fName(fName_), bSound(bSound_){}


        /*virtual*/ std::string GetErrorMessage() const 
        {
            if(!bSound)
                return "Cannot load image from " + fName;
            else
                return "Cannot load sound from " + fName;
        }
    };


    template<class Key>
    class Preloader: virtual public SP_Info 
    {
        typedef std::map<Key, Index> RegMapType;
        typedef std::map<Key, ImageSequence> SeqMapType;
        typedef std::map<Key, Index> SndMapType;
        typedef std::map<Key, SoundSequence> SndSeqMapType;

        RegMapType mpImg;
        SeqMapType mpSeq;
        SndMapType mpSnd;
        SndSeqMapType mpSndSeq;

        std::vector<Color> vTr;
        unsigned nScale;

        FilePath fp;

        SP<GraphicalInterface<Index> > pGr;
        SP<SoundInterface<Index> > pSn;

        ImageSequence LoadSeq(std::string fName);
        Index LoadSndRaw (std::string fName);

    public:
        Preloader(SP<GraphicalInterface<Index> > pGr_, SP<SoundInterface<Index> > pSn_, FilePath fp_ = FilePath())
            :pGr(pGr_), pSn(pSn_), nScale(1), fp(fp_){}

        Index& operator[](Key k);
        ImageSequence& operator()(Key k);
        
        Index& GetSnd(Key k);
        SoundSequence& GetSndSeq(Key k);
        
        void AddTransparent(Color c);
        void SetScale(unsigned nScale_);

        void ApplyTransparency(Index pImg);

        void AddImage(Index pImg, Key k);
        void AddSequence(ImageSequence pImg, Key k);

        void Load  (std::string fName, Key k);
        void LoadT (std::string fName, Key k, Color c = Color(0,0,0,0));
        void LoadS (std::string fName, Key k, unsigned nScale = 0);
        void LoadTS(std::string fName, Key k, Color c = Color(0,0,0,0), unsigned nScale = 0);

        void LoadSeq  (std::string fName, Key k);
        void LoadSeqT (std::string fName, Key k, Color c = Color(0,0,0,0));
        void LoadSeqS (std::string fName, Key k, unsigned nScale = 0);
        void LoadSeqTS(std::string fName, Key k, Color c = Color(0,0,0,0), unsigned nScale_ = 0);

        void LoadSnd    (std::string fName, Key k);
        void LoadSndSeq (std::string fName, Key k);
    };

    typedef Preloader<std::string> MyPreloader;

    template<class Key>
    void Preloader<Key>::AddTransparent(Color c){vTr.push_back(c);}

    template<class Key>
    void Preloader<Key>::SetScale(unsigned nScale_){nScale = nScale_;}

    template<class Key>
    void Preloader<Key>::ApplyTransparency(Index pImg)
    {
        Image* img = pGr->GetImage(pImg);
        for(size_t n = 0, sz = vTr.size(); n < sz; ++n)
            img->ChangeColor(vTr[n], Color(0,0,0,0));
    }

    template<class Key>
    Index& Preloader<Key>::operator[](Key k)
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
    Index& Preloader<Key>::GetSnd(Key k)
    {
        typename SndMapType::iterator itr = mpSnd.find(k);
        if(itr == mpSnd.end())
            throw PreloaderExceptionAccess<Key>("GetSnd", k, false, true);
        return itr->second;
    }

    template<class Key>
    SoundSequence& Preloader<Key>::GetSndSeq(Key k)
    {
        typename SndSeqMapType::iterator itr = mpSndSeq.find(k);
        if(itr == mpSndSeq.end())
            throw PreloaderExceptionAccess<Key>("GetSndSeq", k, true, true);
        return itr->second;
    }


    template<class Key>
    void Preloader<Key>::AddImage(Index pImg, Key k)
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
            Index pImg = pGr->LoadImage(fName);
            //pGr->Image(fName);
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
            Index pImg = pGr->LoadImage(fName);
            
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
            Index pImg = pGr->LoadImage(fName);
            
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
            Index pImg = pGr->LoadImage(fName);
            
            // Format converting
            // pGr->SaveImage(fName, pImg);
            
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

    template<class Key>
    ImageSequence Preloader<Key>::LoadSeq(std::string fName)
    {
        fp.Parse(fName);
        ImageSequence imgSeq;

		SP<InStreamHandler> pFl = fp.ReadFile(fName);
		std::istream& ifs = pFl->GetStream();

        std::string strFolder;
        Separate(fName, strFolder);

        bool bFancy = false;
        
        for(int i = 0;;++i)
        {
            unsigned n = 1;
            std::string s;
            ifs >> s;
            if(bFancy)
                ifs >> n;

            if(ifs.fail())
                break;

            if(i == 0 && s == "FANCY")
            {
                bFancy = true;
                continue;
            }
            fp.Format(s);

            imgSeq.Add(pGr->LoadImage(strFolder + s), n);
            
            // Format Converting
            // pGr->SaveImage(strFolder + s, pGr->LoadImage(strFolder + s));

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

    template<class Key>
    Index Preloader<Key>::LoadSndRaw (std::string fName)
    {
        try
        {
            return pSn->LoadSound(fName);
        }
        catch(SimpleException& exSm)
        {
            PreloaderExceptionLoad ex("LoadSndRaw", fName, true);
            ex.InheritException(exSm);
            throw ex;
        }
    }


    template<class Key>
    void Preloader<Key>::LoadSnd (std::string fName, Key k)
    {
        try
        {
            fp.Parse(fName);
            Index i = LoadSndRaw(fName);
            mpSnd[k] = i;
        }
        catch(PreloaderException& exPre)
        {
            exPre.AddFnName("LoadSnd");
            throw;
        }
    }


    template<class Key>
    void Preloader<Key>::LoadSndSeq(std::string fName, Key k)
    {
        fp.Parse(fName);
        SoundSequence sndSeq;

		SP<InStreamHandler> pFl = fp.ReadFile(fName);
		std::istream& ifs = pFl->GetStream();

		std::string strFolder;
        Separate(fName, strFolder);

        bool bFancy = false;
        
        for(int i = 0;;++i)
        {
            unsigned n = 1;
            std::string s;
            ifs >> s;
            if(bFancy)
                ifs >> n;

            if(ifs.fail())
                break;

            if(i == 0 && s == "FANCY")
            {
                bFancy = true;
                continue;
            }

            try
            {
                sndSeq.Add(LoadSndRaw(strFolder + s), n);
            }
            catch(PreloaderException& exPre)
            {
                exPre.AddFnName("LoadSnd");
                throw;
            }
        }

        if(sndSeq.vSounds.empty())
            throw PreloaderExceptionLoad("LoadSnd", fName, true);

        mpSndSeq[k] = sndSeq;

    }
}

#endif // PRELOADER_HEADER_02_20_10_09_09
