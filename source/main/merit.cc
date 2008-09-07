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

cMerit::cMerit(int in_size)
{
  shift = 0;

  while (in_size >= 256) {
    shift++;
    in_size >>= 1;
  }

  base = (UCHAR) in_size;
}

void cMerit::operator=(UINT other_merit)
{
  shift = 0;

  while (other_merit >= 256) {
    shift++;
    other_merit >>= 1;
  }

  base = (UCHAR) other_merit;
}

double cMerit::GetDouble() const
{
  double result = (double) base;
  for (int i = 0; i < shift; i++) result *= 2;
  return result;
}

int cMerit::GetIntLog() const
{
  if (shift) return shift + 7;
  if( base == 0 ) return 0;

  int result = 7; 
  while( (base >> result) == 0 ) result--;
  return result;
}

UCHAR cMerit::Shift(int in_shift)
{
  while (in_shift && base < 128) {
    base <<= 1;
    in_shift--;
  }
  shift += in_shift;

  return shift;
}

void cMerit::QShift(int in_shift)
{
  //  static FILE * fp = fopen("shift.dat", "w");
  //  fprintf(fp, "(%d, %d) -> %d -> ", base, shift, in_shift);

  int tmp_base = base;
  int tmp_shift = in_shift - 3;
  if (tmp_shift < 0) tmp_base >>= -tmp_shift;
  if (tmp_shift > 0) tmp_base <<= tmp_shift;
  tmp_base += base;

  while (tmp_base >= 256) {
    tmp_base >>= 1;
    shift++;
  }
  base = tmp_base;

  //  fprintf(fp, "(%d, %d)\n", base, shift);
}

double cMerit::CalcFitness(int gestation_time) const
{
  if (!gestation_time) return 0.0;
  double fitness = (double) base;
  for (int i = 0; i < shift; i++) fitness *= 2;
  fitness /= (double) gestation_time;

  return fitness;
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
