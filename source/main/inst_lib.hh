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
#include "inst_superlib.hh"

#include "../cpu/hardware_method.hh"

// A typdef to simplify having an instruction point to methods in the
// cHardwareBase object and its dirivitives...
class cHardwareBase;

// moved to cpu/hardware_method.hh for porting to gcc 3.1 -- k
//typedef bool (cHardwareBase::*tHardwareMethod)();

class cInstSuperlibBase;

/**
 * This class is used to create a mapping from the command strings in
 * an organism's genome into real methods in one of the hardware objects.  This
 * object has been designed to allow easy manipulation of the instruction
 * sets, as well as multiple instruction sets within a single soup (just
 * attach different cInstLib objects to different hardware.
 **/

class cInstLib {
public:
  cInstSuperlibBase *m_inst_superlib;
  class cInstEntry2 {
  public:
    int superlib_fun_id;
    int redundancy;           // Weight in instruction set (not impl.)
    int cost;                 // additional time spent to exectute inst.
    int ft_cost;              // time spent first time exec (in add to cost)
    double prob_fail;         // probability of failing to execute inst
  };
  tArray<cInstEntry2> m_superlib_name_map;
  tArray<int> m_superlib_nopmod_map;
  tArray<int> mutation_chart2;     // ID's represented by redundancy values.
  // Static components...
  static cInstruction inst_error2;
  // static const cInstruction inst_none;
  static cInstruction inst_default2;
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

  // Static components...
  static const cInstruction inst_error;
  // static const cInstruction inst_none;
  static const cInstruction inst_default;


public:
  cInstLib();
  cInstLib(const cInstLib & in_inst_lib);
  ~cInstLib();

  cInstLib & operator=(const cInstLib & _in);

  bool OK() const;

  // Accessors
  const cString & GetName(int id) const
  { 
    ///* XXX start -- kgn */
    //if( inst_array[id].name
    //  !=m_inst_superlib->GetName(m_superlib_name_map[id].superlib_fun_id)
    //){ std::cout << "<cInstLib::GetName> mismatch!" << std::endl; }
    ///* XXX end */
    //return inst_array[id].name;

    return m_inst_superlib->GetName(m_superlib_name_map[id].superlib_fun_id);
  }
  const cString & GetName(const cInstruction & inst) const
  {
    return GetName(inst.GetOp());
  }
  int GetCost(const cInstruction & inst) const
  {
    ///* XXX start -- kgn */
    //if( inst_array[inst.GetOp()].cost
    //  !=m_superlib_name_map[inst.GetOp()].cost
    //){ std::cout << "<cInstLib::GetCost> mismatch!" << std::endl; }
    ///* XXX end */
    //return inst_array[inst.GetOp()].cost;

    return m_superlib_name_map[inst.GetOp()].cost;
  }
  int GetFTCost(const cInstruction & inst) const
  {
    ///* XXX start -- kgn */
    //if( inst_array[inst.GetOp()].ft_cost
    //  !=m_superlib_name_map[inst.GetOp()].ft_cost
    //){ std::cout << "<cInstLib::GetFTCost> mismatch!" << std::endl; }
    ///* XXX end */
    //return inst_array[inst.GetOp()].ft_cost;

    return m_superlib_name_map[inst.GetOp()].ft_cost;
  }
  double GetProbFail(const cInstruction & inst) const
  {
    ///* XXX start -- kgn */
    //if( inst_array[inst.GetOp()].prob_fail
    //  !=m_superlib_name_map[inst.GetOp()].prob_fail
    //){ std::cout << "<cInstLib::GetProbFail> mismatch!" << std::endl; }
    ///* XXX end */
    //return inst_array[inst.GetOp()].prob_fail;

    return m_superlib_name_map[inst.GetOp()].prob_fail;
  }
  int GetRedundancy(const cInstruction & inst) const
  {
    ///* XXX start -- kgn */
    //if( inst_array[inst.GetOp()].redundancy
    //  !=m_superlib_name_map[inst.GetOp()].redundancy
    //){ std::cout << "<cInstLib::GetRedundancy> mismatch!" << std::endl; }
    ///* XXX end */
    //return inst_array[inst.GetOp()].redundancy;

    return m_superlib_name_map[inst.GetOp()].redundancy;
  }

  tHardwareMethod GetFunction(const cInstruction & inst) const
  {
    return inst_array[inst.GetOp()].function;
  }

  /* XXX start -- kgn */
  int GetSuperlibFunctionIndex(const cInstruction & inst) const
  {
    return m_superlib_name_map[inst.GetOp()].superlib_fun_id;
  }
  /* XXX end */

  int GetNopMod(const cInstruction & inst) const
  {
    ///* XXX start -- kgn */
    //if( nop_mods[inst.GetOp()]
    //  !=m_inst_superlib->GetNopMod(m_superlib_nopmod_map[inst.GetOp()])
    //){ std::cout << "<cInstLib::GetNopMod> mismatch!" << std::endl; }
    ///* XXX end */
    //return nop_mods[inst.GetOp()];

    return m_inst_superlib->GetNopMod(m_superlib_nopmod_map[inst.GetOp()]);
  }

  cInstruction GetRandomInst() const;
  tHardwareMethod GetRandFunction() const
  {
    return inst_array[ GetRandomInst().GetOp() ].function;
  }

  int GetSize() const {
    ///* XXX start -- kgn */
    //if( inst_array.GetSize() != m_superlib_name_map.GetSize() )
    //{ std::cout << "<cInstLib::GetSize> mismatch!" << std::endl; }
    ///* XXX end */
    //return inst_array.GetSize();

    return m_superlib_name_map.GetSize();
  }
  int GetNumNops() const {
    ///* XXX start -- kgn */
    //if( nop_mods.GetSize() != m_superlib_nopmod_map.GetSize() )
    //{ std::cout << "<cInstLib::GetNumNops> mismatch!" << std::endl; }
    ///* XXX end */
    //return nop_mods.GetSize();

    return m_superlib_nopmod_map.GetSize();
  }

  // Instruction Analysis.
  int IsNop(const cInstruction & inst) const
  {
    ///* XXX start -- kgn */
    //if( nop_mods.GetSize() != m_superlib_nopmod_map.GetSize() )
    //{ std::cout << "<cInstLib::IsNop> mismatch!" << std::endl; }
    ///* XXX end */
    //return (inst.GetOp() < nop_mods.GetSize());

    return (inst.GetOp() < m_superlib_nopmod_map.GetSize());
  }

  // Insertion of new instructions...
  int Add(const cString & _name, tHardwareMethod _fun, const int redundancy=1,
	  const int ft_cost=0, const int cost=0, const double prob_fail=0.0);
  int AddNop(const cString & _name, tHardwareMethod _fun, const int reg,
	     const int redundancy=1, const int ft_cost=0, const int cost=0,
	     const double prob_fail=0.0);

  int Add2(
    const int superlib_fun_id,
    const int redundancy=1,
    const int ft_cost=0,
    const int cost=0,
    const double prob_fail=0.0
  );
  int AddNop2(
    const int superlib_nopmod_id,
    const int redundancy=1,
    const int ft_cost=0,
    const int cost=0,
    const double prob_fail=0.0
  );

  /* XXX start -- kgn */
  // accessors for instruction superlibrary
  cInstSuperlibBase *GetInstSuperlib(){ return m_inst_superlib; }
  void SetInstSuperlib(cInstSuperlibBase *inst_superlib){
    m_inst_superlib = inst_superlib;
    inst_error2 = inst_superlib->GetInstError();
    inst_default2 = inst_superlib->GetInstDefault();
  }
  /* XXX end */

  inline cInstruction GetInst(const cString & in_name) const;
  cString FindBestMatch(const cString & in_name) const;

  // Static methods..
  static const cInstruction & GetInstDefault() {
    /* XXX start --- kgn */
    if(inst_default != inst_default2)
    { std::cout << "<cInstLib::GetInstDefault> mismatch!" << std::endl; }
    /* XXX end */
    return inst_default;
  }
  static const cInstruction & GetInstError()   {
    /* XXX start --- kgn */
    if(inst_error != inst_error2)
    { std::cout << "<cInstLib::GetInstError> mismatch!" << std::endl; }
    /* XXX end */
    return inst_error;
  }
  // static const cInstruction & GetInstNone()    { return inst_none; }
};


inline cInstruction cInstLib::GetInst(const cString & in_name) const
{

  ///* XXX start --- kgn */
  //int i, i1=-1, i2=-1;
  //std::cout <<
  //  "<cInstLib::GetInst> inst_array.GetSize():" <<
  //  inst_array.GetSize() <<
  //  std::endl;
  //for (i = 0; i < inst_array.GetSize(); i++) {
  //  if (inst_array[i].name == in_name){
  //    i1 = i;
  //  }
  ////if (inst_array[i].name == in_name) return cInstruction(i);
  //}
  //std::cout <<
  //  "<cInstLib::GetInst> i:" <<
  //  i <<
  //  std::endl;
  //std::cout <<
  //  "<cInstLib::GetInst> inst_array.GetSize():" <<
  //  inst_array.GetSize() <<
  //  std::endl;
  //for (i = 0; i < m_superlib_name_map.GetSize(); i++) {
  //  if (m_inst_superlib->GetName(m_superlib_name_map[i].superlib_fun_id) == in_name){
  //    i2 = i;
  //  }
  //}
  //std::cout <<
  //  "<cInstLib::GetInst> i:" <<
  //  i <<
  //  std::endl;
  //if(i1 != i2){
  //  std::cout << "<cInstLib::GetInst> mismatch!" << std::endl;
  //}
  //if(i1>-1){ return cInstruction(i1); }
  ///* XXX end */

  //// assert(false); // Requesting ID for unknown inst.
  //return cInstLib::GetInstError();

  for (int i = 0; i < m_superlib_name_map.GetSize(); i++)
    if (m_inst_superlib->GetName(m_superlib_name_map[i].superlib_fun_id) == in_name)
      return cInstruction(i);
}


#endif

