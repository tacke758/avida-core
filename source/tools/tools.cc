//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include "tools.hh"

////////////////////////
// Global Variables:
////////////////////////

cDebug g_debug;
cMemTrack g_memory;
cRandom g_random;
cString default_dir;

///////////////////////
//  cFlags
///////////////////////

cFlags::cFlags()
{
  flags = 0;
}

cFlags::~cFlags()
{
}
