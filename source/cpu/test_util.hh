//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef TEST_UTIL_HH
#define TEST_UTIL_HH

// ------------------------------------------------------------------------
//  This class uses test CPUs in combination with genotypes and all types
//  of hardware in order to produce more useful test info.
// ------------------------------------------------------------------------

#include "../tools/string.hh"

class cGenome;
class cGenotype;

class cTestUtil {
public:
  static void PrintGenome(const cGenome & genome, cString filename="",
			  cGenotype * genotype=NULL, int update_out=-1);
};

#endif
