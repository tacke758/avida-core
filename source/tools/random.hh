#ifndef RANDOM_HH
#define RANDOM_HH

#include <time.h>

#define MBIG 1000000000
#define MSEED 161803398
#define FAC (1.0/MBIG)

class cRandom {
private:
  int seed;

  int inext;
  int inextp;
  int ma[56];

  void Init(int in_seed);
public:
  cRandom(int in_seed = 0);

  inline double GetDouble() {
    if (++inext == 56) inext = 1;
    if (++inextp == 56) inextp = 1;
    int mj = ma[inext] - ma[inextp];
    if (mj < 0) mj += MBIG;
    ma[inext] = mj;

    return mj * FAC;
  }
  inline double GetDouble(double max) { return GetDouble() * max; }
  inline double GetDouble(double min, double max)
    { return GetDouble() * (max - min) + min; }

  inline void ResetSeed(int new_seed) {
    seed = new_seed;
    Init(new_seed);
  }
  
  inline unsigned int GetUInt(unsigned int max) {
    return (int) (GetDouble() * max);
  }
  inline unsigned int GetUInt(unsigned int min, unsigned int max)
    { return GetUInt(max - min) + min; }

  inline int GetInt(int min, int max)
    { return ((int) GetUInt(max - min)) + min; }

  inline int GetOriginalSeed() { return seed; }
};

#endif
