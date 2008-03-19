/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_HEAD_HH
#define CPU_HEAD_HH

#include "../defs.hh"
#include "../main/code_array.hh"

/**
 * The cCPUHead class contains a pointer to locations in memory for a CPU.
 **/

class cCPUHead {
private:
  cBaseCPU * main_cpu;
  cBaseCPU * cur_cpu;
  int position;

  int FindLabel_Forward(const cCodeLabel & search_label,
				 const cCodeArray & search_mem, int pos);
  int FindLabel_Backward(const cCodeLabel & search_label,
				 const cCodeArray & search_mem, int pos);
public:
  cCPUHead();
  cCPUHead(cBaseCPU * in_cpu, int in_pos = 0);
  cCPUHead(const cCPUHead & in_cpu_head);
  //inline ~cCPUHead() { ; }

  inline void Reset(cBaseCPU * new_cpu = NULL);

    /**
     * This function keeps the position within the range of the current memory.
     **/
    void Adjust();  // @TCC - ?inline?

    /**
     * Set the new position of the head (and adjust it into range in Set()).
     **/
    inline void Set(int new_pos, cBaseCPU * in_cpu = NULL);
  
    inline void AbsSet(int new_pos) { position = new_pos; }
  
    inline void Set(const cCPUHead & in_head) {
	position = in_head.position;
	cur_cpu  = in_head.cur_cpu;
    }

    /**
     * Increment the new position of the head by 'jump'.
     **/
    inline void Jump(int jump);
  
    inline void LoopJump(int jump);
  
    inline void AbsJump(int jump);

  // Other manipulation functions.
  inline void Advance();
  cCPUHead FindLabel(const cCodeLabel & label, int direction=1);

  // Accessors.
  inline int GetPosition() const { return position; }
  inline const cCodeArray & GetMemory() const;
  inline cBaseCPU * GetCurCPU() const { return cur_cpu; }
  inline cBaseCPU * GetMainCPU() const { return main_cpu; }
  inline int GetCurID() const;
  inline int GetMainID() const;
  inline const cInstruction & GetInst() const;
  inline const cInstruction & GetInst(int offset) const;
  inline int GetFlag(int id) const;

  inline void SetInst(const cInstruction & value);
  inline void SetArgValue(int arg_id, int value);
  inline void InsertInst(const cInstruction & in_char);
  inline void RemoveInst();
  inline const cInstruction & GetNextInst();
  inline void SetFlag(int flag);
  inline void UnsetFlag(int flag);
  inline void ToggleFlag(int flag);

  // Operator Overloading...
  inline cCPUHead & operator=(const cCPUHead & in_cpu_head);
  inline cCPUHead & operator++();
  inline cCPUHead & operator--();
  inline cCPUHead & operator++(int);
  inline cCPUHead & operator--(int);
  inline int operator-(const cCPUHead & in_cpu_head) {
    if (cur_cpu != in_cpu_head.cur_cpu) return 0;
    else return position - in_cpu_head.position;
  }
  inline int operator==(const cCPUHead & in_cpu_head) const {
    return
      (cur_cpu == in_cpu_head.cur_cpu) && (position == in_cpu_head.position);
  }

  // Bool Tests...
  inline int AtFront() const { return (position == 0); }
  inline int AtEnd() const;
  inline int InMemory() const;

  // Test functions...
  inline int TestParasite() const;
};

#endif

