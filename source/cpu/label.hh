//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LABEL_HH
#define LABEL_HH

#include "../defs.hh"
#include "../tools/tools.hh"
#include "../tools/memtrack.hh"
#include "../tools/string.hh"
#include "cpu_defs.hh" // #defines only

/**
 * The cCodeLabel class is used to identify a label within the genotype of
 * a creature, and aid in its manipulation.
 **/

class cCodeLabel {
private:
  UCHAR size;
  UCHAR nop_sequence[MAX_LABEL_SIZE];
public:
  cCodeLabel();
  cCodeLabel(const cCodeLabel &in_label);
  ~cCodeLabel();

  int OK();
  int operator==(const cCodeLabel & other_label) const;
  int operator!=(const cCodeLabel & other_label) const
    { return !(operator==(other_label)); }
  int FindSublabel(cCodeLabel & sub_label);

  inline void AddNop(int nop_num);
  inline void Clear() { size = 0; }
  int GetTrinary() const;
  void Complement();

  inline UCHAR GetSize() const { return size; }
  inline int GetNop(int position) const { return (int) nop_sequence[position]; }
  inline cString AsString() const;

  void SaveState(ostream & fp);
  void LoadState(istream & fp);
};

void cCodeLabel::AddNop(int nop_num) {
#ifdef DEBUG
  if (nop_num >= MAX_NOPS) {
    g_debug.Error("Trying to add instruction %d to label (size=%d)!",
		  nop_num, size);
  }
#endif
  if (size < MAX_LABEL_SIZE) {
    nop_sequence[size++] = (UCHAR) nop_num;
  }
}

cString cCodeLabel::AsString() const
{
  cString out_string;
  for (int i = 0; i < size; i++) {
    out_string += (char) nop_sequence[i] + 'A';
  }

  return out_string;
}

#endif
