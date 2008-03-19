//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef HARDWARE_UTIL_HH
#define HARDWARE_UTIL_HH

#include "../tools/tDictionary.hh"

class cString;
class cInstLib;
class cHardwareBase;

// moved to cpu/hardware_method.hh for porting to gcc 3.1 -- k
//typedef void (cHardwareBase::*tHardwareMethod)();

class cHardwareUtil {
public:
  static void LoadInstLib(
    const cString & filename,
    cInstLib & lib, 
		tDictionary<int> & nop_dict,
		tDictionary<tHardwareMethod> & inst_dict,
		tDictionary<int> & nop_dict_2,
		tDictionary<int> & inst_dict_2
  );
  static void LoadInstLibCPU(const cString & filename, cInstLib & lib);
  static cInstLib & DefaultInstLib(const cString & inst_filename);
};

#endif
