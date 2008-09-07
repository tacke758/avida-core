//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CPU_HH
#define CPU_HH

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

struct sReproData {
  cGenotype * parent_genotype;
  cBaseCPU * parent_cpu;
  cCodeArray child_memory;
  int copied_size;
  int executed_size;
  int gestation_time;
  double fitness;
  cPhenotype * parent_phenotype;

  // flags... (move to a proper flag structure if we get more than one)

  int copy_true;     // Is this a proper copy of the mother?
};


// The cBaseCPU class controls the running and manages all the statistics about
// a creature.  Effectively the chemistry acting on the genome.

class cBaseCPU : public cCreature {
protected:
  cInstLib * inst_lib;        // instruction library...
  cFlags flags;

  cHardware hardware;
  sReproData repro_data;

  cEnvironment * environment;

  cPhenotype phenotype;

  // A few useful functions

  void UseResources();
  void ResetVariables();
  void PrintStatus();

  // Divide functions

  inline int  Divide_CheckViable(int child_size, int parent_size);
  inline void Divide_RecordInfo();
  inline void Divide_SetupChild(int parent_size);
  inline void Divide_DoMutations();
  inline void Divide_CheckCopyTrue();
    
  // Functions for Instruction Implementation

  inline cCPUHead FindFullLabel(const cCodeLabel & in_label, int direction=1);
public:
  void Inst_Nop();

  // Flow Control
  void Inst_IfNot0();
  void Inst_IfNEqu();
  void Inst_IfBit1();
  void Inst_JumpF();
  void Inst_JumpB();
  void Inst_JumpP();
  void Inst_Call();
  void Inst_Return();

  // Single-Argument Math
  void Inst_ShiftR();
  void Inst_ShiftL();
  void Inst_Bit1();
  void Inst_SetNum();
  void Inst_Inc();
  void Inst_Dec();
  void Inst_Zero();
  void Inst_Pop();
  void Inst_Push();
  void Inst_Not();

  // Double Argument Math
  void Inst_Add();
  void Inst_Sub();
  void Inst_Nand();
  void Inst_Nor();
  void Inst_And();
  void Inst_Order();
  void Inst_Xor();

  // Biological
  void Inst_Copy();
  void Inst_ReadInst();
  void Inst_WriteInst();
  void Inst_Compare();
  void Inst_IfNCpy();
  void Inst_Allocate();
  void Inst_Divide();
  void Inst_CAlloc();
  void Inst_CDivide();
  void Inst_Inject();
  
  // I/O and Sensory
  void Inst_TaskGet();
  void Inst_TaskPut();
  void Inst_SearchF();
  void Inst_SearchB();

  // Environment

  void Inst_RotateL();
  void Inst_RotateR();
  void Inst_SetCopyMut();
  void Inst_ModCopyMut();

  // Alternate Caps to be placed on inst_set...

  void Notify(); // Inform the population object on every command executed.

public:
  cBaseCPU(int in_cpu_test = 0);
  ~cBaseCPU();

  cGenotype * LoadCode(char * filename);
  void ChangeGenotype(cGenotype * in_genotype);
  int TestProcess();
  inline void SingleProcess();

  int OK();
  void ActivateChild();
  void Kill();

  double GetFitness();
  inline double CalcMeritRatio() {
    double ratio = (info.age) ?
      (phenotype.GetMerit().GetDouble() / ((double) info.age)) :
      (phenotype.GetMerit().GetDouble() * 2);
    return ratio; // * g_random.GetDouble(0.9, 1.1);
  }

  void Mutate();
  int Mutate(int mut_point);
  void Print(char * filename);

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
  inline const cCPUHead & GetInstPointer() { return hardware.GetInstPointer();}
  inline const cCodeArray & GetMemory() { return hardware.GetMemory(); }
  inline int GetRegister(int reg)  { return hardware.GetRegister(reg); }
  inline int GetStack(int depth=0) { return hardware.GetStack(depth); }
  inline int GetInput(int depth) { return hardware.GetInput(depth); }
  inline int GetMemorySize()       { return hardware.GetMemory().GetSize(); }
  inline int GetTotalTimeUsed()    { return info.total_time_used; }
  inline int GetNumDivides()       { return info.num_divides; }
  inline int GetNumErrors()        { return info.num_errors; }
  inline int GetAge()              { return info.age; }
  inline int GetGestationTime()
    { return info.active_genotype->GetGestationTime(); }
    // { return info.gestation_time; }
  inline int GetCurGestation()
    { return info.total_time_used - info.gestation_start; }
  inline cMerit GetMerit() { return phenotype.GetMerit(); }
  inline cMerit GetCurMerit() { return phenotype.GetCurMerit(); }
  inline int GetFlag(int flag_num) { return flags.GetFlag(flag_num); }
  inline cPhenotype * GetPhenotype() { return &phenotype; }

  inline void SetParentTrue(int in_parent_true)
    { flags(CPU_FLAG_PARENT_TRUE, in_parent_true); }
  inline void SetParasite(int in_parasite=TRUE)
    { flags(CPU_FLAG_PARASITE, in_parasite); }
  inline void SetCopiedSize(int in_copied_size)
    { info.copied_size = in_copied_size; }
  inline void SetExecutedSize(int in_executed_size)
    { info.executed_size = in_executed_size; }
  inline void SetInstLib(cInstLib * in_inst_lib)
    { inst_lib = in_inst_lib; }
  inline void SetFitness(double in_fitness)
    { info.fitness = in_fitness; }
  inline void SetGestationTime(int in_gest)
    { info.gestation_time = in_gest; }
  inline void InitPhenotype(cPhenotype * base_phenotype)
    { phenotype.Clear(info, base_phenotype->GetMerit()); }

  inline void SetMemory(int pos, char value) { hardware.SetMemory(pos, value);}
  inline void SetMemFlag(int pos, int flag) { hardware.SetMemFlag(pos, flag); }

  inline void SetNext(cBaseCPU * in_next) { info.next = in_next; }
  inline void SetPrev(cBaseCPU * in_prev) { info.prev = in_prev; }

  inline void IncAge() { info.age++; }

  inline char GetBasicSymbol();
  inline char GetSpeciesSymbol();
  inline char GetInjectSymbol();
  inline char GetResourceSymbol(int resource_num = 0);
  inline char GetAgeSymbol();
  inline char GetBreedSymbol();
  inline char GetParasiteSymbol();
  inline char GetPointMutSymbol();
};

#include "../main/inst_lib.hh"

#endif
