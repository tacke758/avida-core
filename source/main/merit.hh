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

#define DEFAULT_TASK_BONUS 0.25

class cMerit {
private:
  UINT value;
public:
  inline cMerit() { value = 0; }
  inline cMerit(int in_size) { value = in_size; }

  inline int OK() { return TRUE; }

  inline void operator=(cMerit _merit)  { value = _merit.value; }
  inline void operator=(UINT _merit)    { value = _merit; }
  inline void operator+=(cMerit _merit) { value += _merit.value; }

  inline int  operator>(cMerit _merit)  const { return value > _merit.value; }
  inline int  operator<(cMerit _merit)  const { return value < _merit.value; }
  inline int  operator==(cMerit _merit) const { return value == _merit.value; }
  inline int  operator==(UINT _merit)   const { return value == _merit; }

  inline void Clear() { value = 0; }

  inline UINT GetBase() const {
    return value;
  }
  inline UINT GetShift() const {
    return 0;
  }

  inline UINT GetUInt()   const { return value; }
  inline int GetInt()     const { return value; }
  double GetDouble()      const { return (double) value; }

  inline int GetBit(int bit_num)  const {
    return value & (1 << bit_num);
  }

  int GetIntLog() const;
  void Shift(int in_shift=1);
  // void QShift(int in_shift=1);
  void Bonus(double award=DEFAULT_TASK_BONUS);
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
