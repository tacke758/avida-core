//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2002 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef BIRTH_CHAMBER_HH
#define BIRTH_CHAMBER_HH

#include "../tools/tList.hh"
#include "../tools/merit.hh"

#include "../cpu/cpu_memory.hh"
#include "../cpu/label.hh"

class cOrganism;

/**
 * All genome-based organisms must go through the birth chamber, which will
 * handle any special modifications during the birth process, such as divide
 * mutations reversions, sterilization, and crossover for sex.  If sex is
 * turned on, this class will hold organisms that are waiting to be crossed
 * over before they are born.
 **/

class cBirthChamber {
private:
  class cBirthEntry {
  public:
    cCPUMemory genome;
    cMerit merit;
  };

  cBirthEntry wait_entry;

  // For the moment, the following variable is the only one being used.
  bool genome_waiting;  // Is there are genome waiting to be born?
  
public:
  cBirthChamber();
  ~cBirthChamber();

  // Handle manipulations & tests of genome.  Return false if divide process
  // should halt.  Place offspring in child_array.
  bool SubmitOffspring(const cGenome & child_genome, const cOrganism & parent,
		       tArray<cOrganism *> & child_array,
		       tArray<cMerit> & merit_array);
};

#endif
