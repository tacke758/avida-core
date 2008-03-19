//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef LOOKUP_HH
#define LOOKUP_HH

#include "string.hh"

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


#endif
