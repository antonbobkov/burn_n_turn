#ifndef GUIGEN_ALREADY_INCLUDED_GUI_0308
#define GUIGEN_ALREADY_INCLUDED_GUI_0308

/*
 * GuiGen.h - Implementations for GuiGenHdr.h declarations.
 * Defines: AdjustImageOverlap; default GraphicalInterface<ImageHndl> methods
 * (CopyImage, FlipImage, ScaleImage, LoadImage, SaveImage, DrawImage,
 * RectangleOnto, ImageOnto); GuiLoadImage/GuiSaveImage (BMP);
 * SimpleGraphicalInterface and CameraControl method bodies; BMP file/info
 * header structs for loading.
 */

#include "GuiGenHdr.h"

namespace Gui {

//--------------------------------------------------------------------------------------------------
// Implementation of various helper functions and template definitions
//--------------------------------------------------------------------------------------------------

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
} // namespace Gui

#endif // GUIGEN_ALREADY_INCLUDED_GUI_0308
