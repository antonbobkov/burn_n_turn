#ifndef PRELOADER_HEADER_02_20_10_09_09
#define PRELOADER_HEADER_02_20_10_09_09

/*
 * Preloader.h — Asset loading and caching for images, image sequences, and
 * sounds.
 *
 * Preloader is the main type: it loads and caches images, image sequences, and
 * sounds by string key. All paths are resolved through FilePath. You can add
 * transparent colors (AddTransparent), set a default scale (SetScale), and load
 * single assets or whole sequences from list files.
 *
 * Sequence files list filenames (one per line). The optional "FANCY" format
 * adds per-frame timing: after a line "FANCY", each line is "filename N" where
 * N is the frame duration. The same format is used for image sequences
 * (LoadSeq, LoadSeqT, etc.) and sound sequences (LoadSndSeq).
 *
 * Supporting types: ImageSequence and SoundSequence are in image_sequence.h and
 * sound_sequence.h. ImageFlipper and ImagePainter are functors for transforming
 * images; PreloaderException and its subclasses report missing keys or load
 * failures.
 */

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "game_utils/image_sequence.h"
#include "game_utils/sound_sequence.h"
#include "utils/file_utils.h"
#include "utils/index.h"
#include "utils/smart_pointer.h"
#include "wrappers/GuiGen.h"
#include "wrappers/SuiGen.h"
#include "wrappers/color.h"

/** Functor that flips an image horizontally via the graphical interface. */
struct ImageFlipper {
  smart_pointer<GraphicalInterface<Index>> pGr;
  ImageFlipper(smart_pointer<GraphicalInterface<Index>> pGr_);
  void operator()(Index &img);
};

/** Functor that copies an image and applies a list of color replacements. */
struct ImagePainter {
  typedef std::pair<Color, Color> ColorMap;

  smart_pointer<GraphicalInterface<Index>> pGr;
  std::vector<ColorMap> vTr;

  ImagePainter(smart_pointer<GraphicalInterface<Index>> pGr_, Color cFrom,
               Color cTo);
  ImagePainter(smart_pointer<GraphicalInterface<Index>> pGr_,
               const std::vector<ColorMap> &vTr_);
  /** Copy image and apply all vTr color replacements (from -> to). */
  void operator()(Index &img);
};

/** Base exception for Preloader-related errors. */
class PreloaderException : public MyException {
public:
  PreloaderException(crefString strExcName_, crefString strClsName_,
                     crefString strFnName_);
};

/** Thrown when accessing a missing image, image sequence, or sound by key. */
class PreloaderExceptionAccess : public PreloaderException {
public:
  std::string key;
  bool bSeq;
  bool bSnd;

  PreloaderExceptionAccess(crefString strFnName_, std::string key_, bool bSeq_,
                           bool bSnd_ = false);
  /** Message describing which resource (image/seq/sound) was missing. */
  std::string GetErrorMessage() const override;
};

/** Thrown when loading an image or sound file fails. */
class PreloaderExceptionLoad : public PreloaderException {
public:
  std::string fName;
  bool bSound;

  PreloaderExceptionLoad(crefString strFnName_, crefString fName_,
                         bool bSound_ = false);
  /** Message includes the file path that failed to load. */
  std::string GetErrorMessage() const override;
};

/** Loads and caches images, sequences, and sounds by string key. */
class Preloader : virtual public SP_Info {
public:
  std::string get_class_name() override { return "Preloader"; }
  Preloader(smart_pointer<GraphicalInterface<Index>> pGr_,
            smart_pointer<SoundInterface<Index>> pSn_, FilePath *fp);

  /** Image index by key; throws if not found. */
  Index &operator[](std::string key);
  /** Image sequence by key; throws if not found. */
  ImageSequence &operator()(std::string key);

  Index &GetSnd(std::string key);
  SoundSequence &GetSndSeq(std::string key);

  void AddTransparent(Color c);
  void SetScale(unsigned nScale_);

  /** Replace all vTr colors with transparent on the given image. */
  void ApplyTransparency(Index pImg);

  void AddImage(Index pImg, std::string key);
  void AddSequence(ImageSequence pImg, std::string key);

  /** Load image from file and store under key (path via FilePath). */
  void Load(std::string fName, std::string key);
  /** Load image and apply transparency (vTr or replace c with transparent). */
  void LoadT(std::string fName, std::string key, Color c = Color(0, 0, 0, 0));
  /** Load image and scale; nScale 0 uses current SetScale. */
  void LoadS(std::string fName, std::string key, unsigned nScale = 0);
  /** Load image, apply transparency, then scale. */
  void LoadTS(std::string fName, std::string key, Color c = Color(0, 0, 0, 0),
              unsigned nScale = 0);

  /** Load image sequence from file and store under key. */
  void LoadSeq(std::string fName, std::string key);
  /** Load sequence and apply transparency to each frame. */
  void LoadSeqT(std::string fName, std::string key,
                Color c = Color(0, 0, 0, 0));
  /** Load sequence and scale each frame; nScale 0 uses current SetScale. */
  void LoadSeqS(std::string fName, std::string key, unsigned nScale = 0);
  /** Load sequence, apply transparency to each frame, then scale. */
  void LoadSeqTS(std::string fName, std::string key,
                 Color c = Color(0, 0, 0, 0), unsigned nScale_ = 0);

  /** Load sound from file and store under key. */
  void LoadSnd(std::string fName, std::string key);
  /** Load sound sequence from file (list of filenames, optional FANCY) under
   * key. */
  void LoadSndSeq(std::string fName, std::string key);

private:
  typedef std::map<std::string, Index> RegMapType;
  typedef std::map<std::string, ImageSequence> SeqMapType;
  typedef std::map<std::string, Index> SndMapType;
  typedef std::map<std::string, SoundSequence> SndSeqMapType;

  /** Parse sequence file (optional FANCY); load images. */
  ImageSequence LoadSeq(std::string fName);
  /** Load single sound file; throws PreloaderExceptionLoad on failure. */
  Index LoadSndRaw(std::string fName);

  RegMapType mpImg;
  SeqMapType mpSeq;
  SndMapType mpSnd;
  SndSeqMapType mpSndSeq;

  std::vector<Color> vTr;
  unsigned nScale;

  FilePath *fp_;

  smart_pointer<GraphicalInterface<Index>> pGr;
  smart_pointer<SoundInterface<Index>> pSn;
};

#endif // PRELOADER_HEADER_02_20_10_09_09
