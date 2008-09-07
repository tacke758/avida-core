/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_HEAD_HH
#define CPU_HEAD_HH

// The cCPUHead class contains a pointer to locations in memory for a CPU.

#include "../defs.hh"
#include "../main/code_array.hh" // @TCC

class cCPUHead {
private:
  cBaseCPU * main_cpu;
  cBaseCPU * cur_cpu;
  int position;
public:
  cCPUHead();
  cCPUHead(cBaseCPU * in_cpu, int in_pos = 0);
  cCPUHead(const cCPUHead & in_cpu_head);
  // inline ~cCPUHead() { ; }

  inline void Reset(cBaseCPU * new_cpu = NULL);

  // This function keeps the position within the range of the current memory.
  inline void Adjust();

  // Set the new position of the head (and adjust it into range in Set());
  inline void Set(int new_pos, cBaseCPU * in_cpu = NULL) {
    position = new_pos;
    if (in_cpu) cur_cpu = in_cpu;
    Adjust();
  }
  inline void AbsSet(int new_pos) { position = new_pos; }
  inline void Set(const cCPUHead & in_head) {
    position = in_head.position;
    cur_cpu  = in_head.cur_cpu;
  }

  // Increment the new position of the head by 'jump'.
  inline void Jump(int jump) { position += jump; Adjust(); }
  inline void AbsJump(int jump) { position += jump; }

  inline int GetPosition() const { return position; }
  inline const cCodeArray & GetMemory() const;
  inline cBaseCPU * GetCurCPU() { return cur_cpu; }
  inline cBaseCPU * GetMainCPU() { return main_cpu; }
  inline int GetCurID() const;
  inline UCHAR GetInst();
  inline UCHAR GetInst(int offset);

  inline void SetInst(UCHAR in_char);
  inline UCHAR GetNextInst();
  inline void SetFlag(int flag);

  // Operator Overloading...
  inline cCPUHead & operator=(const cCPUHead & in_cpu_head);
  inline cCPUHead & operator++() { position++; Adjust(); return *this; }
  inline cCPUHead & operator--() { position--; Adjust(); return *this; }
  inline cCPUHead & operator++(int) { return operator++(); }
  inline cCPUHead & operator--(int) { return operator--(); }
  inline int operator-(const cCPUHead & in_cpu_head) {
    if (cur_cpu != in_cpu_head.cur_cpu) return 0;
    else return position - in_cpu_head.position;
  }
  inline int operator==(const cCPUHead & in_cpu_head) const {
    return
      (cur_cpu == in_cpu_head.cur_cpu) && (position == in_cpu_head.position);
  }

  // Bool Tests...
  inline int AtFront() { return (position == 0); }
  inline int AtEnd() { return (position + 1 == GetMemory().GetSize()); }
  inline int InMemory()
    { return (position >= 0 && position < GetMemory().GetSize()); }

  // Test functions...
  inline int TestParasite() const;
};

#endif
