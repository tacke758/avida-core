//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CODE_ARRAY_HH
#define CODE_ARRAY_HH

#define INST_FLAG_COPIED      0
#define INST_FLAG_MUTATED     1
#define INST_FLAG_EXECUTED    2
#define INST_FLAG_BREAKPOINT  3
#define INST_FLAG_POINT_MUT   4
#define INST_FLAG_COPY_MUT    5
#define INST_FLAG_CROSS_MUT   7
#define NUM_INST_FLAGS        8

#define CA_FLAG_TYPE       char



#include "../defs.hh"
#include "../tools/tools.hh"
#include "stats.hh"
#include "inst.hh"

/**
 * This class stores the genome of an Avida organism.
 **/

class cCodeArray {
private:
  int size;
  int max_size;
  cInstruction * data;
  CA_FLAG_TYPE * flags;

  // resizes creature, not initializing data.
  void ResetSize(int new_size, int new_max=0);
public:
  cCodeArray();
  explicit cCodeArray(int in_size, int in_max_size=0);
  /* explicit */  cCodeArray(const cCodeArray &in_code, int in_max_size=0);
  cCodeArray(cString in_string);
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

  int FindInstruction(const cInstruction & inst, int start_index=0);
  inline int HasInstruction(const cInstruction & inst){
    return ( FindInstruction(inst) >= 0 ) ? TRUE : FALSE; }

  inline void Set(int index, const cInstruction & value);
  inline int GetFlag(int index, int flag) const;
  inline void SetFlag(int index, int flag);
  inline void UnsetFlag(int index, int flag);
  inline void ToggleFlag(int index, int flag);
  int CountFlag(int flag) const;

  inline CA_FLAG_TYPE GetFlags(int index) const ;
  inline void SetFlags(int index, CA_FLAG_TYPE flags);


    /**
     * Resizes the genome and keeps the data. If the new size
     * is longer then the old size, the additional positions in
     * the genome are filled up with random instructions.
     **/
    void Resize(int new_size);  // resizes creature, keeping data.

    /**
     * Inserts an instruction into the genome.
     *
     * @param line_num The position at which the instruction should be
     * inserted.
     * @param new_code The new instruction.
     **/
    void Insert(int line_num, const cInstruction & new_code);

    /**
     * Removes an instruction from the genome.
     **/
    void Remove(int line_num);

    /**
     * Resizes and reinitializes the genome. The previous data is lost.
     **/
    void Reset(const int new_size, const int new_max=0);

  // Resets to in_code at a given offset.
  void Reset(const cCodeArray & in_code, const int offset=0,
	     int new_size=0);

    /**
     * Sets all instructions to @ref cInstruction::GetInstDefault().
     **/
    void Clear();

    /**
     * Replaces all instructions in the genome with a sequence of random
     * instructions.
     **/
    void Randomize();

    void Copy(const cCodeArray & code1, const cCodeArray & code2, int offset,
	    int cross_point);

    /**
     * Inserts a sequence of instructions into the current genome.
     **/
    void Inject(cCodeArray * other_code, int inject_line);

    int OK() const;

  // Genetic distance tools.
  int FindGeneticDistance(const cCodeArray & in_code) // Use best method
    { return FindLevensteinDistance(in_code); }

  int FindOverlap(const cCodeArray & in_code, int offset = 0) const;
  int FindBestOffset(const cCodeArray & in_code) const;
  int FindSlidingDistance(const cCodeArray & in_code) const;
  int FindHammingDistance(const cCodeArray & in_code,
			  int offset = 0) const;
  int FindLevensteinDistance(const cCodeArray & in_code) const;

  // Mutations...
  void CrossoverInto(const cCodeArray & code1, int start1, int end1,
		     const cCodeArray & code2, int start2, int end2);


  cString AsString() const;

  cString DebugType() const { return "cCodeArray"; }
  cString DebugInfo() const { return cString::Stringf("size = %d", size); }
  int DebugSize() const { return max_size; }
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


inline CA_FLAG_TYPE cCodeArray::GetFlags(int index) const {
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (SetFlag)",
		  cStats::GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d: SetFlag() - Index is < 0!", cStats::GetUpdate());
  }
#endif
  return flags[index];
}


inline void cCodeArray::SetFlags(int index, CA_FLAG_TYPE _flags){
#ifdef DEBUG
  if (index >= size) {
    g_debug.Error("UD %d: Trying to index %d into code size %d (SetFlag)",
		  cStats::GetUpdate(), index, size);
  }
  if (index < 0) {
    g_debug.Error("UD %d: SetFlag() - Index is < 0!", cStats::GetUpdate());
  }
#endif
  flags[index] = _flags;
}

#endif


