/*
 * random_utils.cc - GetRandNum, GetRandFromDistribution implementation.
 */

#include <cstdlib>
#include <string>

#include "utils/random_utils.h"

unsigned GetRandNum(unsigned nRange) {
  return unsigned(float(rand()) / (float(RAND_MAX) + 1) * nRange);
}

unsigned GetRandFromDistribution(std::vector<float> vProb) {
  float fSum = 0;

  for (unsigned i = 0; i < vProb.size(); ++i)
    fSum += vProb[i];

  float fRand = float(rand()) / (float(RAND_MAX) + 1) * fSum;

  fSum = 0;

  for (unsigned i = 0; i < vProb.size(); ++i) {
    fSum += vProb[i];
    if (fSum > fRand)
      return i;
  }

  throw std::string("bad range");
}
