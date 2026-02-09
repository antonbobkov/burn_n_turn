#ifndef GUIMOCK_ALREADY_INCLUDED
#define GUIMOCK_ALREADY_INCLUDED

#include <map>
#include <memory>
#include <string>

#include "GuiGen.h"

namespace Gui {

/* Fake image so GetImage can return a valid pointer. No real drawing.
 * GetPixelSafe returns default color when out of bounds (for small
 * placeholders). */
class MockImage : public Image {
public:
  explicit MockImage(Size sz);

  /*virtual*/ void SetPixel(Point p, const Color &c);
  /*virtual*/ Color GetPixel(Point p) const;
  /*virtual*/ void SetPixelSafe(Point p, const Color &c);
  /*virtual*/ Color GetPixelSafe(Point p) const;
};

/* Graphics backend that does no real drawing. LoadImage returns the path
 * string; other ops are no-op or use stored mock images. */
class MockGraphicalInterface : public GraphicalInterface<std::string> {
public:
  MockGraphicalInterface() : next_blank_id_(0) {}

  /*virtual*/ void DeleteImage(std::string pImg);
  /*virtual*/ Image *GetImage(std::string pImg) const;
  /*virtual*/ std::string GetBlankImage(Size sz);
  /*virtual*/ std::string LoadImage(std::string sFileName);
  /*virtual*/ void SaveImage(std::string sFileName, std::string pImg);
  /*virtual*/ void DrawImage(Point p, std::string pImg, Rectangle r,
                             bool bRefresh);
  /*virtual*/ void DrawRectangle(Rectangle p, Color c, bool bRedraw);
  /*virtual*/ void RefreshAll();

private:
  /* Handle string -> fake image. */
  std::map<std::string, std::unique_ptr<MockImage>> images_;
  /* Used to build unique keys for GetBlankImage. */
  unsigned next_blank_id_;
};

} // namespace Gui

#endif
