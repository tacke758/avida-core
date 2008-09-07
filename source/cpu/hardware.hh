//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef HARDWARE_HH
#define HARDWARE_HH

#include "../defs.hh"
#include "cpu_defs.hh"
#include "../main/code_array.hh" // Shouldn't code_array be in cpu dir ?
#include "../main/phenotype.hh"

#include "label.hh"
#include "head.hh"

// Each creature has an cHardware structure which keeps track of the
// current status of all the components of the simulated hardware.

class cHardware {
  //public:
private:
  int reg[NUM_REGISTERS];
  cCPUHead inst_pointer;
  int stack[STACK_SIZE];
  UCHAR stack_pointer;

  int input_buffer[IO_SIZE];
  UCHAR input_pointer;
  cCodeArray memory;

  cCodeLabel next_label;
public:
  cHardware();
  ~cHardware();

  void Reset(cBaseCPU * in_cpu = NULL);
  int OK();

  // Helper methods
  inline int FindModifiedRegister(int default_register);

  // Stack Manipulation...
  inline void StackPush(int value);
  inline int StackPop();
  inline int GetStack(int depth=0);

  // Instruction Pointer Manipulation
  inline void AdvanceIP() { inst_pointer++; }
  inline void AdjustIP() { inst_pointer.Adjust(); }
  inline void JumpIP(int in_distance) { inst_pointer.Jump(in_distance); }
  inline void SetIP(int in_position, cBaseCPU * in_cpu=NULL)
    { inst_pointer.Set(in_position, in_cpu); }
  inline void SetIP(const cCPUHead & in_head) { inst_pointer = in_head; }
  inline void SetIPFlag(int flag) { inst_pointer.SetFlag(flag); }

  inline char GetCurInst() { return inst_pointer.GetInst(); }
  inline int GetIPPosition() { return inst_pointer.GetPosition(); }

  // Label Manipulation
  inline void ReadLabel(int max_size=MAX_LABEL_SIZE);
  inline void ComplementLabel() { next_label.Complement(); }
  inline int GetLabelSize() { return next_label.GetSize(); }
  inline int GetTrinaryLabel() { return next_label.GetTrinary(); }

  // Register Manipulation
  inline void Reg_Inc(int reg_id) { reg[reg_id]++; }
  inline void Reg_Dec(int reg_id) { reg[reg_id]--; }
  inline void Reg_ShiftR(int reg_id) { reg[reg_id] >>= 1; }
  inline void Reg_ShiftL(int reg_id) { reg[reg_id] <<= 1; }
  inline void Reg_Set(int reg_id, int value) { reg[reg_id] = value; }

  // Memory Manipulation
  inline int GetMemorySize() { return memory.GetSize(); }
  inline void ResizeMemory(int in_size) { memory.Resize(in_size); }
  inline void SetMemory(int pos, char value) { memory[pos] = value; }
  inline char GetMemData(int pos) { return memory[pos]; }
  inline int GetMemFlag(int pos, int flag) { return memory.GetFlag(pos, flag);}
  inline void SetMemFlag(int pos, int flag) { memory.SetFlag(pos, flag); }
  inline void ResetMemory(int value) { memory.Reset(value); }
  inline void Memory_CopyData(const cCodeArray & in_mem)
    { memory.CopyData(in_mem); }

  // IO Manipulation
  inline int GetInput() {
    int result = input_buffer[input_pointer++];
    if (input_pointer >= IO_SIZE) input_pointer -= IO_SIZE;
    return result;
  }
  inline int GetInput(int depth);
  inline void DoInput(cPhenotype & phenotype) { phenotype.AddGet(); }
  inline void DoOutput(cPhenotype & phenotype, int value) {
    phenotype.SetTasks(input_buffer, IO_SIZE, value);
    phenotype.AddPut();
  }
  inline void RandomizeInputs() {
    for (int i = 0; i < IO_SIZE; i++)
      input_buffer[i] += (g_random.GetUInt(1 << 30) & ~255);
  }


  // Tests

  inline int TestParasite() { return inst_pointer.TestParasite(); }

  // Accessors
  inline int GetRegister(int reg_id) const { return reg[reg_id]; }
  inline const cCPUHead & GetInstPointer() const { return inst_pointer; }
  inline const cCodeLabel & GetLabel() const { return next_label; }
  inline const cCodeArray & GetMemory() const { return memory; }

  cCPUHead FindLabel(int direction);
  cCPUHead FindLabel(cCodeLabel * in_label, int direction);
  cCPUHead FindFullLabel(const cCodeLabel & in_label, int direction);
  void InjectCode(cCodeArray * inject_code, int line_num);
};


int cHardware::FindModifiedRegister(int default_register)
{
  if (!inst_pointer.AtEnd() && inst_pointer.GetNextInst() < NUM_NOPS) {
    inst_pointer++;
    default_register = inst_pointer.GetInst();
    inst_pointer.SetFlag(INST_FLAG_EXECUTED);   // Mark nop as executed. (?)
  }
  return default_register;
}


// This function looks at the current position in the info of
// a creature, and sets the next_label to be the sequence of nops
// which follows.

inline void cHardware::ReadLabel(int max_size)
{
  int count = 0;
  
  next_label.Clear();
  
  while (!inst_pointer.AtEnd() && inst_pointer.GetNextInst() < NUM_NOPS &&
	 (count++ < max_size)) {
    inst_pointer++;
    next_label.AddNop(inst_pointer.GetInst());
    
    // If this is the first line of the template, mark it executed.
    if (next_label.GetSize() == 1)
      inst_pointer.SetFlag(INST_FLAG_EXECUTED);
  }
}

inline void cHardware::StackPush(int value)
{
  if (!stack_pointer) stack_pointer = STACK_SIZE - 1;
  else --stack_pointer;
  stack[stack_pointer] = value;
}

inline int cHardware::StackPop()
{
  int value = stack[stack_pointer];
  stack[stack_pointer] = 0;
  stack_pointer++;
  if (stack_pointer == STACK_SIZE) stack_pointer = 0;
  return value;
}

inline int cHardware::GetStack(int depth)
{
  int array_pos = depth + stack_pointer;
  if (array_pos >= STACK_SIZE) array_pos -= STACK_SIZE;
  return stack[array_pos];
}

inline int cHardware::GetInput(int depth)
{
  int array_pos = depth + input_pointer;
  if (array_pos >= IO_SIZE) array_pos -= IO_SIZE;
  return input_buffer[array_pos];
}

#endif
