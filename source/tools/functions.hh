// This file is for misc. functions which are of general use...

#ifndef FUNCTIONS_HH
#define FUNCTIONS_HH

#include <math.h>

#ifndef NULL
#define NULL 0
#endif


inline int Min(int in1, int in2);
inline int Max(int in1, int in2);
inline int Range(int value, int min, int max);
inline int Mod(int value, int base);
inline int Pow(int bas, int expon);

//////////////////////
// Generic (inline) functions...
//////////////////////

inline int Min(int in1, int in2)
{
  return (in1 > in2) ? in2 : in1;
}

inline int Max(int in1, int in2)
{
  return (in1 < in2) ? in2 : in1;
}

inline int Range(int value, int min, int max)
{
  value = (value < min) ? min : value;
  return (value > max) ? max : value;
}

inline int Mod(int value, int base)
{
  value %= base;
  if (value < 0) value += base;
  return value;
}

inline int Pow(int base, int expon)
{
  int value = 1;
  for (int i = 0; i < expon; i++) value *= base;
  return value;
}

inline int Breakpoint()
{
  int * blah = new int;
  delete blah;
  return (blah == NULL);
}

#endif
