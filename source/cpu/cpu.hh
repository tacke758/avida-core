//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_HH
#define CPU_HH

#define FAULT_LOC_DEFAULT      0
#define FAULT_LOC_INSTRUCTION  1
#define FAULT_LOC_JUMP         2
#define FAULT_LOC_INJECT       3
#define FAULT_LOC_THREAD_FORK  4
#define FAULT_LOC_THREAD_KILL  5
#define FAULT_LOC_ALLOC        6
#define FAULT_LOC_DIVIDE       7

#include "../defs.hh"
#include "cpu_defs.hh"
#include "../main/code_array.hh" // Shouldn't code_array be in cpu dir ?
#include "../main/genotype.hh"   // for GetGestationTime() only 
#include "../main/phenotype.hh"

#include "label.hh"
#include "head.hh"

#include "creature.hh"
#include "hardware.hh"

class cInstLib;
class cEnvironment;

// The sReproData structure carries the information to be sent back to the
// population about the child which this CPU is producing.  It contains all
// the information needed to create the child.

class sReproData { 
public:
  cGenotype * parent_genotype;
  cBaseCPU * parent_cpu;
  cCodeArray child_memory;
  int copied_size;
  int executed_size;
  int gestation_time;
  double fitness;
  int parent_generation;
  cMerit parent_merit;
  double neutral_metric;
  int lineage_label;

  // flags... (move to a proper flag structure if we get more than one)
  UCHAR copy_true;     // Is this a proper copy of the mother?
};


// The cBaseCPU class controls the running and manages all the statistics about
// a creature.  Effectively the chemistry acting on the genome.

class cBaseCPU : public cCreature { 
  friend class cTestCPU;
protected:
  cFlags flags;

  cHardware hardware;
  sReproData repro_data;

  cEnvironment * environment;

  cPhenotype * phenotype;

  // ----= A few useful functions =----

  void UseResources();
  void ResetVariables();
  inline void PrintStatus();

  // Allocate function

  int Allocate_Main(int allocated_size);

  // Divide functions

  void Divide_Main(int divide_point);
  int  Divide_CheckViable(int child_size, int parent_size);
  void Divide_RecordInfo();
  void Divide_SetupChild(int parent_size);
  void Divide_DoMutations();
  void Divide_CheckCopyTrue();
    
  // Functions for Instruction Implementation

  inline cCPUHead FindFullLabel(const cCodeLabel & in_label, int direction=1);
  inline cInstruction ReadArg(int arg_id);
  inline int ReadArgValue(int arg_id);
  inline void WriteArg(int arg_id, const cInstruction & in_inst);
  inline void WriteArg(int arg_id, int value);
  inline void LoadArgs();
  inline cInstruction ReadRedcodeArg(int arg_id);
  inline int ReadRedcodeArgValue(int arg_id);
  inline void WriteRedcodeArg(int arg_id, const cInstruction & in_inst);
  inline void WriteRedcodeArg(int arg_id, int value);
  inline void LoadRedcodeArgs();

  // Other Special Functions
  void Fault(int fault_loc, int fault_type);
public:
  void Inst_Nop();

  // Flow Control
  void Inst_If0();
  void Inst_IfEqu();
  void Inst_IfNot0();
  void Inst_IfNEqu();
  void Inst_IfGr0();
  void Inst_IfGr();
  void Inst_IfGrEqu0();
  void Inst_IfGrEqu();
  void Inst_IfLess0();
  void Inst_IfLess();
  void Inst_IfLsEqu0();
  void Inst_IfLsEqu();
  void Inst_IfBit1();
  void Inst_IfANotEqB();
  void Inst_IfBNotEqC();
  void Inst_IfANotEqC();

  void Inst_JumpF();
  void Inst_JumpB();
  void Inst_JumpP();
  void Inst_JumpSelf();
  void Inst_Call();
  void Inst_Return();

  // Stack and Register Operations
  void Inst_Pop();
  void Inst_Push();

  void Inst_PopA();
  void Inst_PopB();
  void Inst_PopC();
  void Inst_PushA();
  void Inst_PushB();
  void Inst_PushC();

  void Inst_SwitchStack();
  void Inst_FlipStack();
  void Inst_Swap();
  void Inst_SwapAB();
  void Inst_SwapBC();
  void Inst_SwapAC();
  void Inst_CopyReg();
  void Inst_CopyRegAB();
  void Inst_CopyRegAC();
  void Inst_CopyRegBA();
  void Inst_CopyRegBC();
  void Inst_CopyRegCA();
  void Inst_CopyRegCB();
  void Inst_Reset();

  // Single-Argument Math
  void Inst_ShiftR();
  void Inst_ShiftL();
  void Inst_Bit1();
  void Inst_SetNum();
  void Inst_Inc();
  void Inst_Dec();
  void Inst_Zero();
  void Inst_Not();
  void Inst_Neg();
  void Inst_Square();
  void Inst_Sqrt();

  // Double Argument Math
  void Inst_Add();
  void Inst_Sub();
  void Inst_Mult();
  void Inst_Div();
  void Inst_Mod();
  void Inst_Nand();
  void Inst_Nor();
  void Inst_And();
  void Inst_Order();
  void Inst_Xor();

  // Biological
  void Inst_Copy();
  void Inst_ReadInst();
  void Inst_WriteInst();
  void Inst_StackReadInst();
  void Inst_StackWriteInst();
  void Inst_Compare();
  void Inst_IfNCpy();
  void Inst_Allocate();
  void Inst_Divide();
  void Inst_CAlloc();
  void Inst_CDivide();
  void Inst_Inject();
  void Inst_Repro();
  
  // I/O and Sensory
  void Inst_TaskGet();
  void Inst_TaskStackGet();
  void Inst_TaskStackLoad();
  void Inst_TaskPut();
  void Inst_SearchF();
  void Inst_SearchB();
  void Inst_MemSize();

  // Environment

  void Inst_RotateL();
  void Inst_RotateR();
  void Inst_SetCopyMut();
  void Inst_ModCopyMut();

  // Multi-threading...

  void Inst_ForkThread();
  void Inst_KillThread();
  void Inst_ThreadID();

  // Direct Matching Templates

  void Inst_DMJumpF();
  void Inst_DMJumpB();
  void Inst_DMCall();
  void Inst_DMSearchF();
  void Inst_DMSearchB();

  // Relative Addressed Jumps

  void Inst_REJumpF();
  void Inst_REJumpB();

  // Absoulte Addressed Jumps

  void Inst_ABSJump();

  // Binary Inst Set
  void Inst_Bin_Repro();


  //       -=-=  Instructions w/ Arguments  =-=-

  void InstArg_Add();
  void InstArg_And();
  void InstArg_Nand();
  void InstArg_IfEqu();
  void InstArg_IfLess();

  void InstArg_Copy();
  void InstArg_Swap();
  void InstArg_Shift();

  void InstArg_Nop();
  void InstArg_Allocate();
  void InstArg_Divide();
  void InstArg_Rotate();
  void InstArg_Label();


  // Redcode Instructions...

  void InstRed_Nop();
  void InstRed_Data();
  void InstRed_Move();
  void InstRed_Add();
  void InstRed_Sub();
  void InstRed_Nand();
  void InstRed_Jump();
  void InstRed_JumpZero();
  void InstRed_JumpNZero();
  void InstRed_DecJumpNZero();
  void InstRed_Compare();
  void InstRed_Split();
  void InstRed_SkipLess();
  void InstRed_Allocate();
  void InstRed_Divide();
  void InstRed_Get();
  void InstRed_Put();

  // Alternate Caps to be placed on inst_set...

  void Notify(); // Inform the population object on every command executed.

public:
  cBaseCPU(int in_cpu_test = 0);
  ~cBaseCPU();

  cGenotype * LoadCode(const char * filename);
  cGenotype * LoadCode(const cCodeArray & in_code);
  void ChangeGenotype(cGenotype * in_genotype);

  inline void SingleProcess();

  // Input & Output Testing  
  void DoInput(const int value){ hardware.DoInput(value, *phenotype); }
  void DoOutput(const int value){ hardware.DoOutput(value, *phenotype); }

  inline int FindModifiedRegister(int default_register);
  inline cCPUHead GetHeadPosition(cBaseCPU * cur_cpu, int offset);

  int OK();
  void ActivateChild();
  void Kill();

  double GetFitness();
  inline double CalcMeritRatio() {
    double ratio = (info.age) ?
      (phenotype->GetMerit().GetDouble() / ((double) info.age)) :
      (phenotype->GetMerit().GetDouble() * 2);
    return ratio; // * g_random.GetDouble(0.9, 1.1);
  }

  int Mutate(int mut_point);
  inline void Mutate() { Mutate(g_random.GetUInt(hardware.GetMemorySize())); }

  void Print(const cString filename);

  // Interaction with other CPUs

  inline void InjectCode(cCodeArray * inject_code, int line_num);

  // Access to private variables

  inline cGenotype * GetActiveGenotype() { return info.active_genotype; }
  inline cCodeArray * GetChild()   { return &(repro_data.child_memory); }
  // inline int GetID()               { return environment->GetID(); }
  inline cBaseCPU * GetNext()      { return (cBaseCPU *) info.next; }
  inline cBaseCPU * GetPrev()      { return (cBaseCPU *) info.prev; }

  inline cEnvironment * GetEnvironment() { return environment; }

  inline int GetCopyTrue()         { return repro_data.copy_true; }
  inline int GetParentTrue()       { return flags[CPU_FLAG_PARENT_TRUE]; }
  inline int GetCopiedSize()       { return info.copied_size; }
  inline int GetExecutedSize()     { return info.executed_size; }
  inline const cInstLib & GetInstLib()     { return hardware.GetInstLib(); }
  inline const cCPUHead & GetInstPointer() { return hardware.GetInstPointer();}
  inline const cCPUHead & GetInstPointer(int in_thread)
    { return hardware.GetInstPointer(in_thread);}
  inline const cCodeArray & GetMemory() { return hardware.GetMemory(); }
  inline int GetRegister(int reg)  { return hardware.GetRegister(reg); }
  inline int GetStack(int depth=0) { return hardware.GetStack(depth); }
  inline int GetActiveStack()      { return hardware.GetActiveStack(); }
  inline int GetInput(int depth)   { return hardware.GetInput(depth); }
  inline int GetMemorySize()       { return hardware.GetMemory().GetSize(); }
  inline int GetTotalTimeUsed()    { return hardware.GetTimeUsed(); }
  inline int GetNumThreads()       { return hardware.GetNumThreads(); }
  inline int GetActiveThreadID()   { return hardware.GetCurThread(); }
  inline int GetNumDivides()       { return info.num_divides; }
  inline int GetNumErrors()        { return info.num_errors; }
  inline int GetAge()              { return info.age; }
  inline int GetGestationTime()
    // { return info.active_genotype->GetGestationTime(); }
    { return info.gestation_time; }
  inline int GetCurGestation()
    { return hardware.GetTimeUsed() - info.gestation_start; }
  inline int GetGestationStart() { return info.gestation_start; }
  inline cMerit GetMerit() { return phenotype->GetMerit(); }
  inline cMerit GetCurMerit() { return phenotype->GetCurMerit(); }
  inline int GetFlag(int flag_num) { return flags.GetFlag(flag_num); }
  inline cPhenotype * GetPhenotype() { return phenotype; }

  inline void SetParentTrue(int in_parent_true)
    { flags(CPU_FLAG_PARENT_TRUE, in_parent_true); }
  inline void SetParasite(int in_parasite=TRUE)
    { flags(CPU_FLAG_PARASITE, in_parasite); }
  inline void SetModified() { flags(CPU_FLAG_INST_INJECT, TRUE); }
  inline void SetInjected() { flags(CPU_FLAG_INJECTED, TRUE); }
  inline void SetCopiedSize(int in_copied_size)
    { info.copied_size = in_copied_size; }
  inline void SetExecutedSize(int in_executed_size)
    { info.executed_size = in_executed_size; }
  inline void SetInstLib(cInstLib * in_inst_lib)
    { hardware.SetInstLib(in_inst_lib); }
  inline void SetInstPointer(const cCPUHead & in_head)
    { hardware.SetIP(in_head); }
  inline void SetFitness(double in_fitness)
    { info.fitness = in_fitness; }
  inline void SetGestationTime(int in_gest)
    { info.gestation_time = in_gest; }
  inline void InitPhenotype(const sReproData & repro_data)
    { phenotype->Clear(repro_data, repro_data.parent_merit); }

  inline void SetMemory(int pos, const cInstruction & value)
    { hardware.SetMemory(pos, value);}
  inline void InsertMemory(int pos, const cInstruction & value)
    { hardware.InsertMemory(pos, value);}
  inline void RemoveMemory(int pos)
    { hardware.RemoveMemory(pos);}
  inline void SetMemFlag(int pos, int flag)
    { hardware.SetMemFlag(pos, flag); }
  inline void UnsetMemFlag(int pos, int flag)
    { hardware.UnsetMemFlag(pos, flag); }
  inline void ToggleMemFlag(int pos, int flag)
    { hardware.ToggleMemFlag(pos, flag); }

  inline void SetNext(cBaseCPU * in_next) { info.next = in_next; }
  inline void SetPrev(cBaseCPU * in_prev) { info.prev = in_prev; }

  inline void IncAge() { info.age++; }

  inline char GetBasicSymbol();
  inline char GetSpeciesSymbol();
  inline char GetModifiedSymbol();
  inline char GetResourceSymbol(int resource_num = 0);
  inline char GetAgeSymbol();
  inline char GetBreedSymbol();
  inline char GetParasiteSymbol();
  inline char GetPointMutSymbol();
  inline char GetThreadSymbol();
};

#include "../main/inst_lib.hh"

#endif
