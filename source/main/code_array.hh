//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CODE_ARRAY_HH
#define CODE_ARRAY_HH

#define INST_FLAG_COPIED   0
#define INST_FLAG_MUTATED  1
#define INST_FLAG_EXECUTED 2
#define NUM_INST_FLAGS     3

#include "../defs.hh"
#include "../tools/tools.hh"
#include "stats.hh"

class cCodeArray {
private:
  int size;
  char * data;
  char * flags;
public:
  cCodeArray();
  cCodeArray(int in_size);
  cCodeArray(const cCodeArray &in_code_array);
  ~cCodeArray();

  void operator=(const cCodeArray &other_code);
  inline int operator==(const cCodeArray &other_code) const;
  inline char & operator[](int index);

  void CopyData(const cCodeArray &other_code);

  inline int GetSize() const { return size; }
  inline char Get(int index) const { return data[index]; }
  inline void Set(int index, char value) { operator[](index) = value; }
  inline int GetFlag(int index, int flag) const;
  inline void SetFlag(int index, int flag);
  inline void UnsetFlag(int index, int flag);
  int CountFlag(int flag);

  void Resize(int new_size);  // resizes creature, keeping data.
  void Insert(int line_num, char new_code);
  void Remove(int line_num);
  void Reset(int new_size);   // resizes creature, losing data.
  void Reset(const cCodeArray & in_code, int offset = 0); // resets to in_code
  void Clear();
  void Randomize();

  void Copy(cCodeArray * code1, cCodeArray * code2, int offset,
	    int cross_point);
  void Inject(cCodeArray * other_code, int inject_line);

  int OK();
  void TestPrint();

  // Genetic distance tools.
  int FindGeneticDistance(cCodeArray * in_code_array){  // Use best method
    return FindSlidingDistance(in_code_array); }
  
  int FindOverlap(cCodeArray * in_code_array, int offset = 0);
  int FindBestOffset(cCodeArray * in_code_array);
  int FindLevenschteinDistance(cCodeArray * in_code_array);  // Not Implemented
  int FindSlidingDistance(cCodeArray * in_code_array);
  int FindHammingDistance(cCodeArray * in_code_array, int offset = 0);
};


inline int cCodeArray::GetFlag(int index, int flag) const
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d past size %d",
		  stats.GetUpdate(), index, size);
  }
  if (index < 0) {    
    g_debug.Error("UD %d: GetFlag() - Index is < 0!", stats.GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  return flags[index] & (1 << flag);
}

inline void cCodeArray::SetFlag(int index, int flag)
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d past size %d",
		  stats.GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d: SetFlag() - Index is < 0!", stats.GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  flags[index] |= (1 << flag);
}

inline void cCodeArray::UnsetFlag(int index, int flag)
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d past size %d",
	    stats.GetUpdate(), index, size);
  }
  if (index < 0) {    
    g_debug.Error("UD %d: UnsetFlag() - Index is < 0!", stats.GetUpdate());
  }
  if (flag >= NUM_INST_FLAGS) g_debug.Error("flag > NUM_INST_FLAGS");
#endif
  flags[index] &= ~(1 << flag);
}

char & cCodeArray::operator[](int index)
{
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d past size %d",
		  stats.GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d.%d: operator[](%d) - Index is < 0!",
	    stats.GetUpdate(), stats.GetSubUpdate(), index);
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

#endif
