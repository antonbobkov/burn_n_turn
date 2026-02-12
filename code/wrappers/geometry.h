/*
 * geometry.h - Gui namespace: Point, fPoint, Size, Rectangle and helpers.
 * Declarations only; definitions in geometry.cc.
 */

#ifndef GUI_GEOMETRY_HEADER_ALREADY_DEFINED
#define GUI_GEOMETRY_HEADER_ALREADY_DEFINED

#include <iostream>
#include <string>

namespace Gui {
typedef int Crd;

/* 2D integer point (Crd x, y) with +=, -=, *= and free operators +, -, *, ==,
 * !=. */
struct Point {
  Crd x, y;

  Point();
  Point(Crd x_, Crd y_);

  Point &operator+=(const Point &p);
  Point &operator-=(const Point &p);
  Point &operator*=(Crd c);
};

Point operator-(const Point &p);
Point operator+(const Point &p1, const Point &p2);
Point operator-(const Point &p1, const Point &p2);
bool operator==(const Point &p1, const Point &p2);
bool operator!=(const Point &p1, const Point &p2);
Point operator*(const Point &p, Crd c);

std::ostream &operator<<(std::ostream &ofs, Point f);
std::istream &operator>>(std::istream &ifs, Point &f);

/* 2D float point; Normalize(), Length(), conversion to/from Point. */
struct fPoint {
  float x, y;

  fPoint();
  fPoint(double x_, double y_);
  fPoint(Point p);

  Point ToPnt() const;
  void Normalize(float f = 1);
  float Length() const;
};

fPoint &operator+=(fPoint &f1, const fPoint &f2);
fPoint &operator-=(fPoint &f1, const fPoint &f2);
fPoint operator+(const fPoint &f1, const fPoint &f2);
fPoint operator-(const fPoint &f1, const fPoint &f2);
bool operator==(const fPoint &f1, const fPoint &f2);
bool operator!=(const fPoint &f1, const fPoint &f2);
fPoint &operator/=(fPoint &f1, float f);
fPoint &operator*=(fPoint &f1, float f);
fPoint operator/(const fPoint &f1, float f);
fPoint operator*(const fPoint &f1, float f);
float Dot(const fPoint &f1, const fPoint &f2);

std::ostream &operator<<(std::ostream &ofs, fPoint f);
std::istream &operator>>(std::istream &ifs, fPoint &f);

/* 2D extent (Crd x, y); Area() = x * y. */
struct Size {
  Crd x, y;

  Size();
  Size(Crd x_, Crd y_);

  Crd Area() const;
};

bool operator==(const Size &p1, const Size &p2);
bool operator!=(const Size &p1, const Size &p2);

std::ostream &operator<<(std::ostream &ofs, Size s);
std::istream &operator>>(std::istream &ifs, Size &s);

/* Axis-aligned rectangle (Point p, Size sz); normalizes to non-negative size.
 */
struct Rectangle {
  Point p;
  Size sz;

  Rectangle();
  Rectangle(Size sz_);
  Rectangle(Point p_, Size sz_);
  Rectangle(Point p1, Point p2);
  Rectangle(Crd l, Crd t, Crd r, Crd b);

  void Normalize();
  Crd Left() const;
  Crd Right() const;
  Crd Top() const;
  Crd Bottom() const;
  Point GetBottomRight() const;
  Crd Area() const;
};

bool operator==(const Rectangle &r1, const Rectangle &r2);
bool operator!=(const Rectangle &r1, const Rectangle &r2);

std::ostream &operator<<(std::ostream &ofs, Rectangle r);
std::istream &operator>>(std::istream &ifs, Rectangle &r);

bool InsideRectangle(Rectangle r, Point p);
Point Center(Rectangle r);
Point RandomPnt(Rectangle r);
Rectangle operator+(const Rectangle &r, const Point &p);
Rectangle operator+(const Rectangle &r1, const Rectangle &r2);
Rectangle Intersect(const Rectangle &r1, const Rectangle &r2);
std::string RectangleToString(Rectangle r);
} // namespace Gui

#endif // GUI_GEOMETRY_HEADER_ALREADY_DEFINED
