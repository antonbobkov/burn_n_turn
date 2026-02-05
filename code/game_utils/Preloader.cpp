#include "Preloader.h"

namespace Gui {

// --- SoundSequence ---

SoundSequence::SoundSequence() : nActive(0) {}

bool SoundSequence::Toggle() {
  if (nActive == vSounds.size() - 1) {
    nActive = 0;
    return true;
  }
  nActive++;
  return false;
}

Index SoundSequence::GetSound() { return vSounds[nActive]; }

unsigned SoundSequence::GetTime() const {
  if (vIntervals.empty())
    return 1;
  return vIntervals[nActive];
}

void SoundSequence::Add(Index iSnd, unsigned nTime) {
  vSounds.push_back(iSnd);
  vIntervals.push_back(nTime);
}

// --- ImageSequence ---

void ImageSequence::INIT() {
  nActive = 0;
  t = Timer(0);
}

ImageSequence::ImageSequence() { INIT(); }

ImageSequence::ImageSequence(Index img1) {
  INIT();
  Add(img1);
}

ImageSequence::ImageSequence(Index img1, Index img2) {
  INIT();
  Add(img1);
  Add(img2);
}

ImageSequence::ImageSequence(Index img1, Index img2, Index img3) {
  INIT();
  Add(img1);
  Add(img2);
  Add(img3);
}

Index ImageSequence::GetImage() { return vImage[nActive]; }

unsigned ImageSequence::GetTime() const {
  if (vIntervals.empty())
    return 1;
  return vIntervals[nActive];
}

unsigned ImageSequence::GetTotalTime() const {
  int nRet = 0;
  for (unsigned i = 0; i < vIntervals.size(); ++i)
    nRet += vIntervals[i];
  if (nRet == 0)
    return 1;
  return nRet;
}

void ImageSequence::Add(Index nImg, unsigned nTime) {
  vImage.push_back(nImg);
  vIntervals.push_back(nTime);
}

bool ImageSequence::Toggle() {
  if (vImage.empty())
    return true;
  if (nActive == vImage.size() - 1) {
    nActive = 0;
    return true;
  }
  nActive++;
  return false;
}

bool ImageSequence::ToggleTimed() {
  if (t.nPeriod == 0)
    t = Timer(vIntervals.at(nActive));
  if (t.Tick()) {
    bool b = Toggle();
    t = Timer(vIntervals.at(nActive));
    return b;
  }
  return false;
}

// --- ImageFlipper ---

ImageFlipper::ImageFlipper(SP<GraphicalInterface<Index>> pGr_) : pGr(pGr_) {}

void ImageFlipper::operator()(Index &img) { img = pGr->FlipImage(img); }

// --- ImagePainter ---

ImagePainter::ImagePainter(SP<GraphicalInterface<Index>> pGr_, Color cFrom,
                           Color cTo)
    : pGr(pGr_) {
  vTr.push_back(ColorMap(cFrom, cTo));
}

ImagePainter::ImagePainter(SP<GraphicalInterface<Index>> pGr_,
                           const std::vector<ColorMap> &vTr_)
    : pGr(pGr_), vTr(vTr_) {}

void ImagePainter::operator()(Index &img) {
  img = pGr->CopyImage(img);
  Image *pImg = pGr->GetImage(img);
  for (unsigned i = 0; i < vTr.size(); ++i)
    pImg->ChangeColor(vTr[i].first, vTr[i].second);
}

// --- PreloaderException ---

PreloaderException::PreloaderException(crefString strExcName_,
                                       crefString strClsName_,
                                       crefString strFnName_)
    : MyException(strExcName_, strClsName_, strFnName_) {}

// --- PreloaderExceptionAccess ---

PreloaderExceptionAccess::PreloaderExceptionAccess(crefString strFnName_,
                                                   std::string key_, bool bSeq_,
                                                   bool bSnd_)
    : PreloaderException("PreloaderException", "Preloader", strFnName_),
      key(key_), bSeq(bSeq_), bSnd(bSnd_) {}

std::string PreloaderExceptionAccess::GetErrorMessage() const {
  if (bSeq && !bSnd)
    return "No image sequence found with id " + S(key);
  if (!bSeq && !bSnd)
    return "No image found with id " + S(key);
  if (bSeq && bSnd)
    return "No sound sequence found with id " + S(key);
  return "No sound found with id " + S(key);
}

// --- PreloaderExceptionLoad ---

PreloaderExceptionLoad::PreloaderExceptionLoad(crefString strFnName_,
                                               crefString fName_, bool bSound_)
    : PreloaderException("PreloaderException", "Preloader", strFnName_),
      fName(fName_), bSound(bSound_) {}

std::string PreloaderExceptionLoad::GetErrorMessage() const {
  if (!bSound)
    return "Cannot load image from " + fName;
  return "Cannot load sound from " + fName;
}

// --- Preloader ---

Preloader::Preloader(SP<GraphicalInterface<Index>> pGr_,
                     SP<SoundInterface<Index>> pSn_, FilePath fp_)
    : pGr(pGr_), pSn(pSn_), nScale(1), fp(fp_) {}

void Preloader::AddTransparent(Color c) { vTr.push_back(c); }

void Preloader::SetScale(unsigned nScale_) { nScale = nScale_; }

void Preloader::ApplyTransparency(Index pImg) {
  Image *img = pGr->GetImage(pImg);
  for (size_t n = 0, sz = vTr.size(); n < sz; ++n)
    img->ChangeColor(vTr[n], Color(0, 0, 0, 0));
}

Index &Preloader::operator[](std::string key) {
  RegMapType::iterator itr = mpImg.find(key);
  if (itr == mpImg.end())
    throw PreloaderExceptionAccess("[]", key, false);
  return itr->second;
}

ImageSequence &Preloader::operator()(std::string key) {
  SeqMapType::iterator itr = mpSeq.find(key);
  if (itr == mpSeq.end())
    throw PreloaderExceptionAccess("()", key, true);
  return itr->second;
}

Index &Preloader::GetSnd(std::string key) {
  SndMapType::iterator itr = mpSnd.find(key);
  if (itr == mpSnd.end())
    throw PreloaderExceptionAccess("GetSnd", key, false, true);
  return itr->second;
}

SoundSequence &Preloader::GetSndSeq(std::string key) {
  SndSeqMapType::iterator itr = mpSndSeq.find(key);
  if (itr == mpSndSeq.end())
    throw PreloaderExceptionAccess("GetSndSeq", key, true, true);
  return itr->second;
}

void Preloader::AddImage(Index pImg, std::string key) { mpImg[key] = pImg; }

void Preloader::AddSequence(ImageSequence pImg, std::string key) {
  mpSeq[key] = pImg;
}

void Preloader::Load(std::string fName, std::string key) {
  fp.Parse(fName);
  try {
    Index pImg = pGr->LoadImage(fName);
    mpImg[key] = pImg;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("Load", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("Load");
    throw;
  }
}

void Preloader::LoadT(std::string fName, std::string key, Color c) {
  fp.Parse(fName);
  try {
    Index pImg = pGr->LoadImage(fName);
    if (c == Color(0, 0, 0, 0))
      ApplyTransparency(pImg);
    else
      pGr->GetImage(pImg)->ChangeColor(c, Color(0, 0, 0, 0));
    mpImg[key] = pImg;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadT", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadT");
    throw;
  }
}

void Preloader::LoadS(std::string fName, std::string key, unsigned nScale_) {
  fp.Parse(fName);
  try {
    Index pImg = pGr->LoadImage(fName);
    if (nScale_ == 0)
      pImg = pGr->ScaleImage(pImg, nScale);
    else
      pImg = pGr->ScaleImage(pImg, nScale_);
    mpImg[key] = pImg;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadS", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadS");
    throw;
  }
}

void Preloader::LoadTS(std::string fName, std::string key, Color c,
                       unsigned nScale_) {
  fp.Parse(fName);
  try {
    Index pImg = pGr->LoadImage(fName);
    if (c == Color(0, 0, 0, 0))
      ApplyTransparency(pImg);
    else
      pGr->GetImage(pImg)->ChangeColor(c, Color(0, 0, 0, 0));
    if (nScale_ == 0)
      pImg = pGr->ScaleImage(pImg, nScale);
    else
      pImg = pGr->ScaleImage(pImg, nScale_);
    mpImg[key] = pImg;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadTS", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadTS");
    throw;
  }
}

ImageSequence Preloader::LoadSeq(std::string fName) {
  fp.Parse(fName);
  ImageSequence imgSeq;
  SP<InStreamHandler> pFl = fp.ReadFile(fName);
  std::istream &ifs = pFl->GetStream();
  std::string strFolder;
  Separate(fName, strFolder);
  bool bFancy = false;
  for (int i = 0;; ++i) {
    unsigned n = 1;
    std::string s;
    ifs >> s;
    if (bFancy)
      ifs >> n;
    if (ifs.fail())
      break;
    if (i == 0 && s == "FANCY") {
      bFancy = true;
      continue;
    }
    fp.Format(s);
    imgSeq.Add(pGr->LoadImage(strFolder + s), n);
  }
  if (imgSeq.vImage.empty())
    throw PreloaderExceptionLoad("LoadSeq", fName);
  return imgSeq;
}

void Preloader::LoadSeq(std::string fName, std::string key) {
  try {
    ImageSequence imgSeq = LoadSeq(fName);
    mpSeq[key] = imgSeq;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadSeq", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadSeq");
    throw;
  }
}

void Preloader::LoadSeqT(std::string fName, std::string key, Color c) {
  try {
    ImageSequence imgSeq = LoadSeq(fName);
    for (unsigned i = 0, sz = imgSeq.vImage.size(); i < sz; ++i)
      if (c == Color(0, 0, 0, 0))
        ApplyTransparency(imgSeq.vImage[i]);
      else
        pGr->GetImage(imgSeq.vImage[i])->ChangeColor(c, Color(0, 0, 0, 0));
    mpSeq[key] = imgSeq;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadSeqT", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadSeqT");
    throw;
  }
}

void Preloader::LoadSeqS(std::string fName, std::string key, unsigned nScale_) {
  try {
    ImageSequence imgSeq = LoadSeq(fName);
    for (unsigned i = 0, sz = imgSeq.vImage.size(); i < sz; ++i)
      if (nScale_ == 0)
        imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale);
      else
        imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale_);
    mpSeq[key] = imgSeq;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadSeqS", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadSeqS");
    throw;
  }
}

void Preloader::LoadSeqTS(std::string fName, std::string key, Color c,
                          unsigned nScale_) {
  try {
    ImageSequence imgSeq = LoadSeq(fName);
    for (size_t i = 0, sz = imgSeq.vImage.size(); i < sz; ++i) {
      if (c == Color(0, 0, 0, 0))
        ApplyTransparency(imgSeq.vImage[i]);
      else
        pGr->GetImage(imgSeq.vImage[i])->ChangeColor(c, Color(0, 0, 0, 0));
      if (nScale_ == 0)
        imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale);
      else
        imgSeq.vImage[i] = pGr->ScaleImage(imgSeq.vImage[i], nScale_);
    }
    mpSeq[key] = imgSeq;
  } catch (GraphicalInterfaceException &exGr) {
    PreloaderExceptionLoad ex("LoadSeqTS", fName);
    ex.InheritException(exGr);
    throw ex;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadSeqTS");
    throw;
  }
}

Index Preloader::LoadSndRaw(std::string fName) {
  try {
    return pSn->LoadSound(fName);
  } catch (SimpleException &exSm) {
    PreloaderExceptionLoad ex("LoadSndRaw", fName, true);
    ex.InheritException(exSm);
    throw ex;
  }
}

void Preloader::LoadSnd(std::string fName, std::string key) {
  try {
    fp.Parse(fName);
    Index i = LoadSndRaw(fName);
    mpSnd[key] = i;
  } catch (PreloaderException &exPre) {
    exPre.AddFnName("LoadSnd");
    throw;
  }
}

void Preloader::LoadSndSeq(std::string fName, std::string key) {
  fp.Parse(fName);
  SoundSequence sndSeq;
  SP<InStreamHandler> pFl = fp.ReadFile(fName);
  std::istream &ifs = pFl->GetStream();
  std::string strFolder;
  Separate(fName, strFolder);
  bool bFancy = false;
  for (int i = 0;; ++i) {
    unsigned n = 1;
    std::string s;
    ifs >> s;
    if (bFancy)
      ifs >> n;
    if (ifs.fail())
      break;
    if (i == 0 && s == "FANCY") {
      bFancy = true;
      continue;
    }
    try {
      sndSeq.Add(LoadSndRaw(strFolder + s), n);
    } catch (PreloaderException &exPre) {
      exPre.AddFnName("LoadSnd");
      throw;
    }
  }
  if (sndSeq.vSounds.empty())
    throw PreloaderExceptionLoad("LoadSnd", fName, true);
  mpSndSeq[key] = sndSeq;
}

} // namespace Gui
