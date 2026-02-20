#ifndef TOWER_DEFENSE_LEVEL_H
#define TOWER_DEFENSE_LEVEL_H

#include "game_utils/draw_utils.h"
#include "entities.h"
#include "utils/exception.h"
#include "utils/smart_pointer.h"

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
struct BrokenLine {
  typedef std::vector<fPoint> VecLine;
  typedef std::vector<VecLine> VecLines;

  VecLines vEdges;

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
};

std::ostream &operator<<(std::ostream &ofs, const BrokenLine &bl);
std::istream &operator>>(std::istream &ifs, BrokenLine &bl);

/** A stretch of road in the chapter: vertical or horizontal, with coordinate
 * and bounds; Draw paints the gray bar. */
struct Road : virtual public VisualEntity {
  std::string get_class_name() override { return "Road"; }
  bool bVertical;
  unsigned nCoord;
  Rectangle rBound;

  Road(bool bVertical_, unsigned nCoord_, Rectangle rBound_)
      : bVertical(bVertical_), nCoord(nCoord_), rBound(rBound_) {}
  Road(const Road &) = default;
  Road &operator=(const Road &) = default;
  Road &operator=(Road &&) = delete;

  /*virtual*/ float GetPriority() { return 0; }
  /*virtual*/ Point GetPosition() { return Point(); }

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  void RoadMap(Point &p, Point &v);
};

inline std::ostream &operator<<(std::ostream &ofs, const Road &r) {
  return ofs << r.bVertical << " " << r.nCoord << " ";
}

inline std::istream &operator>>(std::istream &ifs, Road &r) {
  return ifs >> r.bVertical >> r.nCoord;
}

struct LevelController;

/** A road with a tiled image; needs the level keeper for its art. */
struct FancyRoad : public Road {
  FancyRoad(const Road &rd, LevelController *pAd_) : Road(rd), pAd(pAd_) {}
  LevelController *pAd;

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

/** One chapter: bounds, where knights appear, castle spots, roads, timer, and
 * spawn rates; Convert scales the realm's measure. */
struct LevelLayout {
  Rectangle sBound;

  unsigned nLvl;

  LevelLayout(Rectangle sBound_) : sBound(sBound_) {}

  BrokenLine blKnightGen;
  std::vector<Point> vCastleLoc;
  std::vector<Road> vRoadGen;
  unsigned nTimer;

  std::vector<float> vFreq;

  void Convert(int n = 24);
};

void ReadLevels(FilePath *fp, std::string sFile, Rectangle rBound,
                std::vector<LevelLayout> &vLvl);

std::ostream &operator<<(std::ostream &ofs, const LevelLayout &f);
std::istream &operator>>(std::istream &ifs, LevelLayout &f);

#endif
