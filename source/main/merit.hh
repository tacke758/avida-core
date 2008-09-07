//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef MERIT_HH
#define MERIT_HH

#include "../defs.hh"
#include "../tools/tools.hh" // for g_debug

class cMerit {
private:
  UCHAR base;
  UCHAR shift;
public:
  inline cMerit() { base = 1; shift = 0; }
  cMerit(int in_size); // Input the size of the creature!

  inline int OK();

  inline void operator=(cMerit other_merit);
         void operator=(UINT other_merit);
  inline int  operator>(cMerit other_merit)  const;
  inline int  operator<(cMerit other_merit)  const;
  inline void operator+=(cMerit other_merit);
  inline int  operator==(cMerit other_merit) const;
  inline int  operator==(UINT other_merit)   const;

  inline void Clear() { base = 0; shift = 0; }

  inline UCHAR GetBase()  const { return base; }
  inline UCHAR GetShift() const { return shift; }
  inline UINT GetUInt()   const { return (UINT) base << (UINT) shift; }
  inline int GetInt()     const { return ((int) base) << shift; }
  double GetDouble()      const;

  inline int GetBit(int bit_num)  const {
    return (bit_num < shift || bit_num > shift + 7) ?
      0 : (base & (1 << bit_num - shift));
  }

  int GetIntLog() const;
  UCHAR Shift(int in_shift=1);
  void QShift(int in_shift=1);
  double CalcFitness(int gestation_time) const;
};

// Used for summing up normal merits.
class cLongMerit {
private:
  UINT base;
  UINT shift;
public:
  inline cLongMerit() { base = 0; shift = 0; }

  inline void operator=(UINT in_int);
  void operator+=(const cMerit & in_merit);
  inline cMerit GetAverage(int num_entries) const;
  inline void Clear() { base = 0; shift = 0; }

  inline UINT GetBase() const { return base; }
  inline UINT GetShift() const { return shift; }
};

/////////////
//  cMerit
/////////////

inline int cMerit::OK()
{
  int result = TRUE;

  if (shift && base < 128) {
    g_debug.Comment("Merit has positive shift without base being full.");
    result = FALSE;
  }

  return result;
}

inline void cMerit::operator=(cMerit other_merit)
{
  base = other_merit.GetBase();
  shift = other_merit.GetShift();
}

inline int cMerit::operator>(cMerit other_merit) const
{
  if (shift != other_merit.GetShift()) return (shift > other_merit.GetShift());
  else return (base > other_merit.GetBase());
}

inline int cMerit::operator<(cMerit other_merit) const
{
  if (shift != other_merit.GetShift()) return (shift < other_merit.GetShift());
  else return (base < other_merit.GetBase());
}

inline void cMerit::operator+=(cMerit other_merit)
{
  // First align the shift factors

  UCHAR other_base = other_merit.GetBase();
  if (other_merit.GetShift() > shift) {
    shift += other_merit.GetShift() - shift;
    base >>= other_merit.GetShift() - shift;
  } else {
    other_base >>= shift - other_merit.GetShift();
  }

  // Then just add the bases.  
  int new_base = (int) base + (int) other_base;
  if (new_base > 255) {
    new_base >>= 1;
    shift++;
  }
  base = (UCHAR) new_base;
}

inline int cMerit::operator==(cMerit other_merit) const
{
  return (other_merit.GetShift() == shift && other_merit.GetBase() == base);
}

inline int cMerit::operator==(UINT other_merit) const
{
  return(GetUInt() == other_merit);
}

/////////////////
//  cLongMerit
/////////////////

inline void cLongMerit::operator=(UINT in_int)
{
  base = in_int;
  shift = 0;
}

inline cMerit cLongMerit::GetAverage(int num_entries) const
{
  UINT temp_base;
  if( num_entries!=0 )
    temp_base = base/num_entries;
  else
    temp_base=0;

  cMerit result(temp_base);
  result.Shift(shift);

  return result;
}

#endif
