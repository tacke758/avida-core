/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_HEAD_HH
#define CPU_HEAD_HH

#include "../defs.hh"

#include "../main/inst.hh"

class cHardwareBase;
class cCodeLabel;
class cGenome;
class cCPUMemory;

/**
 * The cCPUHead class contains a pointer to locations in memory for a CPU.
 **/

class cCPUHead {
private:
  cHardwareBase * main_hardware;
  cHardwareBase * cur_hardware;
  int position;

  int FindLabel_Forward(const cCodeLabel & search_label,
			const cGenome & search_mem, int pos);
  int FindLabel_Backward(const cCodeLabel & search_label,
			 const cGenome & search_mem, int pos);
public:
  cCPUHead();
  cCPUHead(cHardwareBase * in_hardware, int in_pos = 0);
  cCPUHead(const cCPUHead & in_cpu_head);
  ~cCPUHead() { ; }

  void Reset(cHardwareBase * new_hardware = NULL);
  
  /**
   * This function keeps the position within the range of the current memory.
   **/
  void Adjust();
  
  /**
   * Set the new position of the head (and adjust it into range in Set()).
   **/

  void Set(int new_pos, cHardwareBase * in_hardware = NULL);
  void AbsSet(int new_pos) { position = new_pos; }
  void Set(const cCPUHead & in_head) {
    position = in_head.position;
    cur_hardware  = in_head.cur_hardware;
  }

  /**
   * Increment the new position of the head by 'jump'.
   **/

  void Jump(int jump);
  void LoopJump(int jump);
  void AbsJump(int jump);
  
  // Other manipulation functions.
  void Advance();
  void Retreat();
  cCPUHead FindLabel(const cCodeLabel & label, int direction=1);

  // Accessors.
  int GetPosition() const { return position; }
  const cCPUMemory & GetMemory() const;
  cHardwareBase * GetCurHardware() const { return cur_hardware; }
  cHardwareBase * GetMainHardware() const { return main_hardware; }
  const cInstruction & GetInst() const;
  const cInstruction & GetInst(int offset) const;
  // int GetFlag(int id) const;

  void SetInst(const cInstruction & value);
  void InsertInst(const cInstruction & in_char);
  void RemoveInst();
  const cInstruction & GetNextInst();

  bool & FlagCopied();
  bool & FlagMutated();
  bool & FlagExecuted();
  bool & FlagBreakpoint();
  bool & FlagPointMut();
  bool & FlagCopyMut();

  // Operator Overloading...
  cCPUHead & operator=(const cCPUHead & in_cpu_head);
  cCPUHead & operator++();
  cCPUHead & operator--();
  cCPUHead & operator++(int);
  cCPUHead & operator--(int);

  inline int operator-(const cCPUHead & in_cpu_head) {
    if (cur_hardware != in_cpu_head.cur_hardware) return 0;
    else return position - in_cpu_head.position;
  }
  inline int operator==(const cCPUHead & in_cpu_head) const {
    return (cur_hardware == in_cpu_head.cur_hardware) &&
      (position == in_cpu_head.position);
  }

  // Bool Tests...
  inline bool AtFront() const { return (position == 0); }
  bool AtEnd() const;
  bool InMemory() const;

  // Test functions...
  int TestParasite() const;
};

#endif

