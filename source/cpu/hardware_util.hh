//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef HARDWARE_UTIL_HH
#define HARDWARE_UTIL_HH

#include "../tools/tDictionary.hh"

class cString;
class cInstSet;
class cHardwareBase;

// moved to cpu/hardware_method.hh for porting to gcc 3.1 -- k
//typedef void (cHardwareBase::*tHardwareMethod)();

class cHardwareUtil {
public:
  static void LoadInstSet( cString filename, cInstSet & inst_set, 
	tDictionary<int> & nop_dict, tDictionary<int> & inst_dict );
  static void LoadInstSet_CPUOriginal(const cString & filename,
				      cInstSet & inst_set);
  static void LoadInstSet_CPU4Stack(const cString & filename,
				    cInstSet & inst_set);
  static cInstSet & DefaultInstSet(const cString & inst_filename);
};

#endif
