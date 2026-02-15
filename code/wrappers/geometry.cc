/*
 * geometry.cc - Definitions for Point, fPoint, Size, Rectangle and helpers.
 */

#include <cmath>
#include <cstdlib>
#include <sstream>

#include "geometry.h"
#include "utils/math_utils.h"

/* Point */
Point::Point() : x(0), y(0) {}
Point::Point(Crd x_, Crd y_) : x(x_), y(y_) {}

Point &Point::operator+=(const Point &p) {
  x += p.x;
  y += p.y;
  return *this;
}
Point &Point::operator-=(const Point &p) {
  x -= p.x;
  y -= p.y;
  return *this;
}
Point &Point::operator*=(Crd c) {
  x *= c;
  y *= c;
  return *this;
}

Point operator-(const Point &p) { return Point(-p.x, -p.y); }
Point operator+(const Point &p1, const Point &p2) { return Point(p1) += p2; }
Point operator-(const Point &p1, const Point &p2) { return Point(p1) -= p2; }
bool operator==(const Point &p1, const Point &p2) {
  return p1.x == p2.x && p1.y == p2.y;
}
bool operator!=(const Point &p1, const Point &p2) {
  return p1.x != p2.x || p1.y != p2.y;
}
Point operator*(const Point &p, Crd c) { return Point(p) *= c; }

std::ostream &operator<<(std::ostream &ofs, Point f) {
  return ofs << f.x << " " << f.y;
}
std::istream &operator>>(std::istream &ifs, Point &f) {
  return ifs >> f.x >> f.y;
}

/* fPoint */
fPoint::fPoint() : x(0), y(0) {}
fPoint::fPoint(double x_, double y_) : x(float(x_)), y(float(y_)) {}
fPoint::fPoint(Point p) : x(float(p.x)), y(float(p.y)) {}

Point fPoint::ToPnt() const { return Point(int(x), int(y)); }

void fPoint::Normalize(float f) {
  if (x == 0 && y == 0)
    return;
  (*this) *= f / Length();
}

float fPoint::Length() const { return sqrt(x * x + y * y); }

fPoint &operator+=(fPoint &f1, const fPoint &f2) {
  f1.x += f2.x;
  f1.y += f2.y;
  return f1;
}
fPoint &operator-=(fPoint &f1, const fPoint &f2) {
  f1.x -= f2.x;
  f1.y -= f2.y;
  return f1;
}
fPoint operator+(const fPoint &f1, const fPoint &f2) {
  fPoint f(f1);
  return f += f2;
}
fPoint operator-(const fPoint &f1, const fPoint &f2) {
  fPoint f(f1);
  return f -= f2;
}
bool operator==(const fPoint &f1, const fPoint &f2) {
  return (f1.x == f2.x) && (f1.y == f2.y);
}
bool operator!=(const fPoint &f1, const fPoint &f2) {
  return (f1.x != f2.x) || (f1.y != f2.y);
}
fPoint &operator/=(fPoint &f1, float f) {
  f1.x /= f;
  f1.y /= f;
  return f1;
}
fPoint &operator*=(fPoint &f1, float f) {
  f1.x *= f;
  f1.y *= f;
  return f1;
}
fPoint operator/(const fPoint &f1, float f) {
  fPoint ff(f1);
  return ff /= f;
}
fPoint operator*(const fPoint &f1, float f) {
  fPoint ff(f1);
  return ff *= f;
}
float Dot(const fPoint &f1, const fPoint &f2) {
  return f1.x * f2.x + f1.y * f2.y;
}

std::ostream &operator<<(std::ostream &ofs, fPoint f) {
  ofs << f.x << " " << f.y;
  return ofs;
}
std::istream &operator>>(std::istream &ifs, fPoint &f) {
  ifs >> f.x >> f.y;
  return ifs;
}

/* Polar */
Polar::Polar(fPoint p) : r(p.Length()) {
  if (p.y == 0 && p.x == 0)
    a = 0;
  else
    a = atan2(p.y, p.x);
}

fPoint ComposeDirection(int dir1, int dir2) {
  fPoint r(0, 0);
  switch (dir1) {
  case 1:
    r += fPoint(-1, 0);
    break;
  case 2:
    r += fPoint(1, 0);
    break;
  case 3:
    r += fPoint(0, 1);
    break;
  case 4:
    r += fPoint(0, -1);
    break;
  default:
    break;
  }
  switch (dir2) {
  case 1:
    r += fPoint(-1, 0);
    break;
  case 2:
    r += fPoint(1, 0);
    break;
  case 3:
    r += fPoint(0, 1);
    break;
  case 4:
    r += fPoint(0, -1);
    break;
  default:
    break;
  }
  r.Normalize();
  return r;
}

fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich,
                    unsigned nHowMany) {
  if (nHowMany == 1)
    return fDir;

  float d = 3.1415F * 2 * dWidth / (nHowMany - 1) * nWhich;

  return (Polar(fDir) * Polar(d - 3.1415F * dWidth, 1)).TofPoint();
}

fPoint RandomAngle(fPoint fDir, float fRange) {
  return (Polar(fDir) *
          Polar((float(rand()) / RAND_MAX - .5F) * fRange * 2 * 3.1415F, 1))
      .TofPoint();
}

/* Size */
Size::Size() : x(0), y(0) {}
Size::Size(Crd x_, Crd y_) : x(x_), y(y_) {}

Crd Size::Area() const { return x * y; }

bool operator==(const Size &p1, const Size &p2) {
  return p1.x == p2.x && p1.y == p2.y;
}
bool operator!=(const Size &p1, const Size &p2) {
  return p1.x != p2.x || p1.y != p2.y;
}

std::ostream &operator<<(std::ostream &ofs, Size s) {
  return ofs << s.x << " " << s.y;
}
std::istream &operator>>(std::istream &ifs, Size &s) {
  return ifs >> s.x >> s.y;
}

/* Rectangle */
Rectangle::Rectangle() : p(0, 0), sz(0, 0) {}
Rectangle::Rectangle(Size sz_) : p(Point(0, 0)), sz(sz_) { Normalize(); }
Rectangle::Rectangle(Point p_, Size sz_) : p(p_), sz(sz_) { Normalize(); }
Rectangle::Rectangle(Point p1, Point p2)
    : p(p1), sz(Size(p2.x - p1.x, p2.y - p1.y)) {
  Normalize();
}
Rectangle::Rectangle(Crd l, Crd t, Crd r, Crd b) : p(l, t), sz(r - l, b - t) {
  Normalize();
}

void Rectangle::Normalize() {
  if (sz.x < 0 || sz.y < 0)
    sz = Size(0, 0);
}

Crd Rectangle::Left() const { return p.x; }
Crd Rectangle::Right() const { return p.x + sz.x; }
Crd Rectangle::Top() const { return p.y; }
Crd Rectangle::Bottom() const { return p.y + sz.y; }
Point Rectangle::GetBottomRight() const {
  return Point(p.x + sz.x, p.y + sz.y);
}
Crd Rectangle::Area() const { return sz.Area(); }

bool operator==(const Rectangle &r1, const Rectangle &r2) {
  return r1.p == r2.p && r1.sz == r2.sz;
}
bool operator!=(const Rectangle &r1, const Rectangle &r2) {
  return !(r1 == r2);
}

std::ostream &operator<<(std::ostream &ofs, Rectangle r) {
  return ofs << r.p.x << " " << r.p.y << " " << r.sz.x << " " << r.sz.y;
}
std::istream &operator>>(std::istream &ifs, Rectangle &r) {
  return ifs >> r.p.x >> r.p.y >> r.sz.x >> r.sz.y;
}

bool InsideRectangle(Rectangle r, Point p) {
  return r.p.x <= p.x && p.x < r.p.x + r.sz.x && r.p.y <= p.y &&
         p.y < r.p.y + r.sz.y;
}

Point Center(Rectangle r) {
  return Point(r.p.x + r.sz.x / 2, r.p.y + r.sz.y / 2);
}

Point RandomPnt(Rectangle r) {
  return Point(r.p.x + rand() % (r.sz.x), r.p.y + rand() % (r.sz.y));
}

Rectangle operator+(const Rectangle &r, const Point &p) {
  return Rectangle(r.p + p, r.sz);
}

Rectangle operator+(const Rectangle &r1, const Rectangle &r2) {
  return Rectangle(r1.p + r2.p, Size(r1.sz.x + r2.sz.x, r1.sz.y + r2.sz.y));
}

Rectangle Intersect(const Rectangle &r1, const Rectangle &r2) {
  Crd l = Gmax(r1.Left(), r2.Left());
  Crd r = Gmin(r1.Right(), r2.Right());
  Crd t = Gmax(r1.Top(), r2.Top());
  Crd b = Gmin(r1.Bottom(), r2.Bottom());
  return Rectangle(l, t, r, b);
}

std::string RectangleToString(Rectangle r) {
  std::ostringstream ostr;
  ostr << "(" << r.p.x << ", " << r.p.y << ", " << r.sz.x << ", " << r.sz.y
       << ")";
  return ostr.str();
}
