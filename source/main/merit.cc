//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "merit.hh"

/////////////
//  cMerit
/////////////

int cMerit::GetIntLog() const
{
  int shift = 0;
  while ((value >> shift ) > 0) shift++;
  return shift;
}

void cMerit::Shift(int in_shift)
{
  value <<= in_shift;
}

/*
void cMerit::QShift(int in_shift)
{
  UINT temp_value = value;
  in_shift -= 3;
  if (in_shift >= 0) value += value << in_shift;
  else value += value >> -in_shift;
  if (temp_value == value) value++;
}
*/

void cMerit::Bonus(double award)
{
  value += (UINT) (award * (double) value);
}

double cMerit::CalcFitness(int gestation_time) const
{
  if (!gestation_time) return 0.0;
  return ((double) value) / ((double) gestation_time);
}


/////////////////
//  cLongMerit
/////////////////


void cLongMerit::operator+=(const cMerit & in_merit)
{
  // first, expand out the in_merit...

  UINT in_base  = (UINT) in_merit.GetBase();
  UINT in_shift = (UINT) in_merit.GetShift();
  while (in_base < ((UINT) ((1 << 31) && in_shift))) {
    in_base <<= 1;
    in_shift--;
  }

  // Now align them so that they both have the same shift-factor...

  if (in_shift <= shift) {
    in_base >>= (shift - in_shift);
    in_shift = shift;
  }
  else {
    base >>= (in_shift - shift);
    shift = in_shift;
  }

  // And add them togeather...

  if (base + in_base >= base) base += in_base;
  else {
    base >>= 1;
    in_base >>= 1;
    shift++;
    base += in_base;
  }
}
