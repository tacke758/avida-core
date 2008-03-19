//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "label.hh"
#include "../main/classes.hh"
#include "../tools/tools.hh" // for g_memory & g_debug
#include "../main/stats.hh"  // for g_debug call to cStats::GetUpdate()

////////////////////////////////
// cCodeLabel stuff...
////////////////////////////////

cCodeLabel::cCodeLabel()
{
  g_memory.Add(C_LABEL);
  size = 0;
}

cCodeLabel::cCodeLabel(const cCodeLabel &in_label)
{
  g_memory.Add(C_LABEL);
  size = in_label.size;
  for (int i = 0; i < size; i++) {
    nop_sequence[i] = in_label.nop_sequence[i];
  }
}

cCodeLabel::~cCodeLabel()
{
  g_memory.Remove(C_LABEL);
}

int cCodeLabel::OK()
{
  int result = TRUE;

  if (size > MAX_LABEL_SIZE) {
    g_debug.Error("Label size too large!");
    result = FALSE;
  }

  for (int i = 0; i < size; i++) {
    if (nop_sequence[i] >= MAX_NOPS) {
      g_debug.Error("UD %d: Label (size=%d) has element [%d] out of range!",
		    cStats::GetUpdate(), size, nop_sequence[i]);
      result = FALSE;
    }
  }

  return result;
}

int cCodeLabel::operator==(const cCodeLabel & other_label) const
{
  if (size != other_label.GetSize()) {
    return FALSE;
  }

  for (int i = 0; i < size; i++) {
    if (nop_sequence[i] != (UCHAR) other_label.GetNop(i)) {
      return FALSE;
    }
  }

  return TRUE;
}


// This function returns true if the sub_label can be found within
// the label affected.
int cCodeLabel::FindSublabel(cCodeLabel & sub_label)
{
  int error = FALSE;
  int i, j;

  for (i = 0; i <= size - sub_label.GetSize(); i++) {
    for (j = 0; j < sub_label.GetSize(); j++) {
      if (nop_sequence[i + j] != (UCHAR) sub_label.GetNop(j)) {
	error = TRUE;
	break;
      }
    }
    if (!error) return i;
    error = FALSE;
  }

  return -1;
}

int cCodeLabel::GetTrinary() const
{
  int i, trin = 0;

  for (i = 0; i < size; i++) {
    trin *= 3;
    trin += nop_sequence[i];
  }

  return trin;
}

void cCodeLabel::Complement()
{
  int i;

  for (i = 0; i < size; i++) {
    nop_sequence[i]++;
    if (nop_sequence[i] == NUM_REGISTERS) nop_sequence[i] = 0;
  }
}


void cCodeLabel::SaveState(ostream & fp){
  assert(fp.good());
  fp<<"cCodeLabel"<<" ";

  fp<<"|"; // marker

  fp<<size;
  for( int i=0; i<MAX_LABEL_SIZE; ++i ){
    fp<<nop_sequence[i];
  }
  fp<<endl;
}


void cCodeLabel::LoadState(istream & fp){
  assert(fp.good());
  cString foo;
  fp>>foo;
  assert( foo == "cCodeLabel");

  char marker;
  fp>>marker;
  assert( marker == '|' );

  fp.get(size);
  for( int i=0; i<MAX_LABEL_SIZE; ++i ){
    fp.get(nop_sequence[i]);
  }
}



