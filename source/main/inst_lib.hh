//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef INST_LIB_HH
#define INST_LIB_HH

#define INST_BLOCK_SIZE 16
#define NOP_BLOCK_SIZE 4

#define INSTLIB_TYPE_DEFAULT  0
#define INSTLIB_TYPE_ARGS     1
#define INSTLIB_TYPE_REDCODE  2
#define INSTLIB_TYPE_BINARY   3

#define INST_MODE_BASE 0
#define INST_MODE_ALT  1
#define NUM_INST_MODES 2

#include "stats.hh"
#include "../cpu/cpu.hh"

// A typdef to simplify having an instruction point to methods in the
// CPU object.
typedef void (cBaseCPU::*tCPUMethod)();


/**
 * This class gives all of the info about a single instruction
 * in the library. Not all of the info that is stored in the library
 * is necessarily used by the CPU's, scheduler, and so on.
 **/

class cInstEntry {
private:
  cString name;               // Name of this instruction.
  tCPUMethod function;        // Pointer to CPU method for this instruction.
  tCPUMethod alt_function;    // Alternate function, for notify.
  int redundancy;             // Relative weight in instruction set (not impl.)
  int cost;                   // additional CPU spent to exectute inst.
  int ft_cost;                // CPU spent first time exec (in add to cost)
  double prob_exe;            // probability of executing inst (not impl.)
  int num_args;               // Number of arguments this inst uses.
public:
  cInstEntry();
  cInstEntry(const cInstEntry & _in);
  ~cInstEntry();

  void Init(const cString & in_name, tCPUMethod in_function,
		      int in_num_args, int in_redundancy = 1, int in_cost = 0,
		      int in_ft_cost = 0, double in_prob_exe = 1);

  void operator=(const cInstEntry & _in);

  /**
   * @return The name of the represented instruction.
   **/
  const cString & GetName()          const { return name; }
  
  /**
   * @return A pointer to the CPU method for this instruction.
   **/
  inline tCPUMethod GetFunction()    const { return function; }
  
  /**
   * @return Alternate function, for notify.
   **/
  inline tCPUMethod GetAltFunction() const { return alt_function; }
  
  /**
   * @return Relative weight in instruction set (not impl.)
   **/
  inline int        GetRedundance()  const { return redundancy; }
  
  /**
   * @return additional CPU time spent to exectute inst.
   **/
  inline int        GetCost()        const { return cost; }
  
  /**
   * @return CPU spent first time exec (in add to cost)
   **/
  inline int        GetFTCost()      const { return ft_cost; }
  inline double     GetProbExe()     const { return prob_exe; }
  
  /**
   * @return The number of arguments this instruction uses.
   **/
  inline int        GetNumArgs()     const { return num_args; }
  
  void SetName(const cString & _in)   { name = _in; }
  void SetFunction(tCPUMethod _in)    { function = _in; }
  void SetAltFunction(tCPUMethod _in) { alt_function = _in; }
  void SetRedundance(int _in)         { redundancy = _in; }
  void SetCost(int _in)               { cost = _in; }
  void SetFTCost(int _in)             { ft_cost = _in; }
  void SetProbExe(double _in)         { prob_exe = _in; }
  void SetNumArgs(int _in)            { num_args = _in; }
};

/**
 * This class is used to create a mapping from the command strings in
 * a creatures' genome into real methods in one of the CPU objects.  This
 * object has been designed to allow easy manipulation of the instruction
 * sets, as well as multiple instruction sets within a single soup (just
 * attach different cInstLib objects to different CPUs.
 **/

class cInstLib {
private:
  int num_instructions;
  int max_size;
  int num_nops;
  int max_nops;
  int num_args;
  int type;
  int mode;

  cInstEntry * inst_array;

  //  'nop_mods' indicates which register (or stack? buffer?) that a nop
  //       modifies a function to use.
  int * nop_mods;

  // An few static values...
  static const cString name_unknown;

  void IncreaseMaxInst();
  void IncreaseMaxNops();
public:
  cInstLib();
  cInstLib(cInstLib * in_inst_lib);
  ~cInstLib();

  cInstLib & operator=(const cInstLib & in_inst_lib);

  int OK() { return TRUE; } /// @CAO FIX!!!!!

  void Add(const cInitFile & file, char * in_name, tCPUMethod in_function,
	   int in_num_args=0);
  void AddNop(const cInitFile & file, char * in_name, int reg); // non-Arg only
  void AddLabel(const cInitFile & file, char * in_name);        // Arg only!
  void SetAltFunctions(tCPUMethod in_alt_function);
  inline void ActivateAltFunctions();

  // Accessors

  inline const cInstEntry & GetInstEntry(int id) const {return inst_array[id];}
  inline const cString & GetName(const cInstruction & inst) const {
    UCHAR id = inst.GetOp();
    // Should be able to do this without rv, but HP-CC complains,  @TCC
    const cString & rv = (id < num_instructions)
      ? inst_array[id].GetName() : name_unknown;
    return rv;
  }

  inline int    GetCost(const int inst) const {
    assert(inst>=0 && inst<num_instructions);
    return inst_array[inst].GetCost(); }
  inline int    GetFTCost(const int inst) const {
    assert(inst>=0 && inst<num_instructions);
    return inst_array[inst].GetFTCost(); }
  inline double GetProbExe(const int inst) const {
    assert(inst>=0 && inst<num_instructions);
    return inst_array[inst].GetProbExe(); }
  inline int    GetCost(const cInstruction & inst) const {
    return inst_array[inst.GetOp()].GetCost(); }
  inline int    GetFTCost(const cInstruction & inst) const {
    return inst_array[inst.GetOp()].GetFTCost(); }
  inline double GetProbExe(const cInstruction & inst) const {
    return inst_array[inst.GetOp()].GetProbExe(); }

  inline tCPUMethod GetFunction(const cInstruction & inst) const {
    return inst_array[inst.GetOp()].GetFunction();
  }
  inline tCPUMethod GetAltFunction(const cInstruction & inst) const {
    return inst_array[inst.GetOp()].GetAltFunction();
  }
  inline tCPUMethod GetActiveFunction(const cInstruction & inst) const;
  inline tCPUMethod GetRandFunction() const;
  inline int GetNopMod(const cInstruction & inst) const
    { return nop_mods[inst.GetOp()]; }

  inline int GetSize() const { return num_instructions; }
  inline int GetNumNops() const { return num_nops; }
  inline int GetNumArgs() const { return num_args; }
  inline int GetNumArgs(const cInstruction & inst) const {
    return inst_array[inst.GetOp()].GetNumArgs();
  }
  inline int GetType() const { return type; }
  inline int GetMode() const { return mode; }

  inline cInstruction GetRandomInst() const {
    // @CAO - find a way to do this without INST_ARGS
    // @CAO - for the moment, this only handles 24 bits...
#ifdef INST_ARGS
    cInstruction out_inst((UCHAR) g_random.GetUInt(GetSize()));
    for (int i = 0; i < GetNumArgs(); i++) {
      char type = g_random.GetUInt(256);
      // @CAO currently args range only from -256 to 255
      int value = g_random.GetInt(-256, 256);
      out_inst.SetArg(i, type, value);
    }
    return out_inst;
#else
    return cInstruction((UCHAR) g_random.GetUInt(GetSize()));
#endif
  }
  UCHAR GetInstID(const cString & in_name) const;
  void SetupInstArg(const cString & name, int id, cLongInstruction & info)
    const;
  void SetupInstRed(const cString & name, int id, cLongInstruction & info)
    const;
  cInstruction GetInst(const cString & in_name) const;

  void PrintCode(const cCodeArray & code, ofstream & fp) const;
  inline int Compare(const cInstruction & inst1, const cInstruction & inst2);

  // Instruction Analysis.
  inline int IsNop(const int inst) const;
  inline int IsNop(const cInstruction & inst) const;
  inline int IsLabel(const cInstruction & inst) const;

  // Static function to initialize an instruction set.
  static cInstLib * InitInstructions(const char * inst_filename=NULL);
  static void InitBinaryInst(const cInitFile & file, cInstLib & lib);
  static void InitArgsInst(const cInitFile & file, cInstLib & lib);
  static void InitRedcodeInst(const cInitFile & file, cInstLib & lib);
  static void InitBaseInst(const cInitFile & file, cInstLib & lib);
};


// Inline functions...

inline void cInstLib::ActivateAltFunctions()
{
  if (mode == INST_MODE_BASE) mode = INST_MODE_ALT;
  else mode = INST_MODE_BASE;
}

inline tCPUMethod cInstLib::GetActiveFunction(const cInstruction & inst) const
{
  tCPUMethod rv;
  if( inst.GetOp() > LAST_POSSIBLE_INSTRUCTION ){
    rv = &cBaseCPU::Inst_Nop;
  }else{
    if (mode == INST_MODE_BASE) {
      rv = inst_array[inst.GetOp()].GetFunction();
    } else {  // if (mode == INST_MODE_ALT) {
      rv = inst_array[inst.GetOp()].GetAltFunction();
    }
  }
  return rv;
}

inline tCPUMethod cInstLib::GetRandFunction() const
{
  tCPUMethod rv;
  cInstruction inst = GetRandomInst();
  if (inst.GetOp() > LAST_POSSIBLE_INSTRUCTION){
    rv = &cBaseCPU::Inst_Nop;
  } else {
    if (mode == INST_MODE_BASE) {
      rv = inst_array[inst.GetOp()].GetFunction();
    } else {  // if (mode == INST_MODE_ALT) {
      rv = inst_array[inst.GetOp()].GetAltFunction();
    }
  }
  return rv;
}

inline int cInstLib::Compare(const cInstruction & inst1,
			     const cInstruction & inst2)
{
  if (inst1.GetOp() != inst2.GetOp()) return FALSE;
#ifdef INST_ARGS
  for (int i = 0; i < GetNumArgs(inst1); i++) {
    if (inst1.GetArg(i) != inst2.GetArg(i)) return FALSE;
  }
#endif
  return TRUE;
}

inline int cInstLib::IsNop(const int inst) const
{
#ifdef DEBUG
  if (num_args > 0) g_debug.Warning("Calling IsNop in an Arg-based inst-set");
#endif
  return (inst < num_nops);
}

inline int cInstLib::IsNop(const cInstruction & inst) const
{
  return IsNop(inst.GetOp());
}

inline int cInstLib::IsLabel(const cInstruction & inst) const
{
#ifdef DEBUG
  if (num_args == 0)
    g_debug.Warning("Calling IsLabel in a non Arg-based inst-set");
#endif
  return (inst.GetOp() < num_nops);
}


#endif
