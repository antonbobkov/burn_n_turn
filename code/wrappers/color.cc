#include <sstream>

#include "color.h"

namespace Gui {

Color::Color()
    : R(0), G(0), B(0), nTransparent(255) {}

Color::Color(unsigned char R_, unsigned char G_, unsigned char B_,
             unsigned char nTransparent_)
    : R(R_), G(G_), B(B_), nTransparent(nTransparent_) {}

Color::Color(int R_, int G_, int B_, int nTransparent_)
    : R(static_cast<unsigned char>(R_)),
      G(static_cast<unsigned char>(G_)),
      B(static_cast<unsigned char>(B_)),
      nTransparent(static_cast<unsigned char>(nTransparent_)) {}

bool operator==(const Color &c1, const Color &c2) {
  if (c1.nTransparent == 0 && c2.nTransparent == 0)
    return true;
  if (c1.nTransparent != 0 && c2.nTransparent == 0)
    return false;
  if (c1.nTransparent == 0 && c2.nTransparent != 0)
    return false;
  return (c1.B == c2.B) && (c1.G == c2.G) && (c1.R == c2.R);
}

bool operator<(const Color &c1, const Color &c2) {
  if (c1 == c2)
    return false;

  if (c1.R < c2.R)
    return true;
  if (c1.R > c2.R)
    return false;
  if (c1.G < c2.G)
    return true;
  if (c1.G > c2.G)
    return false;
  if (c1.B < c2.B)
    return true;
  return false;
}

std::string ColorToString(Color c) {
  std::ostringstream ostr;
  ostr << "(" << c.R << ", " << c.G << ", " << c.B << ")";
  return ostr.str();
}

} // namespace Gui
