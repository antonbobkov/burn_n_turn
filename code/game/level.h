#ifndef TOWER_DEFENSE_LEVEL_H
#define TOWER_DEFENSE_LEVEL_H

#include "../game_utils/draw_utils.h"
#include "entities.h"
#include "../utils/exception.h"

class FilePath;

/** Thrown when a path segment is wrong (e.g. invalid or empty). */
class SegmentSimpleException : public MyException {
  std::string strProblem;

public:
  SegmentSimpleException(crefString strFnName_, crefString strProblem_)
      : MyException("SegmentSimpleException", "BrokenLine", strFnName_),
        strProblem(strProblem_) {}

  /*virtual*/ std::string GetErrorMessage() const { return strProblem; }
};

/** A path made of segments: vEdges holds the point sequences; CloseLast,
 * Add, AddLine, Join, and stream read/write. */
class BrokenLine {
public:
  typedef std::vector<fPoint> VecLine;
  typedef std::vector<VecLine> VecLines;

  void CloseLast();

  void Add(fPoint p);

  void AddLine(const VecLine &vl) { vEdges.push_back(vl); }

  void Join(const BrokenLine &b);

  BrokenLine() {}
  BrokenLine(fPoint p1) { Add(p1); }
  BrokenLine(fPoint p1, fPoint p2);
  BrokenLine(fPoint p1, fPoint p2, fPoint p3);
  BrokenLine(fPoint p1, fPoint p2, fPoint p3, fPoint p4);
  BrokenLine(Rectangle r);

  fPoint RandomByLength();
  fPoint RandomBySegment();

  /** Scale all points by (sx, sy). */
  void Scale(float sx, float sy);

  friend std::ostream &operator<<(std::ostream &, const BrokenLine &);
  friend std::istream &operator>>(std::istream &, BrokenLine &);

private:
  VecLines vEdges;
};

std::ostream &operator<<(std::ostream &ofs, const BrokenLine &bl);
std::istream &operator>>(std::istream &ifs, BrokenLine &bl);

/** A stretch of road in the chapter: vertical or horizontal, with coordinate
 * and bounds; Draw paints the gray bar. */
class Road : public Entity {
public:
  std::string get_class_name() override { return "Road"; }
  bool ShouldDraw() override { return true; }

  Road(bool bVertical_, int nCoord_, Rectangle rBound_)
      : bVertical(bVertical_), nCoord(nCoord_), rBound(rBound_) {}
  Road(const Road &) = default;
  Road &operator=(const Road &) = default;

  float GetPriority() override { return 0; }
  Point GetPosition() override { return Point(); }

  void Draw(ScalingDrawer *pDr) override;
  void RoadMap(Point &p, Point &v);

  bool IsVertical() const { return bVertical; }
  int GetCoord() const { return nCoord; }
  Rectangle GetBound() const { return rBound; }
  /** Scale the coordinate by s (used when converting level units). */
  void ScaleCoord(float s);

private:
  bool bVertical;
  int nCoord;
  Rectangle rBound;
};

inline std::ostream &operator<<(std::ostream &ofs, const Road &r) {
  return ofs << r.IsVertical() << " " << r.GetCoord() << " ";
}

inline std::istream &operator>>(std::istream &ifs, Road &r) {
  bool bV; int nC;
  ifs >> bV >> nC;
  r = Road(bV, nC, r.GetBound());
  return ifs;
}

class LevelController;

/** A road with a tiled image; needs the level keeper for its art. */
class FancyRoad : public Road {
public:
  FancyRoad(const Road &rd, LevelController *pAd_) : Road(rd), pAd(pAd_) {}

  /*virtual*/ void Draw(ScalingDrawer *pDr);

private:
  LevelController *pAd;
};

/** One chapter: bounds, where knights appear, castle spots, roads, timer, and
 * spawn rates; Convert scales the realm's measure. */
class LevelLayout {
public:
  LevelLayout(Rectangle sBound_) : sBound(sBound_) {}

  void Convert(int n = 24);

  int GetLevel() const { return nLvl; }
  const BrokenLine &GetKnightPath() const { return blKnightGen; }
  const std::vector<Point> &GetCastleLocations() const { return vCastleLoc; }
  const std::vector<Road> &GetRoads() const { return vRoadGen; }
  int GetTime() const { return nTimer; }
  float GetFreq(int i) const { return vFreq.at(i); }

  friend std::ostream &operator<<(std::ostream &, const LevelLayout &);
  friend std::istream &operator>>(std::istream &, LevelLayout &);

private:
  Rectangle sBound;
  int nLvl;
  BrokenLine blKnightGen;
  std::vector<Point> vCastleLoc;
  std::vector<Road> vRoadGen;
  int nTimer;
  std::vector<float> vFreq;
};

void ReadLevels(FilePath *fp, std::string sFile, Rectangle rBound,
                std::vector<LevelLayout> &vLvl);

std::ostream &operator<<(std::ostream &ofs, const LevelLayout &f);
std::istream &operator>>(std::istream &ifs, LevelLayout &f);

#endif
