/*
 * random_utils.cc - GetRandNum, GetRandFromDistribution implementation.
 */

#include <cstdlib>
#include <string>

#include "random_utils.h"

int GetRandNum(int nRange) {
  return int(float(rand()) / (float(RAND_MAX) + 1) * nRange);
}

int GetRandFromDistribution(std::vector<float> vProb) {
  float fSum = 0;

  for (int i = 0; i < (int)vProb.size(); ++i)
    fSum += vProb[i];

  float fRand = float(rand()) / (float(RAND_MAX) + 1) * fSum;

  fSum = 0;

  for (int i = 0; i < (int)vProb.size(); ++i) {
    fSum += vProb[i];
    if (fSum > fRand)
      return i;
  }

  throw std::string("bad range");
}
