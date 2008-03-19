//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "lookup.hh"
/*
@COW The class cNameLookup is not used at all,
     and can probably be removed. 2000-04-04   
int cNameLookup::GetID(cString in_name)
{
  // See if this name is already in the lookup table.
  for (int i = 0; i < size; i++) {
    if (name_array[i] == in_name) return i;
  }

  // If not, make sure we have room to add it.
  if (size == max_size) {
    max_size *= 2;
    cString * new_name_array = new cString[max_size];
    for (int i = 0; i < size; i++) new_name_array[i] = name_array[i];
    delete [] name_array;
    name_array = new_name_array;
  }

  // Add it in, and return.
  name_array[size] = in_name;
  size++;
  return size - 1;
}

*/
