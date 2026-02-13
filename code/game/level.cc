#include "level.h"
#include "game.h"
#include "controller/level_controller.h"
#include "smart_pointer.h"


#include "file_utils.h"

void Road::Draw(smart_pointer<ScalingDrawer> pDr) {
  unsigned n = pDr->nFactor;
  if (bVertical)
    pDr->pGr->DrawRectangle(Rectangle((nCoord - 5) * n, rBound.p.y * n,
                                      (nCoord + 5) * n, rBound.sz.y * n),
                            Color(63, 63, 63), false);
  else
    pDr->pGr->DrawRectangle(Rectangle(rBound.p.x * n, (nCoord - 5) * n,
                                      rBound.sz.x * n, (nCoord + 5) * n),
                            Color(63, 63, 63), false);
}

void Road::RoadMap(Point &p, Point &v) {
  if (rand() % 2) {
    if (!bVertical) {
      p.y = nCoord - 7;
      p.x = rBound.p.x;
      v = Point(1, 0);
    } else {
      p.x = nCoord;
      p.y = rBound.p.y;
      v = Point(0, 1);
    }
  } else {
    if (!bVertical) {
      p.y = nCoord - 7;
      p.x = rBound.sz.x - 1;
      v = Point(-1, 0);
    } else {
      p.x = nCoord;
      p.y = rBound.sz.y - 1;
      v = Point(0, -1);
    }
  }
}

void BrokenLine::CloseLast() {
  if (vEdges.empty() || vEdges.back().empty())
    throw SegmentSimpleException("CloseLast", "Invalid (empty) segment arrays");

  if (vEdges.back().front() != vEdges.back().back())
    vEdges.back().push_back(vEdges.back().front());
}

void BrokenLine::Add(fPoint p) {
  if (vEdges.empty())
    vEdges.push_back(VecLine());
  vEdges.back().push_back(p);
}

void BrokenLine::Join(const BrokenLine &b) {
  for (unsigned i = 0; i < b.vEdges.size(); ++i)
    vEdges.push_back(b.vEdges[i]);
}

BrokenLine::BrokenLine(fPoint p1, fPoint p2) {
  Add(p1);
  Add(p2);
}

BrokenLine::BrokenLine(fPoint p1, fPoint p2, fPoint p3) {
  Add(p1);
  Add(p2);
  Add(p3);
}

BrokenLine::BrokenLine(fPoint p1, fPoint p2, fPoint p3, fPoint p4) {
  Add(p1);
  Add(p2);
  Add(p3);
  Add(p4);
}

BrokenLine::BrokenLine(Rectangle r) {
  Add(r.p);
  Add(fPoint(float(r.Left()), float(r.Bottom())));
  Add(fPoint(float(r.Right()), float(r.Top())));
  Add(r.GetBottomRight());
  CloseLast();
}

fPoint BrokenLine::RandomByLength() {
  if (vEdges.empty())
    throw SegmentSimpleException("RandomByLength",
                                 "Invalid (empty) segment arrays");

  float fLength = 0;
  for (unsigned i = 0; i < vEdges.size(); ++i) {
    if (vEdges[i].empty())
      throw SegmentSimpleException("RandomByLength",
                                   "Invalid (empty) segment arrays");
    if (vEdges[i].size() == 1)
      continue;
    for (unsigned j = 1; j < vEdges[i].size(); ++j)
      fLength += (vEdges[i][j] - vEdges[i][j - 1]).Length();
  }

  if (fLength == 0)
    return RandomBySegment();

  fLength = fLength * rand() / RAND_MAX;

  for (unsigned i = 0; i < vEdges.size(); ++i) {
    if (vEdges[i].size() == 1)
      continue;

    for (unsigned j = 1; j < vEdges[i].size(); ++j) {
      fPoint f = vEdges[i][j] - vEdges[i][j - 1];
      if (fLength <= f.Length()) {
        f.Normalize(fLength);
        return vEdges[i][j - 1] + f;
      }

      fLength -= f.Length();
    }
  }

  return vEdges[0][0];
}

fPoint BrokenLine::RandomBySegment() {
  if (vEdges.empty())
    throw SegmentSimpleException("RandomBySegment",
                                 "Invalid (empty) segment arrays");

  unsigned nSegment = rand() % vEdges.size();

  if (vEdges[nSegment].size() == 0)
    throw SegmentSimpleException("RandomBySegment",
                                 "Invalid (empty) segment arrays");
  if (vEdges[nSegment].size() == 1)
    return vEdges[nSegment][0];

  unsigned nSegment2 = rand() % (vEdges[nSegment].size() - 1);

  fPoint f = vEdges[nSegment][nSegment2 + 1] - vEdges[nSegment][nSegment2];
  f.Normalize(f.Length() * rand() / RAND_MAX);
  return vEdges[nSegment][nSegment2] + f;
}

std::ostream &operator<<(std::ostream &ofs, const BrokenLine &bl) {
  for (unsigned i = 0; i < bl.vEdges.size(); ++i) {
    for (unsigned j = 0; j < bl.vEdges[i].size(); ++j)
      ofs << bl.vEdges[i][j] << " ";
    ofs << "| ";
  }
  ofs << "& ";
  return ofs;
}

std::istream &operator>>(std::istream &ifs, BrokenLine &bl) {
  bl.vEdges.clear();

  std::string str;

  std::getline(ifs, str, '&');
  ifs.get();

  std::istringstream istr(str);

  while (true) {
    std::string part;
    std::getline(istr, part, '|');

    if (istr.fail())
      break;

    istr.get();

    BrokenLine bPart;

    std::istringstream is(part);
    fPoint p;
    while (is >> p)
      bPart.Add(p);

    bl.Join(bPart);
  }

  return ifs;
}

void LevelLayout::Convert(int n) {
  float p1 = float(sBound.sz.x) / n;
  float p2 = float(sBound.sz.y) / n;

  unsigned i, j;

  for (i = 0; i < blKnightGen.vEdges.size(); ++i)
    for (j = 0; j < blKnightGen.vEdges[i].size(); ++j) {
      blKnightGen.vEdges[i][j].x *= p1;
      blKnightGen.vEdges[i][j].y *= p2;
    }

  for (i = 0; i < vCastleLoc.size(); ++i) {
    vCastleLoc[i].x = Crd(vCastleLoc[i].x * p1);
    vCastleLoc[i].y = Crd(vCastleLoc[i].y * p2);
  }

  for (i = 0; i < vRoadGen.size(); ++i)
    if (vRoadGen[i].bVertical)
      vRoadGen[i].nCoord = Crd(vRoadGen[i].nCoord * p1);
    else
      vRoadGen[i].nCoord = Crd(vRoadGen[i].nCoord * p2);
}

std::ostream &operator<<(std::ostream &ofs, const LevelLayout &f) {
  ofs << "LEVEL " << f.nLvl << "\n\n";

  ofs << "FREQ ";
  for (unsigned k = 0; k < f.vFreq.size(); ++k)
    ofs << f.vFreq[k] / nFramesInSecond << " ";
  ofs << "\n";

  ofs << "SPWN " << f.blKnightGen << "\n";

  ofs << "CSTL ";
  for (unsigned i = 0; i < f.vCastleLoc.size(); ++i)
    ofs << f.vCastleLoc[i] << " ";
  ofs << "\n";

  ofs << "ROAD ";
  for (unsigned j = 0; j < f.vRoadGen.size(); ++j)
    ofs << f.vRoadGen[j] << " ";
  ofs << "\n";

  ofs << "TIME " << f.nTimer << "\n";

  ofs << "\n";

  return ofs;
}

std::istream &operator>>(std::istream &ifs, LevelLayout &f) {
  f = LevelLayout(f.sBound);

  ParsePosition("LEVEL", ifs);

  ifs >> f.nLvl;

  {
    std::string str;
    ParseGrabLine("FREQ", ifs, str);
    std::istringstream istr(str);

    float n;
    while (istr >> n) {
      n *= nFramesInSecond;

      f.vFreq.push_back(n);
    }
  }

  {
    std::string str;
    ParseGrabLine("SPWN", ifs, str);
    std::istringstream istr(str);

    istr >> f.blKnightGen;
  }

  {
    std::string str;
    ParseGrabLine("CSTL", ifs, str);
    std::istringstream istr(str);

    Point p;
    while (istr >> p)
      f.vCastleLoc.push_back(p);
  }

  {
    std::string str;
    ParseGrabLine("ROAD", ifs, str);
    std::istringstream istr(str);

    Road r(0, 0, f.sBound);
    while (istr >> r)
      f.vRoadGen.push_back(r);
  }

  {
    std::string str;
    ParseGrabLine("TIME", ifs, str);
    std::istringstream istr(str);

    istr >> f.nTimer;

    f.nTimer *= nFramesInSecond;
  }

  return ifs;
}

void FancyRoad::Draw(smart_pointer<ScalingDrawer> pDr) {
  unsigned n = pDr->nFactor;
  Image *p = pDr->pGr->GetImage(pAd->pGl->pr["road"]);
  Size sz = p->GetSize();

  if (bVertical)
    for (int i = 0; (i - 1) * sz.y < rBound.sz.y * int(n); ++i)
      pDr->pGr->DrawImage(Point(nCoord * n - sz.x / 2, i * sz.y),
                          pAd->pGl->pr["road"], false);
  else
    for (int i = 0; (i - 1) * sz.x < rBound.sz.x * int(n); ++i)
      pDr->pGr->DrawImage(Point(i * sz.x, nCoord * n - sz.y / 2),
                          pAd->pGl->pr["road"], false);
}

void ReadLevels(Gui::FilePath *fp, std::string sFile, Rectangle rBound,
                LevelStorage &vLvl) {
  std::string path = fp->GetRelativePath(sFile);
  std::unique_ptr<Gui::InStreamHandler> ih = fp->ReadFile(path);
  std::istream &ifs = ih->GetStream();

  LevelLayout l(rBound);
  while (ifs >> l) {
    l.Convert();
    vLvl.push_back(l);
  }

  if (vLvl.size() == 0)
    throw SimpleException("<global>", "ReadLevels",
                          "Cannot read levels at " + sFile);
}
