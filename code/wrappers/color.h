/*
 * color.h - Gui namespace: Color struct and comparison/string helpers.
 */

#ifndef GUI_COLOR_HEADER_ALREADY_DEFINED
#define GUI_COLOR_HEADER_ALREADY_DEFINED

#include <string>

/* RGBA-style color: B, G, R bytes and transparency (0 = fully transparent).
 */
struct Color {
  unsigned char B, G, R;
  unsigned char nTransparent; // 0 = fully transparent

  Color();
  Color(unsigned char R_, unsigned char G_, unsigned char B_,
        unsigned char nTransparent_ = 255);
  Color(int R_, int G_, int B_, int nTransparent_ = 255);
};

// two fully transparent colors are equal; otherwise compare color components
bool operator==(const Color &c1, const Color &c2);
bool operator<(const Color &c1, const Color &c2);

std::string ColorToString(Color c);

#endif
