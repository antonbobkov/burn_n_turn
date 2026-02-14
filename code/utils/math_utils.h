/*
 * math_utils.h - Gui namespace: Gmin, Gmax, Gabs helpers.
 */

#ifndef GUI_MATH_UTILS_HEADER_ALREADY_DEFINED
#define GUI_MATH_UTILS_HEADER_ALREADY_DEFINED

/* Return the smaller of two values. */
template <class T> inline T Gmin(T a, T b) { return a < b ? a : b; }

/* Return the smallest of three values. */
template <class T> inline T Gmin(T a, T b, T c) {
  return Gmin(a, Gmin(b, c));
}

/* Return the larger of two values. */
template <class T> inline T Gmax(T a, T b) { return a > b ? a : b; }

/* Return the largest of three values. */
template <class T> inline T Gmax(T a, T b, T c) {
  return Gmax(a, Gmax(b, c));
}

/* Return the absolute value of a. */
template <class T> inline T Gabs(T a) { return a >= 0 ? a : -a; }

#endif // GUI_MATH_UTILS_HEADER_ALREADY_DEFINED
