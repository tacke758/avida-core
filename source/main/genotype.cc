//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "genotype.hh"
#include "species.hh"

///////////////////////////
//  cGenotype
///////////////////////////

cGenotype::cGenotype(int in_update_born)
{
  g_memory.Add(C_GENOTYPE);

  update_born = in_update_born;

  static int next_id = 1;

  id_num = next_id++;
  symbol = '.';
  num_CPUs = 0;
  total_CPUs = 0;
  total_parasites = 0;
  creatures_born = 0;
  
  tot_copied_size = 0;
  num_copied_size = 0;
  tot_exe_size = 0;
  num_exe_size = 0;
  gestation_time = 0;
  num_gestations = 0;
  merit = 0;
  num_merits = 0;
  total_fitness = 0.0;
  num_fitness = 0;
  parent_id = -1;
  parent_distance = -1;
  species = NULL;
  name = "(no name)";

  next = NULL;
  prev = NULL;
}

cGenotype::~cGenotype()
{
  g_memory.Remove(C_GENOTYPE);

  // Reset some of the variables to make sure program will crash if a deleted
  // cell is read!

  symbol = '!';

  num_CPUs = -1;
  total_CPUs = -1;
  
  next = NULL;
  prev = NULL;
}

int cGenotype::OK()
{
  int ret_value = TRUE;

  // Check the components...

  if (!code.OK()) ret_value = FALSE;
  
  // And the statistics

  if (id_num < 0 || num_CPUs < 0 || total_CPUs < 0 || tot_copied_size < 0 ||
      tot_exe_size < 0 || gestation_time < 0 || num_gestations < 0 ||
      num_merits < 0 || num_fitness < 0 || update_born < 0 ||
      parent_distance < -1) {
    g_debug.Error("Genotype with neg variable (ID, size, etc..)!");
    ret_value = FALSE;
  }
  if (GetCopiedSize() > code.GetSize()) {
    g_debug.Error("UD %d: Copied size [%d] is larger than genotype size[%d]!",
		  stats.GetUpdate(), GetCopiedSize(), code.GetSize());
    ret_value = FALSE;
  }

  return ret_value;
};

void cGenotype::SetParent(cGenotype * parent)
{
  parent_id = parent->GetID();
  parent_distance = code.FindSlidingDistance(&(parent->code));

  // Initialize all stats to those of the parent genotype....
  merit.Clear();
  merit += parent->GetApproxMerit();
  gestation_time  = parent->GetApproxGestationTime();
  total_fitness   = parent->GetApproxFitness();
  tot_exe_size    = parent->GetApproxExecutedSize();
  tot_copied_size = parent->GetApproxCopiedSize();
  
  num_merits      = 0;
  num_gestations  = 0;
  num_fitness     = 0;
  num_exe_size    = 0;
  num_copied_size = 0;
}

void cGenotype::Mutate()  // Check each location to be mutated.
{
  int i;

  for (i = 0; i < code.GetSize(); i++) {
    if (TRUE) { // g_random.GetUInt()) {     //@CAO always true!
      code[i] = g_random.GetUInt(stats.GetNumInstructions());
      // Flag command as having been mutated? @CAO
    }
  }
}

void cGenotype::UpdateReset()
{
  creatures_born = 0;
}

void cGenotype::SetCode(cCodeArray * in_code)
{
  code.CopyData(*in_code);

  gestation_time = 0;
  num_gestations = 0;
  merit = 0;
  num_merits = 0;
  total_fitness = 0.0;
  num_fitness = 0;
  parent_distance = -1;
}

char cGenotype::GetSpeciesSymbol()
{
  if (species) return species->GetSymbol();
  return '.';
}

void cGenotype::SetSpecies(cSpecies * in_species)
{
  species = in_species;
}
