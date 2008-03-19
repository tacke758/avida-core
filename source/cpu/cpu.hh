//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
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
#define FAULT_LOC_BCOPY        8

#include "../defs.hh"
#include "../tools/tArray.hh"
#include "cpu_defs.hh"
#include "cpu_stats.hh"
#include "../main/code_array.hh" // Shouldn't code_array be in cpu dir ?
#include "../main/genotype.hh"   // for GetGestationTime() only
#include "../main/phenotype.hh"

#include "label.hh"
#include "head.hh"

#include "creature.hh"
#include "hardware.hh"

class cInstLib;
class cEnvironment;
class cLineage;

/**
 * This class carries the information to be sent back to the
 * population about the child which this CPU is producing.  It contains all
 * the information needed to create the child.
 **/

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
  double parent_bonus;
  double neutral_metric;
  int lineage_label;
  cLineage *lineage;
  bool child_fertile;

  double parent_merit_base;
  tArray<int> * tasks;

  // flags... (move to a proper flag structure if we get more than one)
  bool copy_true;     // Is this a proper copy of the mother?

  sReproData(){ tasks = NULL; }

  void Clear(){
    parent_genotype = NULL;
    parent_cpu = NULL;
    child_memory.Clear();
    copied_size = 0;
    executed_size = 0;
    gestation_time = 0;
    fitness = 0;
    parent_generation = 0;
    parent_merit.Clear();
    neutral_metric = 0;
    lineage_label = 0;
    lineage = 0;
    copy_true = FALSE;
    parent_merit_base = 0;
    parent_bonus = 0;
    if( tasks != NULL ){
      delete tasks;
      tasks = NULL;
    }
    child_fertile = true;
  }
};



/**
 * The cBaseCPU class controls the running and manages all the statistics about
 * a creature.  Effectively the chemistry acting on the genome.
 **/

class cBaseCPU : public cCreature {
  friend class cTestCPU;
protected:
  cHardware hardware;

  cPhenotype * phenotype;
  cEnvironment * environment;

  sReproData repro_data;

  // Flags... @CAO slowly get rid of flag structure!
  cFlags flags;
  bool mal_active;         // Has an allocate occured since last divide?
  bool bio_mal_active;     // ???
  bool full_trace;         // Should we track everything this organism does?
  bool injected;           // Was this organism injected into the population?
  bool parasite;           // Has this organism ever executed outside code?
  bool inst_inject;        // Has a successful 'inject' instruction been done?
  bool modified;           // Has this organism been modified by another?
  bool point_mut;          // Have there been any point mutations in organism?
  bool search_approx_size; // ???
  bool parent_true;        // Is this genome an exact copy of its parents?
  bool advance_ip;         // Should the IP advance after this instruction?
  bool fertile;            // Do we allow this organisms to produce offspring?

public:
  // Info for stats
  sCPUStats  cpu_stats;
private:


  // Arrays for assesing costs
#ifdef INSTRUCTION_COSTS
  int num_inst_cost;
  int * inst_cost;
  int * inst_ft_cost;
#endif

  // ----= A few useful functions =----

  void UseResources();
  void ResetVariables();
  inline void PrintStatus(ostream & fp);

  // Allocate function

  int Allocate_Main(const int allocated_size);

  // Divide functions


  /**
   * This function contains the main logic for all divide commands.
   **/
  void Divide_Main(const int divide_point, const int extra_lines=0);

  bool  Divide_CheckViable(const int child_size, const int parent_size,
			   int extra_lines);
  void Divide_RecordInfo();
  void Divide_SetupChild(const int parent_size, const int extra_lines);
  void Divide_DoMutations();
  void Divide_CheckCopyTrue();
  void Divide_TestFitnessMeasures();

  void DoCrossoverMutation(int start1, int end1,
			   cCodeArray & in, int start2, int end2);

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
  void Fault(int fault_loc, int fault_type, cString fault_desc="");
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
  void Inst_HeadPop();
  void Inst_HeadPush();

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
  void Inst_MaxAlloc();
  void Inst_Inject();
  void Inst_Repro();

  // I/O and Sensory
  void Inst_TaskGet();
  void Inst_TaskStackGet();
  void Inst_TaskStackLoad();
  void Inst_TaskPut();
  void Inst_TaskIO();
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

  // Head-based instructions...

  void Inst_SetHead();
  void Inst_AdvanceHead();
  void Inst_MoveHead();
  void Inst_JumpHead();
  void Inst_GetHead();
  void Inst_IfLabel();
  void Inst_HeadDivide();
  void Inst_HeadRead();
  void Inst_HeadWrite();
  void Inst_HeadCopy();
  void Inst_HeadSearch();
  void Inst_SetFlow();

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


  // Biologically inspired reproduction
  void Inst_BCAlloc();
  void Inst_BCopy();
  void Inst_BDivide();
private:
  void Inst_BCopy_Main(double mut_prob); // Internal called by all BCopy's
public:
  // Bio Error Correction
  void Inst_BCopyDiv2();
  void Inst_BCopyDiv3();
  void Inst_BCopyDiv4();
  void Inst_BCopyDiv5();
  void Inst_BCopyDiv6();
  void Inst_BCopyDiv7();
  void Inst_BCopyDiv8();
  void Inst_BCopyDiv9();
  void Inst_BCopyDiv10();
  void Inst_BCopyPow2();
  void Inst_BIfNotCopy();
  void Inst_BIfCopy();


  //// Placebo ////
  void Inst_Skip();


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

  static cCodeArray LoadCode( const char * filename, const cInstLib & inst_lib);
  cGenotype * LoadCode(const char * filename);
  cGenotype * LoadCode(const cCodeArray & in_code);

  void ChangeGenotype(cGenotype * in_genotype);

  inline void SingleProcess();

  const cHardware & GetHardware() { return hardware; }

  // Input & Output Testing
  void DoInput(const int value){ hardware.DoInput(value, *phenotype); }
  void DoOutput(const int value){ hardware.DoOutput(value, *phenotype); }

  inline int FindModifiedRegister(int default_register);
  inline int FindModifiedHead(int default_head);
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

  // Interaction with other CPUs

  inline void InjectCode(cCodeArray * inject_code, int line_num);

  // Access to private variables

  inline cGenotype * GetActiveGenotype() { return info.active_genotype; }
  inline cCodeArray * GetChild()   { return &(repro_data.child_memory); }
  // inline int GetID()               { return environment->GetID(); }
  inline cBaseCPU * GetNext()      { return (cBaseCPU *) info.next; }
  inline cBaseCPU * GetPrev()      { return (cBaseCPU *) info.prev; }

  inline cEnvironment * GetEnvironment() { return environment; }

  inline bool GetCopyTrue() const    { return repro_data.copy_true; }
  inline bool GetParentTrue() const  { return parent_true; }
  inline bool GetMalActive() const   { return mal_active; }
  inline bool IsParasite() const     { return parasite; }
  inline bool IsInjected() const     { return injected; }
  inline bool IsModified() const     { return modified; }
  inline bool HasPointMut() const    { return point_mut; }
  inline int GetCopiedSize() const   { return info.copied_size; }
  inline int GetExecutedSize() const { return info.executed_size; }
  inline const cInstLib & GetInstLib()     { return hardware.GetInstLib(); }
  inline const cCPUHead & GetInstPointer() { return hardware.GetInstPointer();}
  inline const cCPUHead & GetInstPointer(int in_thread)
    { return hardware.GetInstPointer(in_thread);}
  inline const cCodeArray & GetMemory() { return hardware.GetMemory(); }
  inline int GetRegister(int reg) const  { return hardware.GetRegister(reg); }
  inline int GetStack(int depth=0, int stack_id=-1) const
    { return hardware.GetStack(depth,stack_id); }
  inline int GetActiveStack() const      { return hardware.GetActiveStack(); }
  inline int GetInput(int depth)         { return hardware.GetInput(depth); }
  inline int GetMemorySize() const   { return hardware.GetMemory().GetSize(); }
  inline int GetTotalTimeUsed() const    { return hardware.GetTimeUsed(); }
  inline int GetNumThreads() const       { return hardware.GetNumThreads(); }
  inline int GetActiveThreadID() const   { return hardware.GetCurThread(); }
  inline int GetNumDivides() const       { return info.num_divides; }
  inline int GetNumErrors() const        { return info.num_errors; }
  inline cString & GetFault()            { return info.fault_desc; }
  inline int GetAge() const              { return info.age; }
  inline int GetGestationTime() const    { return info.gestation_time; }
    // { return info.active_genotype->GetGestationTime(); }
  inline int GetCurGestation() const
    { return hardware.GetTimeUsed() - info.gestation_start; }
  inline int GetGestationStart() const   { return info.gestation_start; }
  inline cMerit GetMerit() const         { return phenotype->GetMerit(); }
  inline cMerit GetCurMerit() const      { return phenotype->GetCurMerit(); }
  inline cPhenotype * GetPhenotype() { return phenotype; }

#ifdef TEST_SEARCH_SIZE
  inline int GetSearchApproxSize() { return search_approx_size;}
#endif

  inline void SetParentTrue(bool _pt) { parent_true = _pt; }
  inline void SetParasite(bool _parasite=true) { parasite = _parasite; }
  inline void SetModified() { inst_inject = true; } // @CAO modified???
  inline void SetInjected() { injected = true; }
  inline void SetFertile(bool _fert = true) { fertile = _fert; }
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
  inline void SetMemory(const cCodeArray & in_code)
    { hardware.SetMemory(in_code);}
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
  inline char GetLineageSymbol();

  void TestSearchSize(const int search_size);

  void SaveState(ofstream & fp);
  void LoadState(ifstream & fp);

  cCodeArray LoadCodeFromStream(ifstream & fp);
};

#include "../main/inst_lib.hh"

#endif
