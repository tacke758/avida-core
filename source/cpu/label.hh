//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LABEL_HH
#define LABEL_HH

// The cCodeLabel class is used to identify a label within the genotype of
// a creature, and aid in its manipulation.

#include "../defs.hh"
#include "../tools/memtrack.hh"
#include "../tools/string.hh"
#include "cpu_defs.hh" // #defines only


class cCodeLabel { 
private:
  UCHAR size;
  UCHAR nop_sequence[MAX_LABEL_SIZE];
public:
  cCodeLabel();
  cCodeLabel(const cCodeLabel &in_label);
  ~cCodeLabel();

  int OK();
  int operator==(cCodeLabel other_label);
  int FindSublabel(cCodeLabel & sub_label);

  void AddNop(int nop_num);
  inline void Clear() { size = 0; }
  int GetTrinary() const;
  void Complement();

  inline UCHAR GetSize() const { return size; }
  inline int GetNop(int position) const { return (int) nop_sequence[position]; }
  inline cString AsString() const;
};


cString cCodeLabel::AsString() const
{
  cString out_string;
  for (int i = 0; i < size; i++) {
    out_string += (char) nop_sequence[i] + 'A';
  }

  return out_string;
}

#endif
