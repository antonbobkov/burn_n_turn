#ifndef SUIGEN_ALREADY_INCLUDED_08_25
#define SUIGEN_ALREADY_INCLUDED_08_25

#include "General.h"
#include "SmartPointer.h"
namespace Gui {

template <class SndHndl> class SoundInterface : virtual public SP_Info {
public:
  virtual ~SoundInterface() {}

  virtual void DeleteSound(SndHndl snd) = 0;
  virtual SndHndl LoadSound(std::string sFile) = 0;

  virtual void PlaySound(SndHndl snd, int nChannel = -1,
                         bool bLoop = false) = 0;
  virtual void StopSound(int channel = -1) = 0;
  virtual void SetVolume(float fVolume) {}
};

template <class SndHndl>
class SimpleSoundInterface : public SoundInterface<Index>, public IndexRemover {
  SP<SoundInterface<SndHndl>> pSn;

  IndexKeeper<SndHndl> kp;

public:
  SimpleSoundInterface(SP<SoundInterface<SndHndl>> pSn_) : pSn(pSn_) {}

  /*virtual*/ void DeleteSound(Index snd);
  /*virtual*/ Index LoadSound(std::string sFile);

  /*virtual*/ void PlaySound(Index snd, int nChannel = -1, bool bLoop = false);
  /*virtual*/ void StopSound(int nChannel = -1);
  /*virtual*/ void SetVolume(float fVolume); // value 0-1

  /*virtual*/ void DeleteIndex(const Index &i);
};

template <class T> class DummySoundInterface : public SoundInterface<T> {
  /*virtual*/ void DeleteSound(T snd){};
  /*virtual*/ T LoadSound(std::string sFile) { return T(); }

  /*virtual*/ void PlaySound(T snd, int nChannel = -1, bool bLoop = false) {}
};

template <class SndHndl>
void SimpleSoundInterface<SndHndl>::DeleteSound(Index snd) {}

template <class SndHndl>
Index SimpleSoundInterface<SndHndl>::LoadSound(std::string sFile) {
  SndHndl snd = pSn->LoadSound(sFile);
  unsigned n = kp.GetNewIndex();
  kp.GetElement(n) = snd;
  return Index(n, this);
}

template <class SndHndl>
void SimpleSoundInterface<SndHndl>::PlaySound(Index snd, int nChannel,
                                              bool bLoop) {
  pSn->PlaySound(kp.GetElement(snd.GetIndex()), nChannel, bLoop);
}

template <class SndHndl>
void SimpleSoundInterface<SndHndl>::StopSound(int nChannel) {
  pSn->StopSound(nChannel);
}

template <class SndHndl>
void SimpleSoundInterface<SndHndl>::SetVolume(float fVolume) {
  pSn->SetVolume(fVolume);
}

template <class SndHndl>
void SimpleSoundInterface<SndHndl>::DeleteIndex(const Index &i) {
  pSn->DeleteSound(kp.GetElement(i.GetIndex()));
  kp.FreeElement(i.GetIndex());
}
} // namespace Gui

#endif // SUIGEN_ALREADY_INCLUDED_08_25