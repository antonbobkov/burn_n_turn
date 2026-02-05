#ifndef PRELOADER_HEADER_02_20_10_09_09
#define PRELOADER_HEADER_02_20_10_09_09

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "General.h"
#include "GuiGen.h"
#include "SuiGen.h"

namespace Gui {

struct SoundSequence {
  std::vector<Index> vSounds;
  std::vector<unsigned> vIntervals;
  unsigned nActive;

  bool Toggle() {
    if (nActive == vSounds.size() - 1) {
      nActive = 0;
      return true;
    } else
      nActive++;
    return false;
  }

  Index GetSound() { return vSounds[nActive]; }
  unsigned GetTime() const {
    if (vIntervals.empty())
      return 1;
    return vIntervals[nActive];
  }

  void Add(Index iSnd, unsigned nTime = 1) {
    vSounds.push_back(iSnd);
    vIntervals.push_back(nTime);
  }

  SoundSequence() : nActive(0) {}
};

struct ImageSequence : virtual public SP_Info {
  std::vector<Index> vImage;
  std::vector<unsigned> vIntervals;
  unsigned nActive;

  Timer t;

  bool Toggle();
  bool ToggleTimed();

  Index GetImage() { return vImage[nActive]; }
  unsigned GetTime() const {
    if (vIntervals.empty())
      return 1;
    return vIntervals[nActive];
  }
  unsigned GetTotalTime() const {
    int nRet = 0;
    for (unsigned i = 0; i < vIntervals.size(); ++i)
      nRet += vIntervals[i];
    if (nRet == 0)
      return 1;
    return nRet;
  }

  void Add(Index nImg, unsigned nTime = 1) {
    vImage.push_back(nImg);
    vIntervals.push_back(nTime);
  }

  void INIT() {
    nActive = 0;
    t = Timer(0);
  }

  ImageSequence() { INIT(); }
  ImageSequence(Index img1) {
    INIT();
    Add(img1);
  }
  ImageSequence(Index img1, Index img2) {
    INIT();
    Add(img1);
    Add(img2);
  }
  ImageSequence(Index img1, Index img2, Index img3) {
    INIT();
    Add(img1);
    Add(img2);
    Add(img3);
  }
};

template <class T> void ForEachImage(ImageSequence &img, T t) {
  for (unsigned i = 0; i < img.vImage.size(); ++i)
    t(img.vImage[i]);
}

struct ImageFlipper {
  SP<GraphicalInterface<Index>> pGr;
  ImageFlipper(SP<GraphicalInterface<Index>> pGr_) : pGr(pGr_) {}
  void operator()(Index &img) { img = pGr->FlipImage(img); }
};

struct ImagePainter {
  typedef std::pair<Color, Color> ColorMap;

  SP<GraphicalInterface<Index>> pGr;

  std::vector<ColorMap> vTr;

  ImagePainter(SP<GraphicalInterface<Index>> pGr_, Color cFrom, Color cTo)
      : pGr(pGr_) {
    vTr.push_back(ColorMap(cFrom, cTo));
  }

  ImagePainter(SP<GraphicalInterface<Index>> pGr_,
               const std::vector<ColorMap> &vTr_)
      : pGr(pGr_), vTr(vTr_) {}

  void operator()(Index &img) {
    img = pGr->CopyImage(img);
    Image *pImg = pGr->GetImage(img);

    for (unsigned i = 0; i < vTr.size(); ++i)
      pImg->ChangeColor(vTr[i].first, vTr[i].second);
  }
};

class PreloaderException : public MyException {
public:
  PreloaderException(crefString strExcName_, crefString strClsName_,
                     crefString strFnName_)
      : MyException(strExcName_, strClsName_, strFnName_) {}
};

class PreloaderExceptionAccess : public PreloaderException {
public:
  std::string key;
  bool bSeq;
  bool bSnd;

  PreloaderExceptionAccess(crefString strFnName_, std::string key_, bool bSeq_,
                           bool bSnd_ = false)
      : PreloaderException("PreloaderException", "Preloader", strFnName_),
        key(key_), bSeq(bSeq_), bSnd(bSnd_) {}

  /*virtual*/ std::string GetErrorMessage() const {
    if (bSeq && !bSnd)
      return "No image sequence found with id " + S(key);
    else if (!bSeq && !bSnd)
      return "No image found with id " + S(key);
    else if (bSeq && bSnd)
      return "No sound sequence found with id " + S(key);
    else
      return "No sound found with id " + S(key);
  }
};

class PreloaderExceptionLoad : public PreloaderException {
public:
  std::string fName;
  bool bSound;

  PreloaderExceptionLoad(crefString strFnName_, crefString fName_,
                         bool bSound_ = false)
      : PreloaderException("PreloaderException", "Preloader", strFnName_),
        fName(fName_), bSound(bSound_) {}

  /*virtual*/ std::string GetErrorMessage() const {
    if (!bSound)
      return "Cannot load image from " + fName;
    else
      return "Cannot load sound from " + fName;
  }
};

class Preloader : virtual public SP_Info {
public:
  Preloader(SP<GraphicalInterface<Index>> pGr_, SP<SoundInterface<Index>> pSn_,
            FilePath fp_ = FilePath())
      : pGr(pGr_), pSn(pSn_), nScale(1), fp(fp_) {}

  Index &operator[](std::string key);
  ImageSequence &operator()(std::string key);

  Index &GetSnd(std::string key);
  SoundSequence &GetSndSeq(std::string key);

  void AddTransparent(Color c);
  void SetScale(unsigned nScale_);

  void ApplyTransparency(Index pImg);

  void AddImage(Index pImg, std::string key);
  void AddSequence(ImageSequence pImg, std::string key);

  void Load(std::string fName, std::string key);
  void LoadT(std::string fName, std::string key, Color c = Color(0, 0, 0, 0));
  void LoadS(std::string fName, std::string key, unsigned nScale = 0);
  void LoadTS(std::string fName, std::string key, Color c = Color(0, 0, 0, 0),
              unsigned nScale = 0);

  void LoadSeq(std::string fName, std::string key);
  void LoadSeqT(std::string fName, std::string key,
                Color c = Color(0, 0, 0, 0));
  void LoadSeqS(std::string fName, std::string key, unsigned nScale = 0);
  void LoadSeqTS(std::string fName, std::string key,
                 Color c = Color(0, 0, 0, 0), unsigned nScale_ = 0);

  void LoadSnd(std::string fName, std::string key);
  void LoadSndSeq(std::string fName, std::string key);

private:
  typedef std::map<std::string, Index> RegMapType;
  typedef std::map<std::string, ImageSequence> SeqMapType;
  typedef std::map<std::string, Index> SndMapType;
  typedef std::map<std::string, SoundSequence> SndSeqMapType;

  ImageSequence LoadSeq(std::string fName);
  Index LoadSndRaw(std::string fName);

  RegMapType mpImg;
  SeqMapType mpSeq;
  SndMapType mpSnd;
  SndSeqMapType mpSndSeq;

  std::vector<Color> vTr;
  unsigned nScale;

  FilePath fp;

  SP<GraphicalInterface<Index>> pGr;
  SP<SoundInterface<Index>> pSn;
};

} // namespace Gui

#endif // PRELOADER_HEADER_02_20_10_09_09
