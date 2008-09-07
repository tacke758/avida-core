//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef INST_LIB_HH
#define INST_LIB_HH

#define INST_FLAG_ALT_MODE 1

#include "stats.hh"
#include "../cpu/cpu.hh"

// A typdef to simplify having an instruction point to methods in the
// CPU object.
typedef void (cBaseCPU::*tCPUMethod)();

// The cInstLib class is used to create a mapping from the command strings in
// a creatures' genome into real methods in one of the CPU objects.  This
// object has been designed to allow easy manipulation of the instruction
// sets, as well as multiple instruction sets within a single soup (just
// attact different cInstLib objects to different CPUs.

class cInstLib {
private:
  int num_instructions;
  int max_size;
public:
  cInstLib(int in_max_size);
  cInstLib(cInstLib * in_inst_lib);
  ~cInstLib();

  // Public variables:
  //  'name[N]' is the name of instruction number N.
  //  'function' is a pointer the CPU method to be executed for insturction
  //        N.  This is a virtual function which will automatically find the
  //        proper method for the dirived CPU type executing it.
  //  'alt_function' alternative functions to be run instead of the actual
  //       function for this instruction.  ActivateAltFunctions() swaps the
  //       function[] and the alt_function[] arrays.  Currently this is used
  //       to step through a specific creature's execution; Notify() is used
  //       for the alt_function, and when it is activated, lets the viewer
  //       know that a new instruction has occured in the CPU, and then runs
  //       the proper instruction.

  cString name[MAX_INST_SET_SIZE];
  tCPUMethod function[MAX_INST_SET_SIZE];
  tCPUMethod alt_function[MAX_INST_SET_SIZE];
  cFlags flags;

  int OK() { return TRUE; } /// @CAO FIX!!!!!
  void Add(char * in_name, tCPUMethod in_function);
  void SetAltFunctions(tCPUMethod in_alt_function);
  void ActivateAltFunctions();

  inline int GetSize() { return num_instructions; }
};

cInstLib * InitInstructions(const char * inst_filename=NULL);

#endif
