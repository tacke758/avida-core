//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef INST_LIB_HH
#define INST_LIB_HH

#include "../tools/string.hh"
#include "../tools/tArray.hh"
#include "../tools/tools.hh"

#include "inst.hh"
#include "inst_lib.hh"

#include "../cpu/hardware_method.hh"

// A typdef to simplify having an instruction point to methods in the
// cHardwareBase object and its dirivitives...
class cHardwareBase;

// moved to cpu/hardware_method.hh for porting to gcc 3.1 -- k
//typedef bool (cHardwareBase::*tHardwareMethod)();

class cInstLibBase;

/**
 * This class is used to create a mapping from the command strings in
 * an organism's genome into real methods in one of the hardware objects.  This
 * object has been designed to allow easy manipulation of the instruction
 * sets, as well as multiple instruction sets within a single soup (just
 * attach different cInstSet objects to different hardware.
 **/

class cInstSet {
#ifdef USE_INST_SET_CODE
public:
  cInstLibBase *m_inst_lib;
  class cInstEntry2 {
  public:
    int lib_fun_id;
    int redundancy;           // Weight in instruction set (not impl.)
    int cost;                 // additional time spent to exectute inst.
    int ft_cost;              // time spent first time exec (in add to cost)
    double prob_fail;         // probability of failing to execute inst
  };
  tArray<cInstEntry2> m_lib_name_map;
  tArray<int> m_lib_nopmod_map;
  tArray<int> mutation_chart2;     // ID's represented by redundancy values.
  // Static components...
  static cInstruction inst_error2;
  // static const cInstruction inst_none;
  static cInstruction inst_default2;
#else /* USE_INST_SET_CODE */
private:
  // This class gives full info about a single instruction in the library.
  class cInstEntry {
  public:
    cString name;             // Name of this instruction.
    tHardwareMethod function; // Pointer to hardware method.
    int redundancy;           // Weight in instruction set (not impl.)
    int cost;                 // additional time spent to exectute inst.
    int ft_cost;              // time spent first time exec (in add to cost)
    double prob_fail;         // probability of failing to execute inst
  };

  tArray<cInstEntry> inst_array;  // The instructions indexed by ID
  tArray<int> nop_mods;           // Modification table for nops
  tArray<int> mutation_chart;     // ID's represented by redundancy values.
#endif /* USE_INST_SET_CODE */

  // Static components...
  static const cInstruction inst_error;
  // static const cInstruction inst_none;
  static const cInstruction inst_default;

public:
  cInstSet();
  cInstSet(const cInstSet & in_inst_set);
  ~cInstSet();

  cInstSet & operator=(const cInstSet & _in);

  bool OK() const;

  // Accessors
  const cString & GetName(int id) const
  { 
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_inst_lib->GetName(m_lib_name_map[id].lib_fun_id);
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return inst_array[id].name;
#endif /* USE_INST_SET_CODE */
  }
  const cString & GetName(const cInstruction & inst) const
  {
    return GetName(inst.GetOp());
  }
  int GetCost(const cInstruction & inst) const
  {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_lib_name_map[inst.GetOp()].cost;
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return inst_array[inst.GetOp()].cost;
#endif /* USE_INST_SET_CODE */
  }
  int GetFTCost(const cInstruction & inst) const
  {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_lib_name_map[inst.GetOp()].ft_cost;
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return inst_array[inst.GetOp()].ft_cost;
#endif /* USE_INST_SET_CODE */
  }
  double GetProbFail(const cInstruction & inst) const
  {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_lib_name_map[inst.GetOp()].prob_fail;
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return inst_array[inst.GetOp()].prob_fail;
#endif /* USE_INST_SET_CODE */
  }
  int GetRedundancy(const cInstruction & inst) const
  {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_lib_name_map[inst.GetOp()].redundancy;
#else /* USE_INST_SET_CODE */
    ///* XXX end */
    return inst_array[inst.GetOp()].redundancy;
#endif /* USE_INST_SET_CODE */
  }

#ifdef USE_INST_SET_CODE
  /* XXX start -- kgn */
  int GetLibFunctionIndex(const cInstruction & inst) const
  {
    return m_lib_name_map[inst.GetOp()].lib_fun_id;
  }
  /* XXX end */
#else /* USE_INST_SET_CODE */
  tHardwareMethod GetFunction(const cInstruction & inst) const
  {
    return inst_array[inst.GetOp()].function;
  }
#endif /* USE_INST_SET_CODE */

  int GetNopMod(const cInstruction & inst) const
  {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_inst_lib->GetNopMod(m_lib_nopmod_map[inst.GetOp()]);
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return nop_mods[inst.GetOp()];
#endif /* USE_INST_SET_CODE */
  }

  cInstruction GetRandomInst() const;
#ifdef USE_INST_SET_CODE
  int GetRandFunctionIndex() const
  {
    return m_lib_name_map[ GetRandomInst().GetOp() ].lib_fun_id;
  }
#else /* USE_INST_SET_CODE */
  tHardwareMethod GetRandFunction() const
  {
    return inst_array[ GetRandomInst().GetOp() ].function;
  }
#endif /* USE_INST_SET_CODE */

  int GetSize() const {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_lib_name_map.GetSize();
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return inst_array.GetSize();
#endif /* USE_INST_SET_CODE */
  }
  int GetNumNops() const {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return m_lib_nopmod_map.GetSize();
#else /* USE_INST_SET_CODE */
    ///* XXX end */
    return nop_mods.GetSize();
#endif /* USE_INST_SET_CODE */
  }

  // Instruction Analysis.
  int IsNop(const cInstruction & inst) const
  {
#ifdef USE_INST_SET_CODE
    ///* XXX start -- kgn */
    return (inst.GetOp() < m_lib_nopmod_map.GetSize());
    ///* XXX end */
#else /* USE_INST_SET_CODE */
    return (inst.GetOp() < nop_mods.GetSize());
#endif /* USE_INST_SET_CODE */
  }

  // Insertion of new instructions...
#ifdef USE_INST_SET_CODE
  int Add2(
    const int lib_fun_id,
    const int redundancy=1,
    const int ft_cost=0,
    const int cost=0,
    const double prob_fail=0.0
  );
  int AddNop2(
    const int lib_nopmod_id,
    const int redundancy=1,
    const int ft_cost=0,
    const int cost=0,
    const double prob_fail=0.0
  );
#else /* USE_INST_SET_CODE */
  int Add(const cString & _name, tHardwareMethod _fun, const int redundancy=1,
	  const int ft_cost=0, const int cost=0, const double prob_fail=0.0);
  int AddNop(const cString & _name, tHardwareMethod _fun, const int reg,
	     const int redundancy=1, const int ft_cost=0, const int cost=0,
	     const double prob_fail=0.0);
#endif /* USE_INST_SET_CODE */

#ifdef USE_INST_SET_CODE
  /* XXX start -- kgn */
  // accessors for instruction library
  cInstLibBase *GetInstLib(){ return m_inst_lib; }
  void SetInstLib(cInstLibBase *inst_lib){
    m_inst_lib = inst_lib;
    inst_error2 = inst_lib->GetInstError();
    inst_default2 = inst_lib->GetInstDefault();
  }
  /* XXX end */
#endif /* USE_INST_SET_CODE */

  inline cInstruction GetInst(const cString & in_name) const;
  cString FindBestMatch(const cString & in_name) const;

  // Static methods..
  static const cInstruction & GetInstDefault() {
#ifdef USE_INST_SET_CODE
    /* XXX start --- kgn */
    //if(inst_default != inst_default2)
    //{ std::cout << "<cInstSet::GetInstDefault> mismatch!" << std::endl; }
    return inst_default2;
    /* XXX end */
#else /* USE_INST_SET_CODE */
    return inst_default;
#endif /* USE_INST_SET_CODE */
  }
  static const cInstruction & GetInstError()   {
    /* XXX start --- kgn */
    //if(inst_error != inst_error2)
    //{ std::cout << "<cInstSet::GetInstError> mismatch!" << std::endl; }
#ifdef USE_INST_SET_CODE
    return inst_error2;
#else /* USE_INST_SET_CODE */
    /* XXX end */
    return inst_error;
#endif /* USE_INST_SET_CODE */
  }
  // static const cInstruction & GetInstNone()    { return inst_none; }
};


inline cInstruction cInstSet::GetInst(const cString & in_name) const
{
#ifdef USE_INST_SET_CODE
  for (int i = 0; i < m_lib_name_map.GetSize(); i++) {
    if (m_inst_lib->GetName(m_lib_name_map[i].lib_fun_id) == in_name) {
      return cInstruction(i);
    }
  }
  // Adding default answer if nothing is found...
  /*
  FIXME:  this return value is supposed to be cInstSet::GetInstError
  which should be the same as m_inst_lib->GetInstError().
  -- kgn
  */
  return cInstruction(0);
#else /* USE_INST_SET_CODE */
  /* XXX start --- kgn */
  for (int i = 0; i < inst_array.GetSize(); i++) {
    if (inst_array[i].name == in_name) return cInstruction(i);
  }
  // assert(false); // Requesting ID for unknown inst.
  return cInstSet::GetInstError();
  /* XXX end */
#endif /* USE_INST_SET_CODE */
}


#endif

