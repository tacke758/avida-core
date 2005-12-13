//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_MEMORY_HH
#define CPU_MEMORY_HH

#ifndef GENOME_HH
#include "cGenome.h"
#endif
#ifndef tManagedPointerArray_h
#include "tManagedPointerArray.h"
#endif
#ifndef MEMORY_FLAGS_HH
#include "cMemoryFlags.h"
#endif

class cGenome;
class cInstruction;
class cMemoryFlags; // access
class cString;

class cCPUMemory : public cGenome {
private:
	static const unsigned char MASK_COPIED   = 0x01;
	static const unsigned char MASK_MUTATED  = 0x02;
	static const unsigned char MASK_EXECUTED = 0x04;
	static const unsigned char MASK_BREAK    = 0x08;
	static const unsigned char MASK_POINTMUT = 0x10;
	static const unsigned char MASK_COPYMUT  = 0x20;
	static const unsigned char MASK_INJECTED = 0x40;
	static const unsigned char MASK_UNUSED   = 0x80; // unused bit

  tArray<unsigned char> flag_array;

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

  void Clear()
	{
		for (int i = 0; i < active_size; i++) {
			genome[i].SetOp(0);
			flag_array[i] = 0;
		}
	}
  void ClearFlags() { flag_array.SetAll(0); }
  void Reset(int new_size);     // Reset size, clearing contents...
  void Resize(int new_size);    // Reset size, save contents, init to default
  void ResizeOld(int new_size); // Reset size, save contents, init to previous

  bool FlagCopied(int pos) const     { return MASK_COPIED   & flag_array[pos]; }
  bool FlagMutated(int pos) const    { return MASK_MUTATED  & flag_array[pos]; }
  bool FlagExecuted(int pos) const   { return MASK_EXECUTED & flag_array[pos]; }
  bool FlagBreakpoint(int pos) const { return MASK_BREAK    & flag_array[pos]; }
  bool FlagPointMut(int pos) const   { return MASK_POINTMUT & flag_array[pos]; }
  bool FlagCopyMut(int pos) const    { return MASK_COPYMUT  & flag_array[pos]; }
  bool FlagInjected(int pos) const   { return MASK_INJECTED & flag_array[pos]; }

  void SetFlagCopied(int pos)     { flag_array[pos] |= MASK_COPIED;   }
  void SetFlagMutated(int pos)    { flag_array[pos] |= MASK_MUTATED;  }
  void SetFlagExecuted(int pos)   { flag_array[pos] |= MASK_EXECUTED; }
  void SetFlagBreakpoint(int pos) { flag_array[pos] |= MASK_BREAK;    }
  void SetFlagPointMut(int pos)   { flag_array[pos] |= MASK_POINTMUT; }
  void SetFlagCopyMut(int pos)    { flag_array[pos] |= MASK_COPYMUT;  }
  void SetFlagInjected(int pos)   { flag_array[pos] |= MASK_INJECTED; }
	
	void ClearFlagMutated(int pos)  { flag_array[pos] &= ~MASK_MUTATED;  }
	void ClearFlagCopyMut(int pos)  { flag_array[pos] &= ~MASK_COPYMUT;  }

  void Insert(int pos, const cInstruction & in_inst);
  void Insert(int pos, const cGenome & in_genome);
  void Remove(int pos, int num_insts=1);
  void Replace(int pos, int num_insts, const cGenome & in_genome);
};

#endif
