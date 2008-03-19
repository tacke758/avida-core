//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CODE_ARRAY_HH
#define CODE_ARRAY_HH

#define INST_FLAG_COPIED      0
#define INST_FLAG_MUTATED     1
#define INST_FLAG_EXECUTED    2
#define INST_FLAG_BREAKPOINT  3
#define NUM_INST_FLAGS        4

#include "../defs.hh"
#include "../tools/tools.hh"
#include "stats.hh"
#include "inst.hh"

class cCodeArray {
private:
  int size;
  int max_size;
  cInstruction * data;
  char * flags;

  // resizes creature, not initializing data.
  void ResetSize(int new_size, int new_max=0);
public:
  cCodeArray();
  explicit cCodeArray(int in_size, int in_max_size=0);
  /* explicit */  cCodeArray(const cCodeArray &in_code, int in_max_size=0);
  virtual ~cCodeArray();

  void operator=(const cCodeArray &other_code);
  inline int operator==(const cCodeArray &other_code) const;
  inline cInstruction & operator[](int index);

  void CopyData(const cCodeArray & other_code);

  inline int GetSize() const { return size; }
  inline int GetMaxSize() const { return max_size; } // @CAO PROPTLY REMOVE!
  inline const cInstruction & Get(int index) const {
#ifdef DEBUG
    assert(index < size);
    if (index >= size) g_debug.Error("Get(%d) in code size %d", index, size);
#endif
    return data[index];
  }
  inline void Set(int index, const cInstruction & value);
  inline int GetFlag(int index, int flag) const;
  inline void SetFlag(int index, int flag);
  inline void UnsetFlag(int index, int flag);
  inline void ToggleFlag(int index, int flag);
  int CountFlag(int flag);

  void Resize(int new_size);  // resizes creature, keeping data.
  void Insert(int line_num, const cInstruction & new_code);
  void Remove(int line_num);
  void Reset(int new_size, int new_max=0);  // resizes creature, losing data.

  // Resets to in_code at a given offset.
  void Reset(const cCodeArray & in_code, int offset=0, int new_max=0);
  void Clear();
  void Randomize();

  void Copy(const cCodeArray & code1, const cCodeArray & code2, int offset,
	    int cross_point);
  void Inject(cCodeArray * other_code, int inject_line);

  int OK() const;

  // Genetic distance tools.
  int FindGeneticDistance(const cCodeArray & in_code){ // Use best method
    return FindLevenschteinDistance(in_code); }
  
  int FindOverlap(const cCodeArray & in_code, int offset = 0) const;
  int FindBestOffset(const cCodeArray & in_code) const;
  int FindSlidingDistance(const cCodeArray & in_code) const;
  int FindHammingDistance(const cCodeArray & in_code,
			  int offset = 0) const;
  int FindLevenschteinDistance(const cCodeArray & in_code) const;

  cString DebugType() { return "cCodeArray"; }
  cString DebugInfo() { return cString::Stringf("size = %d", size); }
  int DebugSize() { return max_size; } // cString::Stringf("max_size = %d", max_size); }
};

cInstruction & cCodeArray::operator[](int index)
{
#ifdef DEBUG
  assert(index < size);

  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (op[])",
		  cStats::GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d: operator[](%d) - Index is < 0! (size=%d)",
	    cStats::GetUpdate(), index, size);
    assert(FALSE);
  }
#endif
  return data[index];
}

inline int cCodeArray::operator==(const cCodeArray &other_code) const
{
  // Make sure the sizes are the same.
  if (size != other_code.size) return FALSE;

  // Then go through line by line.
  for (int i = 0; i < size; i++)
    if (data[i] != other_code.data[i]) return FALSE;

  return TRUE;
}

inline void cCodeArray::Set(int index, const cInstruction & value)
{
  operator[](index) = value;
}

inline int cCodeArray::GetFlag(int index, int flag) const
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (GetFlag)",
		  cStats::GetUpdate(), index, size);
  }
  if (index < 0) {    
    g_debug.Error("UD %d: GetFlag() - Index is < 0!", cStats::GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  return flags[index] & (1 << flag);
}

inline void cCodeArray::SetFlag(int index, int flag)
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (SetFlag)",
		  cStats::GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d: SetFlag() - Index is < 0!", cStats::GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  flags[index] |= (1 << flag);
}

inline void cCodeArray::UnsetFlag(int index, int flag)
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (UnsetFlag)",
	    cStats::GetUpdate(), index, size);
  }
  if (index < 0) {    
    g_debug.Error("UD %d: UnsetFlag() - Index is < 0!", cStats::GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  flags[index] &= ~(1 << flag);
}

inline void cCodeArray::ToggleFlag(int index, int flag)
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (UnsetFlag)",
	    cStats::GetUpdate(), index, size);
  }
  if (index < 0) {    
    g_debug.Error("UD %d: UnsetFlag() - Index is < 0!", cStats::GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  flags[index] ^= (1 << flag);
}

#endif
