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
