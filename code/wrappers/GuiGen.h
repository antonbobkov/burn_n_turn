#ifndef GUIGEN_ALREADY_INCLUDED_GUI_0308
#define GUIGEN_ALREADY_INCLUDED_GUI_0308

/*
 * GuiGen.h - Gui namespace: graphics primitives, drawing abstraction, and
 * implementations. Declares: Color (see color.h), Point, fPoint, Size,
 * Rectangle; Matrix/Image/GraphicalInterface exceptions; Image and
 * GraphicalInterface<ImageHndl>; SimpleGraphicalInterface, CameraControl,
 * Scale; helpers. Defines default GI methods, GuiLoadImage/GuiSaveImage (BMP),
 * SimpleGraphicalInterface and CameraControl bodies.
 */

#include <cmath>
#include <list>
#include <stdlib.h>
#include <string>
#include <vector>

#include <fstream>

#include "color.h"
#include "utils/exception.h"
#include "utils/file_utils.h"
#include "geometry.h"
#include "utils/index.h"
#include "utils/smart_pointer.h"

typedef unsigned char Byte;

/* Size and Point used to describe matrix/rectangle out-of-range errors. */
struct MatrixErrorInfo {
  Size sz;
  Point p;

  MatrixErrorInfo(Size sz_, Point p_) : sz(sz_), p(p_) {}

  std::string GetErrorMessage() const;
};

/* MyException for matrix/rectangle out-of-range; message from MatrixErrorInfo.
 */
class MatrixException : public MyException {
public:
  MatrixErrorInfo inf;

  MatrixException(crefString strClsName_, crefString strFnName_, Size sz,
                  Point p)
      : MyException("MatrixException", strClsName_, strFnName_), inf(sz, p) {}

  MatrixException(crefString strClsName_, crefString strFnName_,
                  MatrixErrorInfo inf_)
      : MyException("MatrixException", strClsName_, strFnName_), inf(inf_) {}

  /*virtual*/ std::string GetErrorMessage() const {
    return inf.GetErrorMessage();
  }
};

/* Per-pixel transparency grid (Size + vector<Byte>); Set/Get(Point), Safe
 * variants throw. */
struct TransparencyGrid {
  Size sz;
  std::vector<Byte> vGrid;

  TransparencyGrid(Size sz_) : sz(sz_), vGrid(sz.x * sz.y, 255) {}

  void Set(Point p, Byte bTrans) { vGrid[p.x + p.y * sz.x] = bTrans; }

  Byte Get(Point p) const { return vGrid[p.x + p.y * sz.x]; }

  void SetSafe(Point p, Byte bTrans) {
    if (!InsideRectangle(Rectangle(sz), p))
      throw MatrixException("TransparencyGrid", "SetSafe", sz, p);

    vGrid[p.x + p.y * sz.x] = bTrans;
  }

  Byte GetSafe(Point p) const {
    if (!InsideRectangle(Rectangle(sz), p))
      throw MatrixException("TransparencyGrid", "GetSafe", sz, p);

    return vGrid[p.x + p.y * sz.x];
  }
};

/* Base exception for image-related errors (name, class, function). */
class ImageException : public MyException {
public:
  ImageException(crefString strExcName_, crefString strClsName_,
                 crefString strFnName_)
      : MyException(strExcName_, strClsName_, strFnName_) {}
};

/* ImageException for out-of-range access; message from MatrixErrorInfo. */
class ImageMatrixException : public ImageException {
public:
  MatrixErrorInfo inf;

  ImageMatrixException(crefString strClsName_, crefString strFnName_, Size sz,
                       Point p)
      : ImageException("ImageMatrixException", strClsName_, strFnName_),
        inf(sz, p) {}

  /*virtual*/ std::string GetErrorMessage() const {
    return inf.GetErrorMessage();
  }
};

/* Default: no check, return value as-is (for non-pointer handle types). */
template <class NullExc, class T>
T PointerAssert(crefString strClsName_, crefString strFnName_,
                crefString strPntName_, T pnt) {
  (void)strClsName_;
  (void)strFnName_;
  (void)strPntName_;
  return pnt;
}
/* Pointer overload: check for null and throw if invalid. */
template <class NullExc, class P>
P *PointerAssert(crefString strClsName_, crefString strFnName_,
                 crefString strPntName_, P *pnt) {
  if (!pnt)
    throw NullExc(strClsName_, strFnName_, strPntName_);
  return pnt;
}

/* ImageException for null pointer; message includes pointer name. */
class ImageNullException : public ImageException {
public:
  std::string strPntName;

  ImageNullException(crefString strClsName_, crefString strFnName_,
                    crefString strPntName_)
      : ImageException("ImageNullException", strClsName_, strFnName_),
        strPntName(strPntName_) {}

  /*virtual*/ std::string GetErrorMessage() const {
    return "Null pointer passed for " + strPntName;
  }
};

// Image - main building block of drawing class, handled by grafical interface
// class
class Image {
  Size sz;

public:
  Image(Size sz_) : sz(sz_) {}
  virtual ~Image() {}

  Size GetSize() const { return sz; }
  Rectangle GetRectangle() const { return Rectangle(sz); }

  virtual void SetPixel(Point p,
                        const Color &c) = 0; // no excetions (no checks)
  virtual Color GetPixel(Point p) const = 0;

  virtual void SetPixelSafe(Point p,
                            const Color &c); // checks if we are out bounds
  virtual Color GetPixelSafe(Point p) const;

  virtual void ChangeColor(
      const Color &cFrom,
      const Color &cTo); // changes all instances of color cFrom to color cTo

  virtual void SetTransparentColor(
      const Color &c); // makes all points with color c transparent
  virtual void
  ColorTransparent(const Color &c); // colors all transparent points to color c
};

/* Base exception for graphical-interface errors (name, class, function). */
class GraphicalInterfaceException : public MyException {
public:
  GraphicalInterfaceException(crefString strExcName_, crefString strClsName_,
                              crefString strFnName_)
      : MyException(strExcName_, strClsName_, strFnName_) {}
};

/* GraphicalInterfaceException with a single problem string as message. */
class GraphicalInterfaceSimpleException : public GraphicalInterfaceException {
  std::string strProblem;

public:
  GraphicalInterfaceSimpleException(crefString strClsName_,
                                    crefString strFnName_,
                                    crefString strProblem_)
      : GraphicalInterfaceException("GraphicalInterfaceSimpleException",
                                    strClsName_, strFnName_),
        strProblem(strProblem_) {}

  /*virtual*/ std::string GetErrorMessage() const { return strProblem; }
};

/* GraphicalInterfaceException for null pointer; message includes pointer name.
 */
class NullPointerGIException : public GraphicalInterfaceException {
public:
  std::string strPntName;

  NullPointerGIException(crefString strClsName_, crefString strFnName_,
                        crefString strPntName_)
      : GraphicalInterfaceException("NullPointerGIException", strClsName_,
                                    strFnName_),
        strPntName(strPntName_) {}

  /*virtual*/ std::string GetErrorMessage() const {
    return "Null pointer passed for " + strPntName;
  }
};

/* Abstract drawing API: create/load/save images, draw images/rectangles,
 * refresh. ImageHndl is backend-specific (e.g. raw pointer or Index). */
template <class ImageHndl> class GraphicalInterface : virtual public SP_Info {
public:
  std::string get_class_name() override { return "GraphicalInterface"; }
  virtual ~GraphicalInterface() {}

  virtual void DeleteImage(ImageHndl pImg) = 0;
  virtual Image *GetImage(ImageHndl pImg) const = 0;
  virtual ImageHndl CopyImage(ImageHndl pImg);

  virtual ImageHndl FlipImage(ImageHndl pImg, bool bHorizontal = true);
  virtual ImageHndl ScaleImage(ImageHndl pImg, unsigned nScale);

  virtual ImageHndl
  GetBlankImage(Size sz) = 0; // create blank image of specified size

  virtual ImageHndl LoadImage(std::string sFileName); // loads image from file
  virtual void SaveImage(std::string sFileName,
                         ImageHndl pImg); // save image to file

  virtual void DrawImage(Point p, ImageHndl pImg,
                         bool bRefresh = true); // draws an image
  virtual void DrawImage(Point p, ImageHndl pImg, Rectangle r,
                         bool bRefresh = true) = 0; // draws part of the image

  virtual void
  DrawRectangle(Rectangle p, Color c,
                bool bRedraw = true) = 0; // draws rectangle onto a screen
  virtual void RectangleOnto(ImageHndl pImg, Rectangle p,
                             Color c); // draws rectangle onto some image

  // virtual void Refresh(Rectangle r)=0;                                    //
  // refreshes part of the screen
  virtual void RefreshAll() = 0; // refreshes all screen

  // draws one image onto another (it is okay for one image not to be fully
  // inside another) p or r.p can have negative values
  virtual void ImageOnto(ImageHndl pImgDest, Point p, ImageHndl pImgSrc,
                         Rectangle r);
};

/* GraphicalInterface<Index> wrapper: delegates to a real GI, uses IndexKeeper
 * and IndexRemover so images are freed when Index is destroyed. */
template <class ImageHndl>
struct SimpleGraphicalInterface : public GraphicalInterface<Index>,
                                  public IndexRemover {
  std::string get_class_name() override { return "SimpleGraphicalInterface"; }
  smart_pointer<GraphicalInterface<ImageHndl>> pGr;

  IndexKeeper<ImageHndl> kp;

  ImageHndl Get(const Index &n) const;
  Index New(ImageHndl pImg);

  /*virtual*/ void DeleteIndex(const Index &pImg) {
    pGr->DeleteImage(Get(pImg));
    kp.FreeElement(pImg.GetIndex());
  }

public:
  SimpleGraphicalInterface(smart_pointer<GraphicalInterface<ImageHndl>> pGr_)
      : pGr(pGr_) {}

  /*virtual*/ void DeleteImage(Index pImg);
  /*virtual*/ Image *GetImage(Index pImg) const;
  /*virtual*/ Index CopyImage(Index pImg);

  /*virtual*/ Index
  GetBlankImage(Size sz); // create blank image of specified size

  /*virtual*/ Index LoadImage(std::string sFileName); // loads image from file
  /*virtual*/ void SaveImage(std::string sFileName,
                             Index pImg); // save image to file

  /*virtual*/ void DrawImage(Point p, Index pImg,
                             bool bRefresh = true); // draws an image
  /*virtual*/ void DrawImage(Point p, Index pImg, Rectangle r,
                             bool bRefresh = true); // draws part of the image

  /*virtual*/ void
  DrawRectangle(Rectangle p, Color c,
                bool bRedraw = true); // draws rectangle onto a screen
  /*virtual*/ void RectangleOnto(Index pImg, Rectangle p,
                                 Color c); // draws rectangle onto some image

  /*virtual*/ void Refresh(Rectangle r); // refreshes part of the screen
  /*virtual*/ void RefreshAll();         // refreshes all screen

  // draws one image onto another (it is okay for one image not to be fully
  // inside another) p or r.p can have negative values
  /*virtual*/ void ImageOnto(Index pImgDest, Point p, Index pImgSrc,
                             Rectangle r);
};

/* View transform: offset, zoom factor, and box size for coordinate conversion.
 */
struct Scale {
  Point pOffset;
  float fZoom;
  Size szBox;

  Scale(float fZoom_ = 1.F, Point pOffset_ = Point(), Size szBox_ = Size())
      : pOffset(pOffset_), fZoom(fZoom_), szBox(szBox_) {}
  Scale(float fZoom_, Rectangle r) : pOffset(r.p), fZoom(fZoom_), szBox(r.sz) {}
};

/* View/camera: stack of Scale (Push/Pop), translate/zoom/SetBox, and
 * coordinate conversion (toR/fromR, toF/fromF). Draws through a GI with
 * scaling. */
template <class ImageHndl> class CameraControl {
  std::list<Scale> lsScales;
  Scale sCurr;

public:
  smart_pointer<GraphicalInterface<ImageHndl>> pGr;

  CameraControl(smart_pointer<GraphicalInterface<ImageHndl>> pGr_ = 0,
                Scale s = Scale())
      : pGr(pGr_), sCurr(s) {}

  Point GetOffset() { return sCurr.pOffset; }
  float GetZoomF() { return sCurr.fZoom; }
  int GetZoom() { return int(sCurr.fZoom); }
  Size GetBox() { return sCurr.szBox; }

  void Push() { lsScales.push_back(sCurr); }
  void Pop();

  void Translate(Point p);
  void BX_Translate(Point p);
  void LU_Translate(Point p);
  void RB_Translate(Point p);

  void Translate(fPoint p) { Translate(fromF(p)); }
  void BX_Translate(fPoint p) { BX_Translate(fromF(p)); }
  void LU_Translate(fPoint p) { LU_Translate(fromF(p)); }
  void RB_Translate(fPoint p) { RB_Translate(fromF(p)); }

  void Zoom(int z);
  void Zoom(float z);
  void SetBox(Size sz);

  ImageHndl LoadImage(std::string sFileName);

  void DrawImage(Point p, ImageHndl pImg, bool bCentered = false,
                 bool bRefresh = false);
  void DrawImage(fPoint p, ImageHndl pImg, bool bCentered = false,
                 bool bRefresh = false);

  void DrawRectangle(Rectangle p, Color c, bool bRedraw = false);

  Point fromF(fPoint p);
  fPoint toF(Point p);

  Point toR(Point p);
  Point fromR(Point p);

  Size toR(Size sz);
  Size fromR(Size sz);

  Rectangle toR(Rectangle r);
  Rectangle fromR(Rectangle r);
};

template <class T>
void ConvertImage(smart_pointer<GraphicalInterface<T>> pGr, std::string strImg,
                  std::string strExtFrom, std::string strExtTo) {
  T img = pGr->LoadImage(strImg + "." + strExtFrom);
  pGr->SaveImage(strImg + "." + strExtTo, img);
  pGr->DeleteImage(img);
}

//-----------------------------------------------------------------------------
// Implementation of various helper functions and template definitions
//-----------------------------------------------------------------------------

// primarily used by ImageOnto - adjusts p and r so that we don't attempt to
// write outside of pictures while drawing
void AdjustImageOverlap(Size sz1, Size sz2, Point &p, Rectangle &r);

// default loading and saving functions (works only with bmp format)
template <class ImageHndl>
ImageHndl GuiLoadImage(std::istream *pStr, GraphicalInterface<ImageHndl> *pGr);
void GuiSaveImage(std::ostream *pStr, const Image *pImg);

template <class ImageHndl>
void GraphicalInterface<ImageHndl>::DrawImage(Point p, ImageHndl pImg,
                                              bool bRefresh /* = true*/) {
  PointerAssert<ImageNullException>("GraphicalInterface", "DrawImage(small)",
                                    "pImg", pImg);

  DrawImage(p, pImg, Rectangle(GetImage(pImg)->GetSize()),
            bRefresh); // draw all image
}

inline unsigned GetBmpPos(Point p, Size sz) // bmp coordinate recount function
{
  return (3 * sz.x + 3 - (3 * sz.x - 1) % 4) * (sz.y - p.y - 1) + p.x * 3;
}

inline Byte DrawColorOnto(float fColor1, float fColor2,
                          float fTr2) // formula for drawing transparent colors
{
  return Byte((fColor1 * (1 - fTr2 / 255) + fTr2 / 255 * fColor2));
}

inline void ExplDrawColorOnto(Byte &fColor1, float fColor2,
                              float fTr2) // faster version of DrawColorOnto
{
  fColor1 = Byte((float(fColor1) * (1 - fTr2 / 255) + fTr2 / 255 * fColor2));
}

template <class ImageHndl>
ImageHndl GraphicalInterface<ImageHndl>::CopyImage(ImageHndl pImg) {
  Image *pPureImg = GetImage(pImg);
  Size sz = pPureImg->GetSize();

  ImageHndl pRet = GetBlankImage(sz);

  // ImageOnto(pRet, Point(0,0), pImg, Rectangle(sz)); // messing with
  // transparency

  Image *pImgImg = GetImage(pImg);
  Image *pRetImg = GetImage(pRet);

  Point p;
  for (p.y = 0; p.y < sz.y; ++p.y)
    for (p.x = 0; p.x < sz.x; ++p.x)
      pRetImg->SetPixel(p, pImgImg->GetPixel(p));

  return pRet;
}

template <class ImageHndl>
ImageHndl GraphicalInterface<ImageHndl>::FlipImage(ImageHndl pImg,
                                                   bool bHorizontal) {
  Image *imgSrc = GetImage(pImg);
  Size sz = imgSrc->GetSize();
  ImageHndl pRet = GetBlankImage(sz);
  Image *imgTar = GetImage(pRet);

  Point p;
  if (bHorizontal)
    for (p.y = 0; p.y < sz.y; ++p.y)
      for (p.x = 0; p.x < sz.x; ++p.x)
        imgTar->SetPixel(Point(sz.x - p.x - 1, p.y), imgSrc->GetPixel(p));
  else
    for (p.y = 0; p.y < sz.y; ++p.y)
      for (p.x = 0; p.x < sz.x; ++p.x)
        imgTar->SetPixel(Point(p.x, sz.y - p.y - 1), imgSrc->GetPixel(p));

  return pRet;
}

template <class ImageHndl>
ImageHndl GraphicalInterface<ImageHndl>::ScaleImage(ImageHndl pImg,
                                                    unsigned nScale) {
  Image *pOrig = GetImage(pImg);
  Size sz = pOrig->GetSize();
  ImageHndl pRet = GetBlankImage(Size(sz.x * nScale, sz.y * nScale));
  Image *pFin = GetImage(pRet);

  Point p;
  unsigned x, y;
  for (p.y = 0; p.y < sz.y; ++p.y)
    for (p.x = 0; p.x < sz.x; ++p.x)
      for (y = 0; y < nScale; ++y)
        for (x = 0; x < nScale; ++x)
          pFin->SetPixel(Point(p.x * nScale + x, p.y * nScale + y),
                         pOrig->GetPixel(p));

  return pRet;
}

// default implementation - uses GuiLoadImage
template <class ImageHndl>
ImageHndl GraphicalInterface<ImageHndl>::LoadImage(std::string sFileName) {
  std::ifstream ifs(sFileName.c_str(),
                    std::ios_base::in | std::ios_base::binary);

  if (ifs.fail())
    throw GraphicalInterfaceSimpleException(
        "GraphicalInterface", "LoadImage",
        "Cannot load image (cannot open file " + sFileName + ")");

  try {
    return GuiLoadImage(&ifs, this);
  } catch (SimpleException &se) {
    GraphicalInterfaceSimpleException ge("GraphicalInterface", "LoadImage",
                                         "Cannot load image from file " +
                                             sFileName);
    ge.InheritException(se);
    throw ge;
  }
}

// default implementation - uses GuiSaveImage
template <class ImageHndl>
void GraphicalInterface<ImageHndl>::SaveImage(std::string sFileName,
                                              ImageHndl pImg) {
  PointerAssert<NullPointerGIException>("GraphicalInterface", "SaveImage",
                                        "pImg", pImg);

  std::ofstream ofs(sFileName.c_str(),
                    std::ios_base::out | std::ios_base::binary);

  if (ofs.fail())
    throw GraphicalInterfaceSimpleException(
        "GraphicalInterface", "SaveImage",
        "Cannot save image (cannot open file " + sFileName + ")");

  try {
    GuiSaveImage(&ofs, GetImage(pImg));
  } catch (SimpleException &se) {
    GraphicalInterfaceSimpleException ge("GraphicalInterface", "SaveImage",
                                         "Cannot save image into file " +
                                             sFileName);
    ge.InheritException(se);
    throw ge;
  }
}

template <class ImageHndl>
void GraphicalInterface<ImageHndl>::RectangleOnto(ImageHndl pImg, Rectangle p,
                                                  Color c) {
  PointerAssert<NullPointerGIException>("GraphicalInterface", "RectangleOnto",
                                        "pImg", pImg);

  p = Intersect(p, GetImage(pImg)->GetSize());

  Point i;
  Size sz(p.p.x + p.sz.x, p.p.y + p.sz.y);
  for (i.y = p.p.y; i.y < sz.y; ++i.y)
    for (i.x = p.p.x; i.x < sz.x; ++i.x)
      GetImage(pImg)->SetPixel(i, c);
}

template <class ImageHndl>
void GraphicalInterface<ImageHndl>::ImageOnto(ImageHndl pImgDest, Point p,
                                              ImageHndl pImgSrc, Rectangle r) {
  PointerAssert<NullPointerGIException>("GraphicalInterface", "ImageOnto",
                                        "pImgDest", pImgDest);
  PointerAssert<NullPointerGIException>("GraphicalInterface", "ImageOnto",
                                        "pImgSrc", pImgSrc);

  try {
    bool bSelf = (pImgDest == pImgSrc); // take care of the case when we try to
                                        // draw image onto itself

    if (bSelf)
      pImgSrc = CopyImage(pImgDest); // need a copy

    // make sure we are inside the bounds
    AdjustImageOverlap(GetImage(pImgDest)->GetSize(),
                       GetImage(pImgSrc)->GetSize(), p, r);

    Point i, j;
    Size sz = Size(r.p.x + r.sz.x, r.p.y + r.sz.y);
    for (i.y = r.p.y, j.y = p.y; i.y < sz.y; ++i.y, ++j.y)
      for (i.x = r.p.x, j.x = p.x; i.x < sz.x; ++i.x, ++j.x) {
        Color b = GetImage(pImgDest)->GetPixel(j);
        Color t = GetImage(pImgSrc)->GetPixel(i);

        Color b2;
        b2.nTransparent = b.nTransparent;
        b2.R = DrawColorOnto(b.R, t.R, t.nTransparent);
        b2.G = DrawColorOnto(b.G, t.G, t.nTransparent);
        b2.B = DrawColorOnto(b.B, t.B, t.nTransparent);

        if (!(b2 == b))
          GetImage(pImgDest)->SetPixel(j, b2);
      }

    if (bSelf)
      DeleteImage(pImgSrc);
  } catch (GraphicalInterfaceException &ie) {
    ie.AddFnName("ImageOnto");
    throw;
  } catch (ImageException &ie) {
    GraphicalInterfaceSimpleException gse("GraphicalInterface", "ImageOnto",
                                          "Cannot draw onto an image");
    gse.InheritException(ie);
    throw gse;
  }
}

template <class ImageHndl>
ImageHndl GuiLoadImage(std::istream *pStr, GraphicalInterface<ImageHndl> *pGr) {
  PointerAssert<SimpleException>("<global>", "GuiLoadImage", "pStr", pStr);
  PointerAssert<SimpleException>("<global>", "GuiLoadImage", "pGr", pGr);

  // by some weird reason bfType counts as 4 bytes as opposing to 2, giving an
  // invalid format so we have to write everything by hand
  struct BmpFileHdr {
    unsigned short bfType;
    unsigned long bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long bfOffBits;
  };

  struct BmpInfoHdr {
    unsigned long biSize;
    long biWidth;
    long biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long biCompression;
    unsigned long biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    unsigned long biClrUsed;
    unsigned long biClrImportant;
  };

  BmpFileHdr bmfh;

  // pStr->write((char*) &bmfh, sizeof(BmpFileHdr));
  pStr->read((char *)&(bmfh.bfType), 2);
  pStr->read((char *)&(bmfh.bfSize), 4);
  pStr->read((char *)&(bmfh.bfReserved1), 2);
  pStr->read((char *)&(bmfh.bfReserved2), 2);
  pStr->read((char *)&(bmfh.bfOffBits), 4);

  if (pStr->fail())
    throw SimpleException(
        "<global>", "GuiLoadImage",
        "Failed to read from the provided stream (bmp file header)");
  if (bmfh.bfType != 0x4d42)
    throw SimpleException("<global>", "GuiLoadImage", "Bad format of bmp file");

  BmpInfoHdr bmih;

  // pStr->write((char*) &bmih, sizeof(BmpInfoHdr));
  pStr->read((char *)&(bmih.biSize), 4);
  pStr->read((char *)&(bmih.biWidth), 4);
  pStr->read((char *)&(bmih.biHeight), 4);
  pStr->read((char *)&(bmih.biPlanes), 2);
  pStr->read((char *)&(bmih.biBitCount), 2);
  pStr->read((char *)&(bmih.biCompression), 4);
  pStr->read((char *)&(bmih.biSizeImage), 4);
  pStr->read((char *)&(bmih.biXPelsPerMeter), 4);
  pStr->read((char *)&(bmih.biYPelsPerMeter), 4);
  pStr->read((char *)&(bmih.biClrUsed), 4);
  pStr->read((char *)&(bmih.biClrImportant), 4);

  if (pStr->fail())
    throw SimpleException(
        "<global>", "GuiLoadImage",
        "Failed to read from the provided stream (bmp info header)");

  ImageHndl pRet = pGr->GetBlankImage(Size(bmih.biWidth, bmih.biHeight));

  int nFourByteOffset =
      (4 - bmih.biWidth * 3 % 4) % 4; // extra zeros for 4byte forced width

  Point p;
  for (p.y = bmih.biHeight - 1; p.y >= 0; --p.y) {
    for (p.x = 0; p.x < bmih.biWidth; ++p.x) {
      Color c;
      pStr->read((char *)&c, 3);
      c.nTransparent = 255;
      pGr->GetImage(pRet)->SetPixel(p, c);
    }

    char tBuf[4];
    pStr->read(tBuf, nFourByteOffset); // extra zeros for 4byte forced width
  }

  if (pStr->fail()) {
    pGr->DeleteImage(pRet);
    throw SimpleException("<global>", "GuiLoadImage",
                          "Failed to read from the provided stream (bmp data)");
  }

  return pRet;
}

template <class ImageHndl>
ImageHndl SimpleGraphicalInterface<ImageHndl>::Get(const Index &n) const {
  return kp.GetElement(n.GetIndex());
}

template <class ImageHndl>
Index SimpleGraphicalInterface<ImageHndl>::New(ImageHndl pImg) {
  unsigned n = kp.GetNewIndex();
  kp.GetElement(n) = pImg;
  return Index(n, this);
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::DeleteImage(Index pImg) {}

template <class ImageHndl>
Index SimpleGraphicalInterface<ImageHndl>::CopyImage(Index pImg) {
  return New(pGr->CopyImage(Get(pImg)));
}

template <class ImageHndl>
Image *SimpleGraphicalInterface<ImageHndl>::GetImage(Index pImg) const {
  return pGr->GetImage(Get(pImg));
}

template <class ImageHndl>
Index SimpleGraphicalInterface<ImageHndl>::GetBlankImage(Size sz) {
  return New(pGr->GetBlankImage(sz));
}

template <class ImageHndl>
Index SimpleGraphicalInterface<ImageHndl>::LoadImage(std::string sFileName) {
  return New(pGr->LoadImage(sFileName));
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::SaveImage(std::string sFileName,
                                                    Index pImg) {
  pGr->SaveImage(sFileName, Get(pImg));
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::DrawImage(Point p, Index pImg,
                                                    bool bRefresh) {
  pGr->DrawImage(p, Get(pImg), bRefresh);
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::DrawImage(Point p, Index pImg,
                                                    Rectangle r,
                                                    bool bRefresh) {
  pGr->DrawImage(p, Get(pImg), r, bRefresh);
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::DrawRectangle(Rectangle p, Color c,
                                                        bool bRedraw) {
  pGr->DrawRectangle(p, c, bRedraw);
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::RectangleOnto(Index pImg, Rectangle p,
                                                        Color c) {
  pGr->RectangleOnto(Get(pImg), p, c);
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::Refresh(Rectangle r) {
  pGr->Refresh(r);
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::RefreshAll() {
  pGr->RefreshAll();
}

template <class ImageHndl>
void SimpleGraphicalInterface<ImageHndl>::ImageOnto(Index pImgDest, Point p,
                                                    Index pImgSrc,
                                                    Rectangle r) {
  pGr->ImageOnto(Get(pImgDest), p, Get(pImgSrc), r);
}

template <class T> void CameraControl<T>::Pop() {
  if (lsScales.empty())
    return;
  sCurr = lsScales.back();
  lsScales.pop_back();
}

template <class T> void CameraControl<T>::Translate(Point p) {
  sCurr.pOffset = toR(p);
}

template <class T> void CameraControl<T>::BX_Translate(Point p) {
  sCurr.szBox.x -= p.x * 2;
  sCurr.szBox.y -= p.y * 2;
  sCurr.pOffset = toR(p);
}

template <class T> void CameraControl<T>::LU_Translate(Point p) {
  sCurr.szBox.x -= p.x;
  sCurr.szBox.y -= p.y;
  sCurr.pOffset = toR(p);
}

template <class T> void CameraControl<T>::RB_Translate(Point p) {
  sCurr.szBox.x -= p.x;
  sCurr.szBox.y -= p.y;
}

template <class T> void CameraControl<T>::Zoom(int z) {
  sCurr.fZoom *= z;
  sCurr.szBox.x /= z;
  sCurr.szBox.y /= z;
}

template <class T> void CameraControl<T>::Zoom(float z) {
  sCurr.fZoom *= z;
  sCurr.szBox.x = int(sCurr.szBox.x / z);
  sCurr.szBox.y = int(sCurr.szBox.y / z);
}

template <class T> void CameraControl<T>::SetBox(Size sz) { sCurr.szBox = sz; }

template <class ImageHndl>
ImageHndl CameraControl<ImageHndl>::LoadImage(std::string sFileName) {
  ImageHndl img = pGr->LoadImage(sFileName);
  img = pGr->ScaleImage(img, GetZoom());
  return img;
}

template <class ImageHndl>
void CameraControl<ImageHndl>::DrawImage(Point p, ImageHndl pImg,
                                         bool bCentered, bool bRefresh) {
  p = toR(p);
  if (bCentered) {
    Size sz = pGr->GetImage(pImg)->GetSize();
    p.x -= sz.x / 2;
    p.y -= sz.y / 2;
  }

  pGr->DrawImage(p, pImg, bRefresh);
}

template <class ImageHndl>
void CameraControl<ImageHndl>::DrawImage(fPoint p, ImageHndl pImg,
                                         bool bCentered, bool bRefresh) {
  DrawImage(fromF(p), pImg, bCentered, bRefresh);
}

template <class T>
void CameraControl<T>::DrawRectangle(Rectangle p, Color c, bool bRedraw) {
  p.p = toR(p.p);
  p.sz = toR(p.sz);

  pGr->DrawRectangle(p, c, bRedraw);
}

template <class T> Point CameraControl<T>::fromF(fPoint p) {
  Point pRet;
  pRet.x = int(sCurr.szBox.x * p.x);
  pRet.y = int(sCurr.szBox.y * p.y);
  return pRet;
}

template <class T> fPoint CameraControl<T>::toF(Point p) {
  fPoint f;
  f.x = float(p.x) / sCurr.szBox.x;
  f.y = float(p.y) / sCurr.szBox.y;
  return f;
}

template <class T> Point CameraControl<T>::toR(Point p) {
  p.x = int(p.x * sCurr.fZoom);
  p.y = int(p.y * sCurr.fZoom);
  return sCurr.pOffset + p;
}

template <class T> Point CameraControl<T>::fromR(Point p) {
  p -= sCurr.pOffset;
  p.x = int(p.x / sCurr.fZoom);
  p.y = int(p.y / sCurr.fZoom);
  return p;
}

template <class T> Size CameraControl<T>::toR(Size sz) {
  sz.x = int(sz.x * sCurr.fZoom);
  sz.y = int(sz.y * sCurr.fZoom);
  return sz;
}

template <class T> Size CameraControl<T>::fromR(Size sz) {
  sz.x = int(sz.x / sCurr.fZoom);
  sz.y = int(sz.y / sCurr.fZoom);
  return sz;
}

template <class T> Rectangle CameraControl<T>::toR(Rectangle r) {
  return Rectangle(toR(r.p), toR(r.sz));
}

template <class T> Rectangle CameraControl<T>::fromR(Rectangle r) {
  return Rectangle(fromR(r.p), fromR(r.sz));
}

#endif // GUIGEN_ALREADY_INCLUDED_GUI_0308
