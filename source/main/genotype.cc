//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "genotype.hh"
#include "species.hh"
#include "merit.hh"
#include "../cpu/test_cpu.hh"

///////////////////////////
//  cGenotype
///////////////////////////

cGenotype::cGenotype(int in_update_born)
{
  g_memory.Add(C_GENOTYPE);

  update_born = in_update_born;

  static int next_id = 1;

  id_num = next_id++;
  symbol = 0;
  test_fitness = -1;
  num_CPUs = 0;
  last_num_CPUs = 0;
  total_CPUs = 0;
  total_parasites = 0;

  births = 0;
  breed_true = 0;
  breed_in = 0;
  last_births = 0;
  last_breed_true = 0;
  last_breed_in = 0;

  parent_id = -1;
  parent_distance = -1;
  gene_depth = 0;
  species = NULL;
  parent_species = NULL;
  name = "000-no_name";

  parent_genotype = NULL;
  num_offspring_genotypes = 0;
  active = true;
  update_deactivated = -1;

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

int cGenotype::SaveClone(ofstream & fp)
{
  fp << id_num         << " ";
  fp << code.GetSize() << " ";

  for (int i = 0; i < code.GetSize(); i++) {
    fp << ((int) code[i].GetOp()) << " ";
    // @CAO add something here to save arguments for instructions.
  }

  return TRUE;
}

int cGenotype::LoadClone(ifstream & fp)
{
  int code_size = 0;

  fp >> id_num;
  fp >> code_size;

  code.Reset(code_size);
  for (int i = 0; i < code_size; i++) {
    cInstruction temp_inst;
    int inst_op;
    fp >> inst_op;
    temp_inst.SetOp((UCHAR) inst_op);
    code.Set(i, temp_inst);
    // @CAO add something here to load arguments for instructions.
  }

  return TRUE;
}

int cGenotype::OK()
{
  int ret_value = TRUE;

  // Check the components...

  if (!code.OK()) ret_value = FALSE;

  // And the statistics
  if (id_num < 0 || num_CPUs < 0 || total_CPUs < 0
      || update_born < -1 || parent_distance < -1
      || sum_copied_size.Sum() < 0 || sum_exe_size.Sum() < 0
      || sum_gestation_time.Sum() < 0 || sum_repro_rate.Sum() < 0
      || sum_merit.Sum() < 0 || sum_fitness.Sum() < 0
      || tmp_sum_copied_size.Sum() < 0 || tmp_sum_exe_size.Sum() < 0
      || tmp_sum_gestation_time.Sum() < 0 || tmp_sum_repro_rate.Sum() < 0
      || tmp_sum_merit.Sum() < 0 || tmp_sum_fitness.Sum() < 0  ){
    g_debug.Error("Genotype with neg variable (ID, size, etc..)!");
    ret_value = FALSE;
  }

  return ret_value;
};

void cGenotype::SetParent(cGenotype & parent)
{
  parent_id = parent.GetID();
  parent_distance = code.FindSlidingDistance(parent.code);
  parent_species = parent.GetSpecies();
  gene_depth = parent.GetDepth() + 1;
  parent.AddOffspringGenotype();
  parent_genotype = &parent;

  // Initialize all stats to those of the parent genotype....
  tmp_sum_copied_size.Add(    parent.GetApproxCopiedSize());
  tmp_sum_exe_size.Add(       parent.GetApproxExecutedSize());
  tmp_sum_gestation_time.Add( parent.GetApproxGestationTime());
  tmp_sum_repro_rate.Add(   1/parent.GetApproxGestationTime());
  tmp_sum_merit.Add(          parent.GetApproxMerit());
  tmp_sum_fitness.Add(        parent.GetApproxFitness());
}

void cGenotype::Mutate()  // Check each location to be mutated.
{
  int i;

  for (i = 0; i < code.GetSize(); i++) {
    if (TRUE) { // g_random.GetUInt()) {     //@CAO always true!
      code[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
      // Flag command as having been mutated? @CAO
    }
  }
}

void cGenotype::UpdateReset()
{
  // Update the "Last" for the per/update statistics
  SetLastNumCPUs();
  SetLastBirths();
  SetLastBreedTrue();
  SetLastBreedIn();
}

void cGenotype::SetCode(const cCodeArray & in_code)
{
  code.CopyData(in_code);
  // Zero Stats
  sum_copied_size.Clear();
  sum_exe_size.Clear();
  sum_gestation_time.Clear();
  sum_repro_rate.Clear();
  sum_merit.Clear();
  sum_fitness.Clear();
  tmp_sum_copied_size.Clear();
  tmp_sum_exe_size.Clear();
  tmp_sum_gestation_time.Clear();
  tmp_sum_repro_rate.Clear();
  tmp_sum_merit.Clear();
  tmp_sum_fitness.Clear();

  name.Set("%03d-no_name", in_code.GetSize());
}

char cGenotype::GetSpeciesSymbol() const
{
  if (species) return species->GetSymbol();
  return '.';
}

void cGenotype::CalcTestStats() const
{
  cCPUTestInfo test_info;
  cTestCPU::TestCode(test_info, GetCode());
  // we can assign to test_fitness, because it is mutable
  test_generations = test_info.GetMaxDepth();
  test_initial_fitness = test_info.GetGenotypeFitness();
  test_fitness = test_info.GetColonyFitness();
}

double cGenotype::GetTestFitness() const {
  if (test_fitness == -1) CalcTestStats();
  return test_fitness;
}

double cGenotype::GetTestInitialFitness() const {
  if (test_fitness == -1) CalcTestStats();
  return test_initial_fitness;
}

int cGenotype::GetTestGenerations() const {
  if (test_fitness == -1) CalcTestStats();
  return test_generations;
}

void cGenotype::SetSpecies(cSpecies * in_species)
{
  species = in_species;
}

void cGenotype::AddMerit(const cMerit & in)
{
  sum_merit.Add(in.GetDouble());
}

void cGenotype::RemoveMerit(const cMerit & in)
{
  sum_merit.Subtract(in.GetDouble());
}

int cGenotype::AddCPU()
{
  total_CPUs++;
  return ++num_CPUs;
}

int cGenotype::RemoveCPU()
{
  return --num_CPUs;
}

