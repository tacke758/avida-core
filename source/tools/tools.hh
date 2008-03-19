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

class cFlags {
private:
  int flags;
public:
  cFlags();
  ~cFlags();

  inline void SetFlag(int flag_num);
  inline void UnsetFlag(int flag_num);
  inline void FlipFlag(int flag_num);
  inline int GetFlag(int flag_num) const;

  inline int operator[](int flag_num) const;
  inline void operator()(int flag_num, int value);

  inline void Clear();
};

// Globals:

extern cDebug g_debug;
extern cMemTrack g_memory;
extern cRandom g_random;
extern cString default_dir;

/////
// cFlags
/////

inline void cFlags::SetFlag(int flag_num)
{
#ifdef DEBUG
  if (flag_num < 0) g_debug.Error("Flag index must not be negative!");
  if (flag_num >= 32) g_debug.Error("Flag index must not be < 32!");
#endif
  flags |= 1 << flag_num;
}

inline void cFlags::UnsetFlag(int flag_num)
{
  flags &= ~(1 << flag_num);
}

inline void cFlags::FlipFlag(int flag_num)
{
  flags ^= 1 << flag_num;
}

inline int cFlags::GetFlag(int flag_num) const
{
  return flags & (1 << flag_num);
}

inline int cFlags::operator[](int flag_num) const
{
#ifdef DEBUG
  if (flag_num < 0) g_debug.Error("Flag index must not be negative!");
  if (flag_num >= 32) g_debug.Error("Flag index must not be < 32!");
#endif
  return (flags & (1 << flag_num)) ? TRUE : FALSE;
}

inline void cFlags::operator()(int flag_num, int value)
{
  if (value) SetFlag(flag_num);
  else UnsetFlag(flag_num);
}

inline void cFlags::Clear()
{
  flags = 0;
}

#endif
