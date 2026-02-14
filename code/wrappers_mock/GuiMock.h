#ifndef GUIMOCK_ALREADY_INCLUDED
#define GUIMOCK_ALREADY_INCLUDED

#include <map>
#include <memory>
#include <string>

#include "wrappers/GuiGen.h"
#include "wrappers/color.h"


/* Fake image so GetImage can return a valid pointer. No real drawing.
 * GetPixelSafe returns default color when out of bounds (for small
 * placeholders). */
class MockImage : public Image {
public:
  explicit MockImage(std::string name, Size sz);

  /*virtual*/ void SetPixel(Point p, const Color &c);
  /*virtual*/ Color GetPixel(Point p) const;
  /*virtual*/ void SetPixelSafe(Point p, const Color &c);
  /*virtual*/ Color GetPixelSafe(Point p) const;

private:
  std::string name_;
};

/* Graphics backend that does no real drawing. LoadImage and GetBlankImage
 * return unique keys (suffix _0, _1, ...). Never reuses the same object. */
class MockGraphicalInterface : public GraphicalInterface<std::string> {
public:
  std::string get_class_name() override { return "MockGraphicalInterface"; }
  MockGraphicalInterface() : next_id_(0) {}

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
  /* Unique suffix for LoadImage and GetBlankImage keys (_0, _1, ...). */
  unsigned next_id_;
};

#endif
