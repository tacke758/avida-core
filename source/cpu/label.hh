//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LABEL_HH
#define LABEL_HH

#include "../tools/tArray.hh"
#include "../tools/tools.hh"
#include "../tools/string.hh"

#include "../defs.hh"
#include "cpu_defs.hh"        // #defines only

/**
 * The cCodeLabel class is used to identify a label within the genotype of
 * a creature, and aid in its manipulation.
 **/

class cCodeLabel {
private:
  tArray<char> nop_sequence;
  int size;
  const int base;
public:
  cCodeLabel(int in_base=MAX_NOPS);
  cCodeLabel(const cCodeLabel &in_label);
  ~cCodeLabel();

  bool OK();
  bool operator==(const cCodeLabel & other_label) const;
  bool operator!=(const cCodeLabel & other_label) const
    { return !(operator==(other_label)); }
  char operator[](int position) const { return (int) nop_sequence[position]; }
  int FindSublabel(cCodeLabel & sub_label);

  void Clear() { size = 0; }
  inline void AddNop(int nop_num);
  inline void Rotate(const int rot);

  int GetSize() const { return size; }
  int GetBase() const { return base; }
  inline cString AsString() const;
  inline int AsInt() const;

  void SaveState(std::ostream & fp);
  void LoadState(std::istream & fp);
};

void cCodeLabel::AddNop(int nop_num) {
  assert (nop_num < base);

  if (size < MAX_LABEL_SIZE) {
    if (size == nop_sequence.GetSize()) {
      nop_sequence.Resize(size+1);
    }
    nop_sequence[size++] = (char) nop_num;
  }
}

void cCodeLabel::Rotate(const int rot)
{
  for (int i = 0; i < size; i++) {
    nop_sequence[i] += rot;
    if (nop_sequence[i] >= base) nop_sequence[i] -= base;
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

int cCodeLabel::AsInt() const
{
  int value = 0;

  for (int i = 0; i < size; i++) {
    value *= base;
    value += nop_sequence[i];
  }

  return value;
}

#endif
