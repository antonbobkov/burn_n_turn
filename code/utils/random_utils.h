/*
 * random_utils.h - Random number helpers.
 */

#ifndef RANDOM_UTILS_HEADER_ALREADY_DEFINED
#define RANDOM_UTILS_HEADER_ALREADY_DEFINED

#include <vector>

/* Time interval from rate (randomized). */
inline int GetRandTimeFromRate(float fRate) {
  return 1 + int((float(rand()) / RAND_MAX * 1.5 + .25) * fRate);
}

unsigned GetRandNum(unsigned nRange);
unsigned GetRandFromDistribution(std::vector<float> vProb);

#endif // RANDOM_UTILS_HEADER_ALREADY_DEFINED
