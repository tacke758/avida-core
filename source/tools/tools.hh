//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
// This file is for misc. objects which are of general use...

#ifndef TOOLS_HH
#define TOOLS_HH

#include "functions.hh"
#include "../defs.hh"

#include "debug.hh"
#include "memory.hh"
#include "memtrack.hh"

#include "int.hh"
#include "stat.hh"
#include "struct.hh"
#include "string.hh"
#include "file.hh"
#include "block_struct.hh"
#include "b_struct2.hh"

#include "random.hh"

/**
 * A simple class to handle up to 32 different flags.
 **/

class cFlags {
private:
  int flags;
public:
    /**
     * Construct a cFlags object with all flags cleared.
     **/
    cFlags();

    ~cFlags();

    /**
     * Sets a flag.
     *
     * @param flag_num The number of the flag (between 0 and 31).
     **/
    inline void SetFlag(int flag_num);

    /**
     * Clears a flag.
     *
     * @param flag_num The number of the flag (between 0 and 31).
     **/
    inline void UnsetFlag(int flag_num);

    /**
     * Reverses the state of a flag.
     *
     * @param flag_num The number of the flag (between 0 and 31).
     **/
    inline void FlipFlag(int flag_num);

    /**
     * Returns the state of a flag.
     *
     * @param flag_num The number of the flag (between 0 and 31).
     **/
    inline int GetFlag(int flag_num) const;

    /**
     * Implements access to the flags as if they were part of an array.
     *
     * Example:
     * <pre>
     * cFlags f;
     * f.SetFlag(2);
     * cout << f[0] << f[1] << f[2] << endl;
     * </pre>
     *
     * Output:
     * <pre>
     * 001
     * </pre>
     **/
    inline int operator[](int flag_num) const;

    inline void operator()(int flag_num, int value);


    /**
     * Clear all flags.
     **/
    inline void Clear();

  void SaveState(ostream & fp){ assert(fp.good()); fp<<flags<<endl; }
  void LoadState(istream & fp){ assert(fp.good()); fp>>flags;
  }
};

// Globals:

extern cDebug g_debug;
extern cMemTrack g_memory;
extern cRandom g_random;
extern cString default_dir;



// Inline includes
#include "tools.ii"

#endif
