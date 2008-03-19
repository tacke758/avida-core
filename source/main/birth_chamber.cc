//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "birth_chamber.hh"

#include "genome.hh"
#include "genome_util.hh"
#include "../cpu/hardware_method.hh"
#include "../tools/functions.hh"
#include "organism.hh"


using namespace std;


cBirthChamber::cBirthChamber() : genome_waiting(false)
{
}

cBirthChamber::~cBirthChamber()
{
}

bool cBirthChamber::SubmitOffspring(const cGenome & child_genome,
				    const cOrganism & parent,
				    tArray<cOrganism *> & child_array,
				    tArray<cMerit> & merit_array)
{
  // Collect some info for building the child.
  const cPopulationInterface & pop_interface = parent.PopInterface();
  const cEnvironment & environment =
    parent.GetPhenotype().GetEnvironment();
  
  if (parent.GetPhenotype().DivideSex() == false) {
    // This is asexual who doesn't need to wait in the birth chamber
    // just build the child and return.
    child_array.Resize(1);
    child_array[0] = new cOrganism(child_genome, pop_interface, environment);
    merit_array.Resize(1);
    merit_array[0] = parent.GetPhenotype().GetMerit();
    return true;
  }

  // If we make it this far, it must be a sexual or a waiting, 
  // non-recombining  asexual   
  // See if there is anything for it to mate with.
  if (genome_waiting == false) {
    // There is nothing waiting!
    wait_entry.genome = child_genome;
    wait_entry.merit = parent.GetPhenotype().GetMerit();
    genome_waiting = true;
    return false;
  }

  // There is already someone in the birth chamber
  // We must now do a crossover between the two 

  genome_waiting = false;

  // How many crossovers should be do? For now, 0 or 1
  if (parent.GetPhenotype().CrossNum() ==0) {
    child_array.Resize(2);
    child_array[0] = new cOrganism(wait_entry.genome, pop_interface,
				   environment);
    child_array[1] = new cOrganism(child_genome, pop_interface, environment);
    merit_array.Resize(2);
    merit_array[0] = wait_entry.merit;
    merit_array[1] = parent.GetPhenotype().GetMerit();
    return true;
  }
  else {
    cCPUMemory genome0 = wait_entry.genome;
    cCPUMemory genome1 = child_genome;

    // Crossover...
    // get (for now) one region to cross
    
    double start_frac = g_random.GetDouble();
    double end_frac = g_random.GetDouble();

    if (start_frac > end_frac) Swap(start_frac, end_frac);

    int start0 = (int) (start_frac * genome0.GetSize());
    int end0   = (int) (end_frac * genome0.GetSize());
    int start1 = (int) (start_frac * genome1.GetSize());
    int end1   = (int) (end_frac * genome1.GetSize());

    assert( start0 >= 0  &&  start0 < genome0.GetSize() );
    assert( end0   >= 0  &&  end0   < genome0.GetSize() ); 
    assert( start1 >= 0  &&  start1 < genome1.GetSize() );
    assert( end1   >= 0  &&  end1   < genome1.GetSize() );
  
    // Calculate size of sections crossing over...
    int size0 = end0 - start0;
    int size1 = end1 - start1;
  
    int new_size0 = genome0.GetSize() - size0 + size1;
    int new_size1 = genome1.GetSize() - size1 + size0;
  
    // Don't Crossover if offspring will be illegal!!!
    if( new_size0 < MIN_CREATURE_SIZE || new_size0 > MAX_CREATURE_SIZE ||
        new_size1 < MIN_CREATURE_SIZE || new_size1 > MAX_CREATURE_SIZE ){
      return false;
    }

    if (size0 > 0 && size1 > 0) {
      cGenome cross0 = cGenomeUtil::Crop(genome0, start0, end0);
      cGenome cross1 = cGenomeUtil::Crop(genome1, start1, end1);
      genome0.Replace(start0, size0, cross1);
      genome1.Replace(start1, size1, cross0);
    }
    else if (size0 > 0) {
      cGenome cross0 = cGenomeUtil::Crop(genome0, start0, end0);
      genome1.Replace(start1, size1, cross0);
    }
    else if (size1 > 0) {
      cGenome cross1 = cGenomeUtil::Crop(genome1, start1, end1);
      genome0.Replace(start0, size0, cross1);
    }

    // Build the two organisms.
    child_array.Resize(2);
    child_array[0] = new cOrganism(genome0, pop_interface, environment);
    child_array[1] = new cOrganism(genome1, pop_interface, environment);

    double cut_frac = end_frac - start_frac;
    double stay_frac = 1.0 - cut_frac;

    double merit0 = wait_entry.merit.GetDouble();
    double merit1 = parent.GetPhenotype().GetMerit().GetDouble();
    merit_array.Resize(2);

    merit_array[0] = stay_frac * merit0 + cut_frac * merit1;
    merit_array[1] = stay_frac * merit1 + cut_frac * merit0;

    return true;
  }
}

