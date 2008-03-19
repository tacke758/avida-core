//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_MEMORY_HH
#define CPU_MEMORY_HH

#include "../main/genome.hh"

class cCPUMemory;

class cMemoryFlags {
  friend class cCPUMemory;
private:
  bool copied;      // Has this site been copied?
  bool mutated;     // Has this site been mutated?
  bool executed;    // Has this site been executed?
  bool breakpoint;  // Has a breakpoint been set at this position?
  bool point_mut;   // Has there been a point mutation here?
  bool copy_mut;    // Has there been a copy mutation here?
  bool injected;    // Was this instruction injected into the genome?

public:
  cMemoryFlags() { Clear(); }
  ~cMemoryFlags() { ; }

  void Clear();
};

class cCPUMemory : public cGenome {
private:
  tArray<cMemoryFlags> flag_array;

  // A collection of sloppy instructions to perform oft-used functions that
  // will need to be cleaned up after this is run.
  void SloppyResize(int new_size);           // Set size, ignore new contents.
  void SloppyInsert(int pos, int num_lines); // Add lines, ignore new contents.
public:
  explicit cCPUMemory(int _size=1);
  cCPUMemory(const cCPUMemory & in_memory);
  cCPUMemory(const cGenome & in_genome);
  cCPUMemory(const cString & in_string);
  ~cCPUMemory();

  void operator=(const cCPUMemory & other_memory);
  void operator=(const cGenome & other_genome);
  void Copy(int to, int from);

  void Clear();
  void ClearFlags();
  void Reset(int new_size);     // Reset size, clearing contents...
  void Resize(int new_size);    // Reset size, save contents, init to default
  void ResizeOld(int new_size); // Reset size, save contents, init to previous

  bool FlagCopied(int pos) const     { return flag_array[pos].copied; }
  bool FlagMutated(int pos) const    { return flag_array[pos].mutated; }
  bool FlagExecuted(int pos) const   { return flag_array[pos].executed; }
  bool FlagBreakpoint(int pos) const { return flag_array[pos].breakpoint; }
  bool FlagPointMut(int pos) const   { return flag_array[pos].point_mut; }
  bool FlagCopyMut(int pos) const    { return flag_array[pos].copy_mut; }
  bool FlagInjected(int pos) const   { return flag_array[pos].injected; }

  bool & FlagCopied(int pos)     { return flag_array[pos].copied; }
  bool & FlagMutated(int pos)    { return flag_array[pos].mutated; }
  bool & FlagExecuted(int pos)   { return flag_array[pos].executed; }
  bool & FlagBreakpoint(int pos) { return flag_array[pos].breakpoint; }
  bool & FlagPointMut(int pos)   { return flag_array[pos].point_mut; }
  bool & FlagCopyMut(int pos)    { return flag_array[pos].copy_mut; }
  bool & FlagInjected(int pos)   { return flag_array[pos].injected; }

  void Insert(int pos, const cInstruction & in_inst);
  void Insert(int pos, const cGenome & in_genome);
  void Remove(int pos, int num_insts=1);
  void Replace(int pos, int num_insts, const cGenome & in_genome);
};

#endif
