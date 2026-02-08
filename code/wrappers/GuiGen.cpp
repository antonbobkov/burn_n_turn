#include <sstream>

#include "GuiGen.h"

#include "General.h"

namespace Gui {
void fPoint::Normalize(float f) {
  if (x == 0 && y == 0)
    return;

  (*this) *= f / Length();
}

bool InsideRectangle(Rectangle r, Point p) // is the point inside the rectangle?
{
  return r.p.x <= p.x && p.x < r.p.x + r.sz.x && r.p.y <= p.y &&
         p.y < r.p.y + r.sz.y;
}

Point Center(Rectangle r) {
  return Point(r.p.x + r.sz.x / 2, r.p.y + r.sz.y / 2);
}

Point RandomPnt(Rectangle r) {
  return Point(r.p.x + rand() % (r.sz.x), r.p.y + rand() % (r.sz.y));
}

Rectangle operator+(const Rectangle &r, const Point &p) // displace
{
  return Rectangle(r.p + p, r.sz);
}

Rectangle operator+(const Rectangle &r1, const Rectangle &r2) {
  return Rectangle(r1.p + r2.p, Size(r1.sz.x + r2.sz.x, r1.sz.y + r2.sz.y));
}

Rectangle
Intersect(const Rectangle &r1,
          const Rectangle &r2) // return intersection of two rectangles
{
  Crd l = Gmax(r1.Left(), r2.Left());
  Crd r = Gmin(r1.Right(), r2.Right());
  Crd t = Gmax(r1.Top(), r2.Top());
  Crd b = Gmin(r1.Bottom(), r2.Bottom());

  return Rectangle(l, t, r, b);
}

std::string ColorToString(Color c) {
  std::ostringstream ostr;
  ostr << "(" << c.R << ", " << c.G << ", " << c.B
       << ")"; // Looks like (255, 0, 77)
  return ostr.str();
}

std::string RectangleToString(Rectangle r) {
  std::ostringstream ostr;
  ostr << "(" << r.p.x << ", " << r.p.y << ", " << r.sz.x << ", " << r.sz.y
       << ")"; // Looks like (10, 0, 5, 5)
  return ostr.str();
}

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

FontWriter::FontWriter(FilePath &fp, std::string sFont,
                       SP<GraphicalInterface<Index>> pGr_, unsigned nZoom)
    : vImgIndx(256, -1), pGr(pGr_) {
  sFont = fp.GetRelativePath(sFont);

  std::string sFontPath;
  std::string sFontName = sFont;

  Separate(sFontName, sFontPath);

  std::ifstream ifs(sFont.c_str());

  std::string sFontPicture;
  std::getline(ifs, sFontPicture);

  sFontPicture = fp.Format(sFontPicture);

  Color cTransp;
  int na, nb, nc;

  ifs >> na >> nb >> nc;
  cTransp.R = na;
  cTransp.G = nb;
  cTransp.B = nc;
  ifs >> na >> nb >> nc;
  clSymbol.R = na;
  clSymbol.G = nb;
  clSymbol.B = nc;

  ifs >> szSymbol.x >> szSymbol.y >> nGap;

  while (ifs.get() != '\n' && !ifs.fail())
    ;

  szSymbol.x *= nZoom;
  szSymbol.y *= nZoom;
  nGap *= nZoom;

  unsigned n;
  unsigned char c;
  for (n = 0; c = ifs.get(), !ifs.fail(); ++n)
    vImgIndx[c] = n;

  Index nImg = pGr->LoadImage(sFontPath + sFontPicture);
  nImg = pGr->ScaleImage(nImg, nZoom);
  Image *pImg = pGr->GetImage(nImg);
  pImg->ChangeColor(cTransp, Color(0, 0, 0, 0));

  for (unsigned i = 0; i < n; ++i) {
    Index nCurr = pGr->GetBlankImage(szSymbol);
    Image *pCurr = pGr->GetImage(nCurr);

    Point p;
    for (p.y = 0; p.y < szSymbol.y; ++p.y)
      for (p.x = 0; p.x < szSymbol.x; ++p.x)
        pCurr->SetPixel(
            p, pImg->GetPixelSafe(Point(i * (szSymbol.x + nGap) + p.x, p.y)));
    vImg.push_back(nCurr);
  }
}

std::string FontWriter::GetNumber(int n, unsigned nDigits /* = 0*/) {
  std::string s;
  std::ostringstream ostr(s);
  ostr << n;
  std::string sRes = ostr.str();
  if (sRes.length() >= nDigits)
    return sRes;
  return std::string(nDigits - sRes.length(), '0') + sRes;
}

Size FontWriter::GetSize(std::string s) {
  if (s.length() == 0)
    return Size(0, szSymbol.y);

  return Size(szSymbol.x * s.length() + nGap * (s.length() - 1), szSymbol.y);
}

void FontWriter::DrawColorWord(std::string s, Point p,
                               Color c /* = Color(0,0,0,0)*/,
                               bool bCenter /* = false*/,
                               bool bRefresh /* = false*/) {
  if (bCenter) {
    Size szWord = GetSize(s);
    p.x -= szWord.x / 2;
    p.y -= szWord.y / 2;
  }

  for (unsigned i = 0; i < s.length(); ++i) {
    int n = int(s[i]);

    if (vImgIndx[n] == -1)
      continue;

    Point pnt(p.x + (szSymbol.x + nGap) * i, p.y);
    if (c.nTransparent == 0)
      pGr->DrawImage(pnt, vImg[vImgIndx[n]], bRefresh);
    else {
      Index vColImg = pGr->CopyImage(vImg[vImgIndx[n]]);
      pGr->GetImage(vColImg)->ChangeColor(clSymbol, c);
      pGr->DrawImage(pnt, vColImg, bRefresh);
    }
  }
}

void FontWriter::Recolor(Color c) {
  for (unsigned i = 0; i < vImg.size(); ++i)
    pGr->GetImage(vImg[i])->ChangeColor(clSymbol, c);
  clSymbol = c;
}

void FontWriter::SetGap(int nG) { nGap = nG; }
} // namespace Gui