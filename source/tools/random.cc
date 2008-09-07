#include "../defs.hh"
#include "random.hh"

#ifdef MSVC_COMPILER
#include <process.h>
#else
#include <unistd.h>
#endif

cRandom::cRandom(int in_seed)
{
  Init(in_seed);
}

void cRandom::Init(int in_seed)
{
  int mj, mk, ii;
  seed = in_seed;

  // If seed is <= 0, reset it to time * pid.

  if (seed <= 0) {
    int seed_time = (int) time(NULL);

#ifdef MSVC_COMPILER
    int seed_pid = (int) _getpid(); 
#else
    int seed_pid = (int) getpid(); 
#endif
    seed = seed_time ^ (seed_pid << 8);
  }

  // Convert tmp_seed to seed.
  seed %= MSEED;

  // adjust seed if neg
  if (seed < 0) seed *= -1;    // If seed is negative, set it pos.

  mj = MSEED - seed;
  mj %= MBIG;
  ma[55] = mj;
  mk = 1;

  for (int i = 1; i < 55; i++) {
    ii = (21 * i) % 55;
    ma[ii] = mk;
    mk = mj - mk;
    if (mk < 0) mk += MBIG;
    mj = ma[ii];
  }

  for (int k = 0; k < 4; k++) {
    for (int j = 1; j < 55; j++) {
      ma[j] -= ma[1 + (j + 30) % 55];
      if (ma[j] < 0) ma[j] += MBIG;
    }
  }

  inext = 0;
  inextp = 31;
}
