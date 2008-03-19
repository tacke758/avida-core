//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2002 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef HARDWARE_BASE_HH
#define HARDWARE_BASE_HH

#include "../main/inst.hh"

class cOrganism;
class cCodeLabel;
class cGenome;
class cInstLib;
class cCPUMemory;

#define HARDWARE_TYPE_DEFAULT      0
#define HARDWARE_TYPE_VIRTUAL_CPU  1
#define HARDWARE_TYPE_4STACK       2
#define NUM_HARDWARE_TYPES         3

class cHardwareBase {
protected:
  cOrganism * organism;       // Organism using this hardware.
  cInstLib * inst_lib;        // Instruction library.
  int viewer_lock;            // Used if the viewer should only lock onto
                              //  one aspect of the hardware.

  static int instance_count;
public:
  cHardwareBase(cOrganism * in_organism, cInstLib * in_inst_lib);
  virtual ~cHardwareBase();
  virtual void Recycle(cOrganism * new_organism, cInstLib * in_inst_lib);

  // --------  Organism ---------
  cOrganism * GetOrganism() { return organism; }

  // --------  Instruction Library  --------
  const cInstLib & GetInstLib() { return *inst_lib; }
  int GetNumInst();
  cInstruction GetRandomInst();
  virtual void SetInstLib(cInstLib & in_inst_lib) { inst_lib = &in_inst_lib; }

  // --------  No-Operation Instruction --------
  bool Inst_Nop();  // A no-operation instruction that does nothing! 

  // --------  Interaction with Viewer --------
  int & ViewerLock() { return viewer_lock; }

  // --------  Core Functionality  --------
  virtual void Reset() = 0;
  virtual void SingleProcess(std::ostream * trace_fp=NULL) = 0;
  virtual void LoadGenome(const cGenome & new_genome) = 0;
  virtual bool OK() = 0;

  // --------  Other Virtual Tools --------
  virtual int GetType() const = 0;
  virtual int Inject(const cCodeLabel & in_label,
		      const cGenome & injection) = 0;

  // --------  Input and Output --------
  virtual void PrintStatus(std::ostream & fp) = 0;
  virtual void SaveState(std::ostream & fp) = 0;
  virtual void LoadState(std::istream & fp) = 0;


  // --------  Mutations (Must be Virtual)  --------
  virtual int PointMutate(const double mut_rate) = 0;
  virtual bool TriggerMutations(int trigger) = 0;

  // --------  @CAO Should be rethought?  --------
  virtual cCPUMemory & GetMemory() = 0;

  // --------  DEBUG ---------
  static int GetInstanceCount() { return instance_count; }
};

#endif
