//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LOOKUP_HH
#define LOOKUP_HH

#include "string.hh"

/*
@COW The class cNameLookup is not used at all,
     and can probably be removed. 2000-04-04   
class cNameLookup {
private:
  int size;
  int max_size;
  cString * name_array;
public:
  cNameLookup() : size(0), max_size(4) {
    name_array = new cString[max_size];
  }
  ~cNameLookup() { delete [] name_array; }

  int GetID(cString in_name);
  int GetSize() { return size; }
};
*/

#endif
