#include <sstream>

#include "color.h"
#include "GuiGen.h"
#include "utils/smart_pointer.h"

std::string MatrixErrorInfo::GetErrorMessage() const {
  std::ostringstream ostr;
  ostr << "Point (" << p.x << ", " << p.y << ") is out of bounds in " << sz.x
       << "x" << sz.y << " matrix";
  return ostr.str();
}

void Image::SetPixelSafe(Point p, const Color &c) {
  if (!InsideRectangle(Rectangle(sz), p))
    throw ImageMatrixException("Image", "SetPixelSafe", sz, p);
  SetPixel(p, c);
}

Color Image::GetPixelSafe(Point p) const {
  if (!InsideRectangle(Rectangle(sz), p))
    throw ImageMatrixException("Image", "GetPixelSafe", sz, p);
  return GetPixel(p);
}

void Image::ChangeColor(const Color &cFrom, const Color &cTo) {
  Point p;

  for (p.y = 0; p.y < sz.y; ++p.y)
    for (p.x = 0; p.x < sz.x; ++p.x)
      if (GetPixel(p) == cFrom)
        SetPixel(p, cTo);
}

void Image::SetTransparentColor(const Color &c) {
  ChangeColor(c, Color(0, 0, 0, 0));
}

void Image::ColorTransparent(const Color &c) {
  ChangeColor(Color(0, 0, 0, 0), c);
}

void AdjustImageOverlap(Size sz1, Size sz2, Point &p, Rectangle &r) {
  Rectangle rTop = Intersect(r, Rectangle(sz2));
  p = p + rTop.p - r.p;
  Rectangle rBot = Intersect(Rectangle(sz1), Rectangle(p, rTop.sz));
  r = Rectangle(rTop.p + rBot.p - p, rBot.sz);
  p = rBot.p;
}

void GuiSaveImage(std::ostream *pStr, const Image *pImg) {
  PointerAssert<SimpleException>("<global>", "GuiSaveImage", "pStr", pStr);
  PointerAssert<SimpleException>("<global>", "GuiSaveImage", "pImg", pImg);

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

  Size sz = pImg->GetSize();

  BmpFileHdr bmfh;
  bmfh.bfType = 0x4d42; // 'BM'
  bmfh.bfSize = 0;
  bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
  // bmfh.bfOffBits = sizeof(BmpFileHdr) + sizeof(BmpInfoHdr);// +
  // sizeof(RGBQUAD) * m_nColorTableEntries;
  bmfh.bfOffBits = 14 + 40; // + sizeof(RGBQUAD) * m_nColorTableEntries;

  int nFourByteOffset =
      (4 - sz.x * 3 % 4) % 4; // extra zeros for 4byte forced width

  BmpInfoHdr bmih;
  bmih.biSize = sizeof(BmpInfoHdr);
  bmih.biWidth = sz.x;
  bmih.biHeight = sz.y;
  bmih.biPlanes = 1;
  bmih.biBitCount = 24;
  bmih.biCompression = 0;
  bmih.biSizeImage = (sz.x * 3 + nFourByteOffset) * sz.y;
  bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = bmih.biClrUsed =
      bmih.biClrImportant = 0;

  // pStr->write((char*) &bmfh, sizeof(BmpFileHdr));
  pStr->write((char *)&(bmfh.bfType), 2);
  pStr->write((char *)&(bmfh.bfSize), 4);
  pStr->write((char *)&(bmfh.bfReserved1), 2);
  pStr->write((char *)&(bmfh.bfReserved2), 2);
  pStr->write((char *)&(bmfh.bfOffBits), 4);

  // pStr->write((char*) &bmih, sizeof(BmpInfoHdr));
  pStr->write((char *)&(bmih.biSize), 4);
  pStr->write((char *)&(bmih.biWidth), 4);
  pStr->write((char *)&(bmih.biHeight), 4);
  pStr->write((char *)&(bmih.biPlanes), 2);
  pStr->write((char *)&(bmih.biBitCount), 2);
  pStr->write((char *)&(bmih.biCompression), 4);
  pStr->write((char *)&(bmih.biSizeImage), 4);
  pStr->write((char *)&(bmih.biXPelsPerMeter), 4);
  pStr->write((char *)&(bmih.biYPelsPerMeter), 4);
  pStr->write((char *)&(bmih.biClrUsed), 4);
  pStr->write((char *)&(bmih.biClrImportant), 4);

  Point p;
  for (p.y = sz.y - 1; p.y >= 0; --p.y) {
    for (p.x = 0; p.x < sz.x; ++p.x) {
      Color c = pImg->GetPixel(p);
      pStr->write((char *)&c, 3);
    }

    for (int i = 0; i < nFourByteOffset; ++i)
      pStr->write("\0", 1); // extra zeros for 4byte forced width
  }

  if (pStr->fail())
    throw SimpleException("<global>", "GuiSaveImage",
                          "Failed to write into the provided stream");
}