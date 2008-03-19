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
#include "../main/tasks.hh"  // for cIOBuf

#include "label.hh"
#include "head.hh"

class cInstLib;

// Each creature has an cHardware structure which keeps track of the
// current status of all the components of the simulated hardware.

class cCPUStack { 
private:
  int stack[STACK_SIZE];
  UCHAR stack_pointer;  
public:
  cCPUStack();
  cCPUStack(const cCPUStack & in_stack);
  ~cCPUStack();

  void operator=(const cCPUStack & in_stack);

  inline void Push(int value);
  inline int Pop();
  inline int Get(int depth=0) const;
  inline void Clear();
  void Flip();

  int OK();
};

struct cCPUThread {
private:
  int id;
public:
  int reg[NUM_REGISTERS];
  cCPUHead inst_pointer;
  cCPUStack stack;
  int cur_stack;              // 0 = local stack, 1 = global stack.

  UCHAR input_pointer;
  cIOBuf input_buf;
  cIOBuf output_buf;
  cCodeLabel next_label;
public:
  cCPUThread(cBaseCPU * in_cpu=NULL, int _id=-1);
  cCPUThread(const cCPUThread & in_thread, int _id=-1);
  ~cCPUThread();

  void operator=(const cCPUThread & in_thread);

  void Reset(cBaseCPU * in_cpu, int _id);
  int GetID() { return id; }
  void SetID(int _id) { id = _id; }
};

///////////////////////////////////////////////////////////////////
// The following is a speed up for single-threaded runs.
//
// The variable cur_thread need only be used in multi-threaded runs.  When
// we are only going to have a single threads, we can allow cur_thread to
// always be 0.
///////////////////////////////////////////////////////////////////

#ifdef THREADS   // Multiple Threads Allowed
#define CUR_THREAD cur_thread

#ifdef SINGLE_IO_BUFFER   // For Single IOBuffer vs IOBuffer for each Thread
#define IO_THREAD 0
#else
#define IO_THREAD cur_thread
#endif

#else  // Single Thread
#define CUR_THREAD 0
#define IO_THREAD 0
#endif



class cHardware { 
private:
  int input_buffer[IO_SIZE];  // Inputs...
  cCodeArray memory;          // Code...
  cCPUStack global_stack;     // A stack which all threads share.
  cInstLib * inst_lib;        // instruction library...
  int time_used;
  int thread_time_used;

  cCPUThread * threads;
  cFlags id_chart;
  int num_threads;
  int cur_thread;
public:
  cHardware(cBaseCPU * parent_cpu);
  ~cHardware();

  void Reset(cBaseCPU * in_cpu = NULL);

  int OK();
  void PrintStatus(int _id = -1);

  // Helper methods
  inline void SetupProcess();

  // Stack Manipulation...
  inline void StackPush(int value);
  inline int StackPop();
  inline void StackFlip();
  inline int GetStack(int depth=0);
  inline void StackClear();
  inline void SwitchStack();
  inline int GetActiveStack() { return threads[CUR_THREAD].cur_stack; }

  // Tasks & IO
  void DoInput(const int _input, cPhenotype & phenotype){
    threads[IO_THREAD].input_buf.Add(_input);
    phenotype.TestInput(threads[IO_THREAD].input_buf, 
			threads[IO_THREAD].output_buf); }
  void DoOutput(const int _output, cPhenotype & phenotype){
    threads[IO_THREAD].output_buf.Add(_output);
    phenotype.TestOutput(threads[IO_THREAD].input_buf, 
			 threads[IO_THREAD].output_buf); }

  // Instruction Pointer Manipulation
  inline void AdvanceIP();
  inline void AdjustIP();
  inline void JumpIP(int in_distance);
  inline void AbsJumpIP(int in_distance);
  inline void SetIP(int in_position, cBaseCPU * in_cpu=NULL);
  inline void SetIP(const cCPUHead & in_head)
    { threads[CUR_THREAD].inst_pointer.Set(in_head); }
  inline void SetIPFlag(int flag);

  inline const cInstruction & GetCurInst();
  inline const cInstruction & GetNextInst();
  inline int GetIPPosition();
  inline void SetCurInst(const cInstruction & in_inst);


  // Instruction Library
  inline const cInstLib & GetInstLib() { return *inst_lib; }

  inline void SetInstLib(cInstLib * in_inst_lib)
    { inst_lib = in_inst_lib; }
  inline int GetNumInst();
  inline cInstruction GetRandomInst();


  // Instruction Arguments
  inline cInstruction  ReadArg(int arg_id, cPhenotype & phenotype);
  inline void WriteArg(int arg_id, const cInstruction & in_inst,
		       cPhenotype & phenotype);
  inline void LoadArgs(cPhenotype & phenotype);
  inline cInstruction  ReadRedcodeArg(int arg_id, cPhenotype & phenotype);
  inline void WriteRedcodeArg(int arg_id, const cInstruction & in_inst,
			      cPhenotype & phenotype);
  inline void LoadRedcodeArgs(cPhenotype & phenotype);

  // Label Manipulation
  inline void ReadLabel(int max_size=MAX_LABEL_SIZE);
  inline void ComplementLabel()
    { threads[CUR_THREAD].next_label.Complement(); }
  inline int GetLabelSize()
    { return threads[CUR_THREAD].next_label.GetSize(); }
  inline int GetTrinaryLabel()
    { return threads[CUR_THREAD].next_label.GetTrinary(); }

  // Register Manipulation
  inline void Reg_Inc(int reg_id) { threads[CUR_THREAD].reg[reg_id]++; }
  inline void Reg_Dec(int reg_id) { threads[CUR_THREAD].reg[reg_id]--; }
  inline void Reg_ShiftR(int reg_id) { threads[CUR_THREAD].reg[reg_id] >>= 1; }
  inline void Reg_ShiftL(int reg_id) { threads[CUR_THREAD].reg[reg_id] <<= 1; }
  inline void Reg_Set(int reg_id, int value) 
    { threads[CUR_THREAD].reg[reg_id] = value; }

  // Memory Manipulation
  inline int GetMemorySize() { return memory.GetSize(); }
  inline void ResizeMemory(int in_size) { memory.Resize(in_size); }
  inline void SetMemory(int pos, const cInstruction & value)
    { memory[pos] = value; }
  inline void InsertMemory(int pos, const cInstruction & value)
    { memory.Insert(pos, value); }
  inline void RemoveMemory(int pos)
    { memory.Remove(pos); }
  inline const cInstruction & GetMemData(int pos) { return memory[pos]; }
  inline int GetMemFlag(int pos, int flag) { return memory.GetFlag(pos, flag);}
  inline int GetCurMemFlag(int flag)
    { return memory.GetFlag(threads[CUR_THREAD].inst_pointer.GetPosition(), flag); }
  inline void SetMemFlag(int pos, int flag) { memory.SetFlag(pos, flag); }
  inline void UnsetMemFlag(int pos, int flag) { memory.UnsetFlag(pos, flag); }
  inline void ToggleMemFlag(int pos, int flag)
    { memory.ToggleFlag(pos, flag); }
  inline void ResetMemory(int value) { memory.Reset(value); }
  inline void Memory_CopyData(const cCodeArray & in_mem)
    { memory.CopyData(in_mem); }

  // IO Manipulation
  inline int GetInput() {
    int result = input_buffer[threads[IO_THREAD].input_pointer++];
    if (threads[IO_THREAD].input_pointer >= IO_SIZE)
      threads[IO_THREAD].input_pointer -= IO_SIZE;
    return result;
  }
  inline int GetInput(int depth);

  inline void RandomizeInputs() {
    for (int i = 0; i < IO_SIZE; i++) {
      const UINT value = g_random.GetUInt(1 << 30);
      input_buffer[i] += (value & ~255);
    }
  }
  inline void SetupTestInputs() {
    for (int i = 0; i < IO_SIZE; i++) {
      int value = input_buffer[i];
      input_buffer[i] += value << i + 8;
      input_buffer[i] += value << 22 - i;
    }
  }

  // Thread Manipulation
  int ForkThread(); // Adds a new thread based off of cur_thread.
  int KillThread(); // Kill the current thread!
  inline void PrevThread(); // Shift the current thread in use.
  inline void NextThread();

  // Keeping things in range...
  inline int RegRange(int id) {
    id %= NUM_REGISTERS; if (id < 0) id *= -1; return id;
  }

  // Tests

  inline int TestParasite();

  // Accessors
  inline int GetRegister(int reg_id) const
    { return threads[CUR_THREAD].reg[reg_id]; }
  inline const cCPUHead & GetInstPointer() const 
    { return threads[CUR_THREAD].inst_pointer; }
  inline const cCPUHead & GetInstPointer(int thread_id) const
    { return threads[thread_id].inst_pointer; }
  inline const cCodeLabel & GetLabel() const
    { return threads[CUR_THREAD].next_label; }
  inline const cCodeArray & GetMemory() const { return memory; }
  inline int GetTimeUsed() { return time_used; }
  inline int GetThreadTimeUsed() { return thread_time_used; }
  inline int GetNumThreads() { return num_threads; }
  inline int GetCurThread() { return CUR_THREAD; }
  inline int GetCurThreadID() { return threads[CUR_THREAD].GetID(); }

  inline void IncTimeUsed() {
    time_used++;
#ifdef THREADS
    if (num_threads > 1) thread_time_used++;
#endif
  }

  // Complex label manipulation...
  cCPUHead FindLabel(int direction);
  int FindLabel_Forewards(const cCodeLabel & search_label,
			  const cCodeArray & search_mem, int pos);
  int FindLabel_Backwards(const cCodeLabel & search_label,
			  const cCodeArray & search_mem, int pos);
  cCPUHead FindLabel(cCodeLabel * in_label, int direction);
  cCPUHead FindFullLabel(const cCodeLabel & in_label, int direction);
  void InjectCode(cCodeArray * inject_code, int line_num);

  cCPUHead FindArgLabel(int in_label);
};

////////////////
//  cCPUStack
////////////////

inline void cCPUStack::Push(int value)
{
  if (stack_pointer == 0) stack_pointer = STACK_SIZE - 1;
  else stack_pointer--;
  stack[stack_pointer] = value;
}

inline int cCPUStack::Pop()
{
  int value = stack[stack_pointer];
  stack[stack_pointer] = 0;
  stack_pointer++;
  if (stack_pointer == STACK_SIZE) stack_pointer = 0;
  return value;
}

inline int cCPUStack::Get(int depth) const
{
  int array_pos = depth + stack_pointer;
  if (array_pos >= STACK_SIZE) array_pos -= STACK_SIZE;
  return stack[array_pos];
}

inline void cCPUStack::Clear()
{
  for (int i =0; i < STACK_SIZE; i++) { stack[i] = 0; }
  stack_pointer = 0;
}


///////////////
//  cHardware
///////////////

inline void cHardware::NextThread()
{
#ifdef THREADS
  cur_thread++;
  if (cur_thread >= num_threads) cur_thread = 0;
#endif
}

inline void cHardware::PrevThread()
{
#ifdef THREADS
  if (cur_thread == 0) cur_thread = num_threads - 1;
  else cur_thread--;
#endif
}

inline void cHardware::StackPush(int value)
{
  if (threads[CUR_THREAD].cur_stack == 0) {
    threads[CUR_THREAD].stack.Push(value);
  } else {
    global_stack.Push(value);
  }
}

inline int cHardware::StackPop()
{
  int pop_value;

  if (threads[CUR_THREAD].cur_stack == 0) {
    pop_value = threads[CUR_THREAD].stack.Pop();
  } else {
    pop_value = global_stack.Pop();
  }

  return pop_value;
}

inline void cHardware::StackFlip()
{
  if (threads[CUR_THREAD].cur_stack == 0) {
    threads[CUR_THREAD].stack.Flip();
  } else {
    global_stack.Flip();
  }
}

inline int cHardware::GetStack(int depth)
{
  int value;

  if (threads[CUR_THREAD].cur_stack == 0) {
    value = threads[CUR_THREAD].stack.Get(depth);
  } else {
    value = global_stack.Get(depth);
  }

  return value;
}

inline void cHardware::StackClear()
{
  if (threads[CUR_THREAD].cur_stack == 0) {
    threads[CUR_THREAD].stack.Clear();
  } else {
    global_stack.Clear();
  }
}

inline void cHardware::SwitchStack()
{
  threads[CUR_THREAD].cur_stack++;
  if (threads[CUR_THREAD].cur_stack > 1) threads[CUR_THREAD].cur_stack = 0;
}

inline int cHardware::GetInput(int depth)
{
  int array_pos = depth + threads[IO_THREAD].input_pointer;
  if (array_pos >= IO_SIZE) array_pos -= IO_SIZE;
  return input_buffer[array_pos];
}


#endif
