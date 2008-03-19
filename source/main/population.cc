//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <fstream.h>

#include "population.hh"
#include "inst_lib.hh"
#include "genebank.hh"
#include "geneology.hh"
#include "slice.hh"
#include "../cpu/cpu.hh"
#include "species.hh"
#include "../cpu/environment.hh"

#include "../cpu/head.ii"
#include "../cpu/cpu.ii"

cPopulation::cPopulation()
{
  // Initialize the instruction set and event list.
  event_list = new cEventList(cConfig::GetEventFilename(), this);
  inst_lib = cInstLib::InitInstructions(cConfig::GetInstFilename());
  cConfig::SetNumInstructions(inst_lib->GetSize());

  // Default the viewer and schedule.
  viewer = NULL;
  schedule = NULL;

  genebank = new cGenebank(this);
#ifdef GENEOLOGY
  geneology = new cGeneology(genebank,this);  // TCC - Geneology
#endif
  num_creatures = 0;

  // Setup the default environment for the creatures.
  default_environment = new cTestEnvironment(NULL);
  default_environment->SetCopyMutProb(  cConfig::GetCopyMutProb());
  default_environment->SetInsMutProb(   cConfig::GetInsMutProb());
  default_environment->SetDelMutProb(   cConfig::GetDelMutProb());
  default_environment->SetPointMutProb( cConfig::GetPointMutProb());
  default_environment->SetDivideMutProb(cConfig::GetDivideMutProb());
  default_environment->SetDivideInsProb(cConfig::GetDivideInsProb());
  default_environment->SetDivideDelProb(cConfig::GetDivideDelProb());


  // Avida specific information.
  world_x = cConfig::GetWorldX();
  world_y = cConfig::GetWorldY();
  num_cells = world_x * world_y;

  printf("...Creating CPU's...\n");

  cpu_array = new cBaseCPU[num_cells];
  reaper_queue = new cList;
  for (int cpu_id = 0; cpu_id < num_cells; cpu_id++) {
    cpu_array[cpu_id].GetEnvironment()->SetID(this, cpu_id);

    if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_ELDEST) {
      reaper_queue->InsertRear(&(cpu_array[cpu_id]));
    }
    cList * conn_list = new cList;

    // Set connections clockwise from (-1, -1)

    conn_list->InsertFront(&(cpu_array[Mod(cpu_id -world_x -1,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id -world_x   ,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id -world_x +1,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id          +1,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id +world_x +1,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id +world_x   ,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id +world_x -1,num_cells)]));
    conn_list->InsertFront(&(cpu_array[Mod(cpu_id          -1,num_cells)]));

    // Make sure creatures start out facing a proper CPU.
    conn_list->CircNext();

    cpu_array[cpu_id].GetEnvironment()->SetConnections(conn_list);
  }

  BuildTimeSlicer();
}


cPopulation::~cPopulation()
{
  for (int i = 0; i < num_cells; i++) {
    cpu_array[i].ChangeGenotype(NULL);
  }
  delete [] cpu_array;
}


void cPopulation::InitSoup()
{
  if (cConfig::GetCloneFilename() == "") {
    Inject(cConfig::GetStartCreature()(), 0);
  } else {
    ifstream fp(cConfig::GetCloneFilename()());
    LoadClone(fp);
  }
}


void cPopulation::DoUpdate()
{
  // Execute the CPUs

  int UD_size = cConfig::GetAveTimeslice() * GetNumCreatures();
  schedule->Process(UD_size);
  if (GetNumCreatures() == 0) return;

  // Do Point Mutations

  int num_mutations = (int)
    (num_cells * MAX_CREATURE_SIZE * cConfig::GetPointMutProb());

  int i;
  for (i = 0; i < num_mutations; i++) {
    int cpu_mut = g_random.GetUInt(num_cells);
    int line_mut = g_random.GetUInt(MAX_CREATURE_SIZE);
    cpu_array[cpu_mut].Mutate(line_mut);
  }

  // Reset the Genebank now that this update has been processed.

  genebank->UpdateReset();

  // end of update stats...

  CalcUpdateStats();

  // Increment Creature Ages...
  for (i = 0; i < num_cells; i++) cpu_array[i].IncAge();

#ifdef DEBUG_CHECK_OK
  // Diagnostics...
  if (!OK()) {
    g_debug.Warning("Population::OK() is failing.");
  }
#endif
}


void cPopulation::ActivateChild(sReproData & child_info, cBaseCPU * in_cpu)
{
  // If the in_cpu is NULL, get find a new position for it.
  if (in_cpu == NULL) {
    in_cpu = PositionChild((cBaseCPU *) child_info.parent_cpu);
  }

  // If it is still NULL, fail!
  if (in_cpu == NULL) return;

  cGenotype * child_genotype = child_info.parent_genotype;

  // Prep the selected cpu to be changed.

  if(in_cpu->GetActiveGenotype() != NULL) in_cpu->Kill();

  // If the creature is not an exact copy of the parent, find & initialize
  // its genotype.
  if (!child_info.copy_true) {
    child_genotype = genebank->AddGenotype(child_info.child_memory,
			     child_info.parent_genotype->GetSpecies());
    if (!child_genotype->GetNumCPUs())
      child_genotype->SetParent(*(child_info.parent_genotype));
  }

  // Report the new birth to the stats object
  cStats::NewCreature(child_genotype->GetID(),
      child_info.parent_genotype->GetID(),
      child_genotype->FindGeneticDistance(child_info.parent_genotype));

#ifdef GENEOLOGY
  // Report to the geneology object
  geneology->AddBirth(child_info.parent_genotype, child_genotype);
#endif

  // Record the breed information in the parent genotype
  child_info.parent_genotype->SetBreedStats(*child_genotype);
  cStats::AddBirth();
  if( child_info.copy_true )
    cStats::AddBreedTrue();

  // add in the gestation/repro_rate to the genotype and stats sums
  //child_info.parent_genotype->SumReproRate().
    //Add( 1 / (double)(child_info.gestation_time) );
  cStats::SumReproRate().
    Add( 1 / (double)(child_info.gestation_time) );

  // Setup this CPU with its new genotype!
  in_cpu->ChangeGenotype(child_genotype);

  // Setup the initial stats for this new creature.
  in_cpu->SetParentTrue(child_info.copy_true);
  in_cpu->SetCopiedSize(child_info.copied_size);
  in_cpu->SetExecutedSize(child_info.executed_size);
  in_cpu->SetGestationTime(child_info.gestation_time);
  in_cpu->SetFitness(child_info.fitness);
  in_cpu->InitPhenotype(child_info);
  in_cpu->GetEnvironment()->SetGeneration(child_info.parent_generation+1);

  in_cpu->SetNeutralMetric(child_info.neutral_metric);
  in_cpu->SetLineageLabel(child_info.lineage_label);

  // Initialize the time-slice for this new creature.
  AdjustTimeSlice(in_cpu->GetEnvironment()->GetID());

  // In a local run, face the child towards the parent (if it didn't kill it)
  if (in_cpu != child_info.parent_cpu &&
      cConfig::GetBirthMethod() < NUM_LOCAL_POSITION_CHILD) {
    in_cpu->GetEnvironment()->Rotate(child_info.parent_cpu);
  }

  // Special handling for certain birth methods.
  if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_ELDEST) {
    reaper_queue->InsertFront(in_cpu);
  }
}

cBaseCPU * cPopulation::PositionChild(cBaseCPU * parent_cpu)
{
  if (cConfig::GetBirthMethod() >= NUM_LOCAL_POSITION_CHILD) {
    switch (cConfig::GetBirthMethod()) {
    case POSITION_CHILD_FULL_SOUP_RANDOM:
      return & GetCPU(g_random.GetUInt(num_cells));
    case POSITION_CHILD_FULL_SOUP_ELDEST:
      return (cBaseCPU *) reaper_queue->RemoveRear();
    }
  }

  // Save initial facing.
  cBaseCPU * facing = parent_cpu->GetEnvironment()->GetFacing();
  cList found_list;

  FindEmptyCell(parent_cpu->GetEnvironment()->GetConnections(), found_list);

  // If we have not found an empty CPU, we must use the specified function
  // to determine how to choose among the filled CPU's.
  if (!found_list.GetSize()) {
    switch(cConfig::GetBirthMethod()) {
    case POSITION_CHILD_AGE:
      PositionAge(parent_cpu, found_list);
      break;
    case POSITION_CHILD_MERIT:
      PositionMerit(parent_cpu, found_list);
      break;
    case POSITION_CHILD_RANDOM:
      found_list = *(parent_cpu->GetEnvironment()->GetConnections());
      found_list.InsertFront(parent_cpu);
      break;
    case POSITION_CHILD_EMPTY:
      // Nothing is in list if no empty cells are found...
      break;
    }
  }

  // Restore the old facing
  parent_cpu->GetEnvironment()->Rotate(facing);

  // If there are no possibilities, return NULL.
  if (found_list.GetSize() == 0) return NULL;

  // Choose the CPU randomly from those in the list, and return it.
  int choice = g_random.GetUInt(found_list.GetSize());

  return (cBaseCPU *) found_list[choice];
}


void cPopulation::AdjustTimeSlice(int in_cpu)
{
  schedule->Adjust(&cpu_array[in_cpu]);
}




void cPopulation::ProcessEvents()
{
  event_list->Process();
}


// This function will scale the best merit to be between 128 and 256...
// the rest will follow...
#define RANDOM_MERIT_FACTOR 8
int cPopulation::ScaleMerit(const cMerit & in_merit)
{
  int out_merit = (int) ((in_merit.GetDouble() / max_merit.GetDouble()) * 256);

  // Add or subtract up to RANDOM_MERIT_FACTOR.
  out_merit += g_random.GetUInt(2*RANDOM_MERIT_FACTOR);

  return out_merit;
}

void cPopulation::CalcUpdateStats()
{
  int i, j;

  // Declare all variables to be calculated.
  //   SS => Sum of Squares for calculating Variance

  int num_threshold_CPUs = 0;
  double entropy = 0.0;
  double species_entropy = 0.0;
  // double energy = 0.0;

  double max_fitness = 0;
  max_merit = 0;

  int total_cells = 0;

  int num_breed_true = 0;
  int num_parasites = 0;

  cPhenotype * cur_phenotype = NULL;
  cBaseCPU *   cur_cpu       = NULL;
  cGenotype *  cur_genotype  = NULL;
  cSpecies *   cur_species   = NULL;

  // Loop through all cells, genotypes, and species which need to be
  // reset before calculations can be done.

  cur_species = genebank->GetFirstSpecies();
  for (i = 0; i < genebank->GetNumSpecies(); i++) {
    cur_species->ResetStats();
    cur_species = cur_species->GetNext();
  }

  // Clear out sums taken.
  // cStats::SumReproRate().Clear();

  cStats::SumMerit().Clear();
  cStats::SumCreatureAge().Clear();
  cStats::SumGeneration().Clear();
  cStats::SumNeutralMetric().Clear();
  cStats::SumLineageLabel().Clear();
  cStats::SumNumThreads().Clear();
  cStats::SumThreadDist().Clear();
  cStats::SumMemSize().Clear();

  cStats::SumGestation().Clear();
  cStats::SumFitness().Clear();

  cStats::SumGenotypeAge().Clear();

  cStats::SumSize().Clear();
  cStats::SumCopySize().Clear();
  cStats::SumExeSize().Clear();

  cStats::SumAbundance().Clear();
  cStats::SumGenotypeDepth().Clear();

  cStats::SumThresholdAge().Clear();
  cStats::SumSpeciesAge().Clear();


  // Loop through all the *cells* getting stats and doing calculations
  // which must be done on a creature by creature basis.

  for (i = 0; i < num_cells; i++) {
    // Only look at cells with creatures in them.
    cur_cpu = &(cpu_array[i]);

    if (cur_cpu->GetActiveGenotype()) {
      total_cells++;  // Keep a count of the current number of living cells.

      // Collect all general stats.
      cStats::SumCreatureAge().Add(cur_cpu->GetAge());
      cStats::SumGeneration().Add(cur_cpu->GetEnvironment()->GetGeneration());
      cStats::SumNeutralMetric().Add(cur_cpu->GetNeutralMetric());
      cStats::SumLineageLabel().Add(cur_cpu->GetLineageLabel());

      int num_threads = cur_cpu->GetNumThreads();
      cStats::SumNumThreads().Add(num_threads);
      if (num_threads > 1) {
	int thread_dist = cur_cpu->GetInstPointer(0).GetPosition() -
	  cur_cpu->GetInstPointer(1).GetPosition();
	if (thread_dist < 0) thread_dist *= -1;
	cStats::SumThreadDist().Add(thread_dist);
      }

      if (cur_cpu->GetMerit() > max_merit)
	max_merit = cur_cpu->GetMerit();

      cStats::SumMerit().Add(cur_cpu->GetMerit().GetDouble());

      cStats::SumMemSize().Add(cur_cpu->GetMemorySize());

      cStats::SumCopySize().Add(cur_cpu->GetCopiedSize());
      cStats::SumExeSize().Add(cur_cpu->GetExecutedSize());

      // Test what tasks this creatures has completed.
      cur_phenotype = cur_cpu->GetPhenotype();
      for( j=0; j < cur_phenotype->GetNumTasks(); j++ ){
	if( cur_phenotype->GetTaskCount(j) > 0 )  cStats::AddTasks(j,1);
      }

      // Increment the counts for all qualities the CPU has...
      if (cur_cpu->GetParentTrue()) num_breed_true++;
      if (cur_cpu->GetFlag(CPU_FLAG_PARASITE)) num_parasites++;

      // Finally, do anything special you have to for creatures of
      // [non-]threshold genotypes

      cur_genotype = cur_cpu->GetActiveGenotype();
      if (cur_genotype->GetThreshold()) {
	// Nothing yet...
      } else {
	// Nothing yet...
      }
    }
  }


  // Next, loop through all genotypes, once again finding stats and
  // doing calcuations.

  cur_genotype = genebank->GetBestGenotype();
  for (i = 0; i < genebank->GetSize(); i++) {
    const int num_CPUs = cur_genotype->GetNumCPUs();
    // const double gest_time = cur_genotype->GetGestationTime();

    // Update max_fitness if needed
    if ( cur_genotype->GetFitness() > max_fitness)
      max_fitness = cur_genotype->GetFitness();

    // Gestation times and Fitness
    //if( cur_genotype->SumReproRate().Count() > 0 )
    //cStats::SumGestation()
    //          .Add(1/(cur_genotype->SumReproRate().Average()),num_CPUs);
    cStats::SumGestation().Add(1/(cur_genotype->GetReproRate()),num_CPUs);
    cStats::SumFitness().Add(cur_genotype->GetFitness(),num_CPUs);

    cStats::SumGenotypeAge().Add(cur_genotype->GetAge(),num_CPUs);

    // Size info
    cStats::SumSize().Add(cur_genotype->GetLength(),num_CPUs);

    // Breed information for each genotype
    cStats::SumAbundance().Add(num_CPUs);

    // Depth Info
    cStats::SumGenotypeDepth().Add(cur_genotype->GetDepth(),num_CPUs);

    // Calculate this genotype's contribution to entropy
    if (num_CPUs > 0 && num_cells > 0) {
      double partial_ent = Log((double) num_cells / (double) num_CPUs) *
	((double) num_CPUs / (double) num_cells);
      entropy += partial_ent;
    }

    // Do any special calculations for threshold genotypes.
    if (cur_genotype->GetThreshold()) {
      cStats::SumThresholdAge().Add(cur_genotype->GetAge(),num_CPUs);
      num_threshold_CPUs += num_CPUs;
    }

    // Send info from this genotype to its species...
    if (cur_genotype->GetSpecies())
      cur_genotype->GetSpecies()->AddCreatures(num_CPUs);

    // ...and advance to the next genotype...
    cur_genotype = cur_genotype->GetNext();
  }

  // Loop through all of the species in the soup, taking info on them.

  cur_species = genebank->GetFirstSpecies();
  for (i = 0; i < genebank->GetNumSpecies(); i++) {
    const int num_CPUs = cur_species->GetNumCreatures();
    // const int num_genotypes = cur_species->GetNumGenotypes();

    // Basic statistical collection...
    cStats::SumSpeciesAge().Add(cur_species->GetAge(),num_CPUs);

    // Caculate entropy on the species level...
    double partial_ent = 0.0;
    if (num_CPUs > 0 && num_cells > 0) {
      partial_ent = Log((double) num_cells / (double) num_CPUs) *
	((double) num_CPUs / (double) num_cells);
    }
    species_entropy += partial_ent;

    // ...and advance to the next species...
    cur_species = cur_species->GetNext();
  }

  // Finally, do any remaining calculations, and give the stats over to
  // the stats object.

  cStats::SetEntropy(entropy);
  cStats::SetSpeciesEntropy(species_entropy);
  cStats::SetMaxFitness(max_fitness);
  cStats::SetMaxMerit(max_merit.GetDouble());
  cStats::SetBreedTrueCreatures(num_breed_true);
  cStats::SetNumParasites(num_parasites);

  cStats::SetBestGenotype(genebank->GetBestGenotype());
  cStats::SetNumCreatures(GetNumCreatures());
  cStats::SetNumGenotypes(GetNumGenotypes());
  cStats::SetNumThreshSpecies(genebank->GetNumSpecies());

  // Have stats calculate anything it now can...
  cStats::CalcEnergy();
  cStats::CalcFidelity();
}


int cPopulation::SaveClone(ofstream & fp)
{
  int ret_val = fp.good();
  int i;

  // Save the current update
  fp << cStats::GetUpdate() << " ";

  // Save the genebank info.
  genebank->SaveClone(fp);

  // Save the genotypes manually.
  fp << genebank->GetSize() << " ";

  cGenotype * cur_genotype = genebank->GetBestGenotype();
  for (i = 0; i < genebank->GetSize(); i++) {
    cur_genotype->SaveClone(fp);

    // Advance...
    cur_genotype = cur_genotype->GetNext();
  }

  // Save the CPU layout...
  fp << num_cells << " ";
  cGenotype * genotype;
  for (i = 0; i < num_cells; i++) {
    genotype = cpu_array[i].GetActiveGenotype();
    if (genotype != NULL) fp << genotype->GetID() << " ";
    else fp << "-1 ";
  }

  return ret_val;
}

int cPopulation::LoadClone(ifstream & fp)
{
  int ret_val = fp.good();
  int i;
  int cur_update;
  fp >> cur_update;
  cStats::SetCurrentUpdate(cur_update);

  // Save the genebank info.
  genebank->LoadClone(fp);

  // Load up the genotypes.
  int num_genotypes = 0;
  fp >> num_genotypes;

  cGenotype * genotype_array = new cGenotype[num_genotypes];
  for (i = 0; i < num_genotypes; i++) {
    genotype_array[i].LoadClone(fp);
  }

  // Now load them into the CPUs.  @CAO make sure num_cells is right!

  int in_num_cells;
  int genotype_id;
  fp >> in_num_cells;

  for (i = 0; i < num_cells; i++) {
    fp >> genotype_id;

    if (genotype_id == -1) continue;
    int genotype_index = -1;
    for (int j = 0; j < num_genotypes; j++) {
      if (genotype_array[j].GetID() == genotype_id) {
	genotype_index = j;
	break;
      }
    }

    if (genotype_index == -1) {
      g_debug.Error("Genotype %d not found!", genotype_id);
      return FALSE;
    }
    cpu_array[i].LoadCode(genotype_array[genotype_index].GetCode());
    AdjustTimeSlice(i);
  }


  return ret_val;
}


int cPopulation::OK()
{
  int ret_value = TRUE;

  // First check all sub-objects...

  if (!genebank->OK() || !inst_lib->OK() || !schedule->OK() ||
      !reaper_queue->OK()) {
    ret_value = FALSE;
  }

  // Next check CPUs...

  for (int i = 0; i < num_cells; i++) {
    if (!cpu_array[i].OK()) {
      g_debug.Error("AAPop OK() failed in CPU #%d", i);
      ret_value = FALSE;
    }
  }

  // And stats...

  if (world_x * world_y != num_cells) {
    g_debug.Error("world_x * wolrd_y != num_cells");
    ret_value = FALSE;
  }

  return ret_value;

}

void cPopulation::Clear()
{
  for (int i = 0; i < num_cells; i++) {
    cpu_array[i].ChangeGenotype(NULL);
  }
}

void cPopulation::Inject(const char * filename, int in_cpu, int merit, int lineage_label)
{
  cpu_array[in_cpu].LoadCode(filename);
  if ( merit > 0 )
    cpu_array[in_cpu].GetPhenotype()->Clear( merit );
  cpu_array[in_cpu].SetLineageLabel( lineage_label );
  AdjustTimeSlice(in_cpu);
}


void cPopulation::InjectRandom(int mem_size)
{
  UINT cpu_used = g_random.GetUInt(num_cells);
  cCodeArray code_used(mem_size);
  code_used.Randomize();
  cpu_array[cpu_used].LoadCode(code_used);
  AdjustTimeSlice(cpu_used);
}


cGenotype * cPopulation::GetGenotype(int thread)
{
  return genebank->GetGenotype(thread);
}

cBaseCPU * cPopulation::GetRandomCPU()
{
  const UINT rand_cpu = g_random.GetUInt(num_cells);
  return &(cpu_array[rand_cpu]);
}

// This method assumes that there is no structure to the population by
// default.  Loop within this CPU.
cCPUHead cPopulation::GetHeadPosition(cBaseCPU * in_cpu, int offset)
{
  if (offset >= HEAD_RANGE || offset <= -HEAD_RANGE) offset %= HEAD_RANGE;

  int cur_id = in_cpu->GetEnvironment()->GetID();
  cBaseCPU * cur_cpu = in_cpu;

  // Handle positive offsets...
  while (offset >= cur_cpu->GetMemorySize()) {
    offset -= cur_cpu->GetMemorySize();
    cur_id++;
    if (cur_id == num_cells) cur_id = 0;
    cur_cpu = &(cpu_array[cur_id]);
  }

  // Handle negative offsets...
  while (offset < 0) {
    cur_id--;
    if (cur_id < 0) cur_id = num_cells - 1;
    cur_cpu = &(cpu_array[cur_id]);
    offset += cur_cpu->GetMemorySize();
  }

  return cCPUHead(cur_cpu, offset);
}

unsigned int cPopulation::GetTotalMemory()
{
  return genebank->GetTotalMemory();
}

int cPopulation::GetNumGenotypes()
{
  return genebank->GetSize();
}

char * cPopulation::GetBasicGrid()
{
  char * basic_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    basic_grid[i] = cpu_array[i].GetBasicSymbol();
  }

  return basic_grid;
}

char * cPopulation::GetSpeciesGrid()
{
  char * species_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    species_grid[i] = cpu_array[i].GetSpeciesSymbol();
  }

  return species_grid;
}

char * cPopulation::GetModifiedGrid()
{
  char * modified_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    modified_grid[i] = cpu_array[i].GetModifiedSymbol();
  }

  return modified_grid;
}

char * cPopulation::GetResourceGrid()
{
  char * resource_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    resource_grid[i] = cpu_array[i].GetResourceSymbol();
  }

  return resource_grid;
}

char * cPopulation::GetAgeGrid()
{
  char * age_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    age_grid[i] = cpu_array[i].GetAgeSymbol();
  }

  return age_grid;
}

char * cPopulation::GetBreedGrid()
{
  char * breed_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    breed_grid[i] = cpu_array[i].GetBreedSymbol();
  }

  return breed_grid;
}

char * cPopulation::GetParasiteGrid()
{
  char * parasite_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    parasite_grid[i] = cpu_array[i].GetParasiteSymbol();
  }

  return parasite_grid;
}

char * cPopulation::GetPointMutGrid()
{
  char * point_mut_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    point_mut_grid[i] = cpu_array[i].GetPointMutSymbol();
  }

  return point_mut_grid;
}

char * cPopulation::GetThreadGrid()
{
  char * thread_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    thread_grid[i] = cpu_array[i].GetThreadSymbol();
  }

  return thread_grid;
}

int cPopulation::GetNumSpecies()
{
  return genebank->GetNumSpecies();
}

void cPopulation::BuildTimeSlicer()
{
  switch (cConfig::GetSlicingMethod()) {
  case SLICE_CONSTANT:
    printf ("...Building Constant Time Slicer...\n");
    schedule = new cConstSchedule();
    break;
  case SLICE_BLOCK_MERIT:
    printf ("...Building Block Time Slicer...\n");
    schedule = new cBlockSchedule();
    break;
  case SLICE_PROB_MERIT:
    printf ("...Building Probablistic Time Slicer...\n");
    schedule = new cProbSchedule(num_cells);
    break;
  case SLICE_LOGRITHMIC_MERIT:
    printf ("...Building Logrithmic Time Slicer...\n");
    schedule = new cLogSchedule(num_cells);
    break;
  case SLICE_INTEGRATED_MERIT:
    printf ("...Building Integrated Time Slicer...\n");
    schedule = new cIntegratedSchedule(this);
    break;
  default:
    schedule = new cSchedule();
    break;
  }

  if (schedule) {
    schedule->SetPopulation(this);
  }
}

void cPopulation::PositionAge(cBaseCPU * parent_cpu, cList & found_list)
{
  int max_age = parent_cpu->GetAge();
  cList * connection_list = parent_cpu->GetEnvironment()->GetConnections();
  cBaseCPU * test_cpu;

  found_list.InsertFront(parent_cpu);
  for (connection_list->Reset();
       connection_list->GetCurrent();
       connection_list->Next()) {
    test_cpu = (cBaseCPU *) connection_list->GetCurrent();

    if (test_cpu->GetAge() > max_age &&
	test_cpu->GetEnvironment()->TestResources()) {
      max_age = test_cpu->GetAge();
      found_list.Clear();
      found_list.InsertFront(test_cpu);
    }
    else if (test_cpu->GetAge() == max_age &&
	     test_cpu->GetEnvironment()->TestResources()) {
      found_list.InsertRear(test_cpu);
    }
  }
}

void cPopulation::PositionMerit(cBaseCPU * parent_cpu, cList & found_list)
{
  double min_ratio = parent_cpu->CalcMeritRatio();
  cList * connection_list = parent_cpu->GetEnvironment()->GetConnections();
  cBaseCPU * test_cpu;

  found_list.InsertFront(parent_cpu);
  for (connection_list->Reset();
       connection_list->GetCurrent();
       connection_list->Next()) {
    test_cpu = (cBaseCPU *) connection_list->GetCurrent();

    if (test_cpu->CalcMeritRatio() < min_ratio &&
	test_cpu->GetEnvironment()->TestResources()) {
      found_list.Clear();
      found_list.InsertFront(test_cpu);
    }
    else if (test_cpu->CalcMeritRatio() == min_ratio &&
	     test_cpu->GetEnvironment()->TestResources()) {
      found_list.InsertRear(test_cpu);
    }
  }
}

void cPopulation::FindEmptyCell(cList * cpu_list, cList & found_list)
{
  cBaseCPU * test_cpu;

  // See which cells are empty, and add them to the list.
  for (cpu_list->Reset();
       cpu_list->GetCurrent();
       cpu_list->Next()) {
    test_cpu = (cBaseCPU *) cpu_list->GetCurrent();

    // If this cell is empty, add it to the list...
    if (!test_cpu->GetActiveGenotype()) {
      found_list.InsertFront(test_cpu);
    }
  }
}



void cPopulation::CalcConsensus(int lines_saved)
{
  int i, j;

  int inst_size = inst_lib->GetSize();

  static cHistogram * inst_hist = NULL;

  // Setup the histogtams...

  if (inst_hist == NULL) {
    inst_hist = new cHistogram[MAX_CREATURE_SIZE];
    for (i = 0; i < MAX_CREATURE_SIZE; i++) {
      inst_hist[i].Resize(inst_size, -1);
    }
  } else {
    for (i = 0; i < MAX_CREATURE_SIZE; i++) {
      inst_hist[i].Clear();
    }
  }

  // Loop through all of the genotypes adding them to the histograms.

  cGenotype * cur_genotype = genebank->GetBestGenotype();
  for (i = 0; i < genebank->GetSize(); i++) {
    const int num_CPUs = cur_genotype->GetNumCPUs();
    const int length = cur_genotype->GetLength();
    cCodeArray * code = &(cur_genotype->GetCode());

    // Place this genotype into the histograms.
    for (j = 0; j < length; j++) {
      if (code->Get(j).GetOp() >= inst_size)
	fprintf(stderr,"Error, instruction %d in set of size %d\n",
		code->Get(j).GetOp(), inst_size);
      inst_hist[j].Insert(code->Get(j).GetOp(), num_CPUs);
    }

    // Mark all instructions beyond the length as -1 in histogram...
    for (j = length; j < MAX_CREATURE_SIZE; j++) {
      inst_hist[j].Insert(-1, num_CPUs);
    }

    // ...and advance to the next genotype...
    cur_genotype = cur_genotype->GetNext();
  }

  // Now, lets print something!
  static FILE * fp = fopen("consensus.dat", "w");
  static FILE * fp_abundance = NULL;
  static FILE * fp_var = NULL;
  static FILE * fp_entropy = NULL;

  if (lines_saved > 0 && fp_abundance == NULL) {
    fp_abundance = fopen("con-abundance.dat", "w");
    fp_var       = fopen("con-var.dat",       "w");
    fp_entropy   = fopen("con-entropy.dat",   "w");
  }

  int con_length = MAX_CREATURE_SIZE;
  cCodeArray con_code(MAX_CREATURE_SIZE);
  double total_entropy = 0.0;

  // Fill in the consensus creature from the chart...
  int size_found = FALSE;
  for (i = 0; i < MAX_CREATURE_SIZE; i++) {
    int mode = inst_hist[i].GetMode();
    int count = inst_hist[i].GetCount(mode);
    int total = inst_hist[i].GetCount();
    double entropy = inst_hist[i].GetNormEntropy();
    if (!size_found) total_entropy += entropy;

    // If the consensus at this line is -1; we've found the size of the
    // consensus creature.
    if (mode == -1) {
      if (!size_found) {
	con_length = i;
	size_found = TRUE;
      }

      // Break out if ALL creatures have a -1 in this area, and we've
      // finished printing all of the files.
      if (count == total && i >= lines_saved) break;
    } else {
      con_code[i].SetOp(mode);
    }

    // Print all needed files.
    if (i < lines_saved) {
      fprintf(fp_abundance, "%d ", count);
      fprintf(fp_var, "%.0f ", inst_hist[i].GetCountVariance());
      fprintf(fp_entropy, "%f ", entropy);
    }
  }

  // Put end-of-lines on the files.
  fprintf(fp_abundance, "\n");
  fprintf(fp_var,       "\n");
  fprintf(fp_entropy,   "\n");

  // Study the consensus creature.

  con_code.Resize(con_length);

  // Loop through genotypes again, and determine the average genetic
  // distance.

  cur_genotype = genebank->GetBestGenotype();
  cRunningSum distance_sum;
  for (i = 0; i < genebank->GetSize(); i++) {
    const int num_CPUs = cur_genotype->GetNumCPUs();
    distance_sum.Add(con_code.FindGeneticDistance(cur_genotype->GetCode()),
		     num_CPUs);

    // ...and advance to the next genotype...
    cur_genotype = cur_genotype->GetNext();
  }

  // Finally, gather last bits of data and print the results.

  cGenotype * con_genotype = genebank->FindGenotype(con_code);
  int best_dist =
    con_code.FindGeneticDistance(genebank->GetBestGenotype()->GetCode());
  double ave_dist = distance_sum.Average();
  double var_dist = distance_sum.Variance();
  double complexity_base = (double) con_code.GetSize() - total_entropy;

  if (con_genotype) {
    fprintf(fp,
	    "%d %5.0f %f %5.3f %f %d %f %f %d %d %d %d %d %d %d %d %f %f %f %f\n",
	    cStats::GetUpdate(),                         //  1
	    //con_genotype->GetMerit().GetDouble(),      //  2
	    con_genotype->GetMerit(),                  //  2
	    con_genotype->GetGestationTime(),          //  3
	    con_genotype->GetFitness(),                //  4
	    con_genotype->GetReproRate(),              //  5
	    con_genotype->GetLength(),                 //  6
	    con_genotype->GetCopiedSize(),             //  7
	    con_genotype->GetExecutedSize(),           //  8
	    con_genotype->GetBirths(),                 //  9
	    con_genotype->GetBreedTrue(),              // 10
	    con_genotype->GetBreedIn(),                // 11
	    con_genotype->GetNumCPUs(),                // 12
	    con_genotype->GetDepth(),                  // 13
	    con_genotype->GetID(),                     // 14
            con_genotype->GetAge(),                    // 15
	    best_dist,                                 // 16
	    ave_dist,                                  // 17
	    var_dist,                                  // 18
	    total_entropy,                             // 19
	    complexity_base                            // 20
	    );
  } else {

    cCPUTestInfo test_info;
    cTestCPU::TestCode(test_info, con_code);

    fprintf(fp,
	    "%d %5.0f %5.1f %5.3f %f %d %f %f %d %d %d %d %d %d %d %d %f %f %f %f\n",
	    cStats::GetUpdate(),                  //  1
	    test_info.CalcMerit(),                //  2
	    test_info.CalcGestation(),            //  3
	    test_info.CalcFitness(),                //  4
	    1.0 / (0.1  + test_info.CalcGestation()),     //  5
	    con_code.GetSize(),                   //  6
	    test_info.CalcCopiedSize(),             //  7
	    test_info.CalcExeSize(),           //  8
	    0,  // Births                         //  9
	    0,  // Breed True                     // 10
	    0,  // Breed In                       // 11
	    0,  // Num CPUs                       // 12
	    -1, // Depth                          // 13
	    -1, // ID                             // 14
            0,  // Age                            // 15
	    best_dist,                            // 16
	    ave_dist,                             // 17
	    var_dist,                             // 18
	    total_entropy,                        // 19
	    complexity_base                       // 20
	    );
  }

  // Flush the file...
  fflush(fp);
}

// Returns the code-array of maximal fitness.
cCodeArray cPopulation::CalcLandscape(int dist,
            const cGenotype & genotype, cString * fitnessChartFileName)
{
  // Create the filename for the mutant file:
  // landscape-<genotype name>-<dist>
  //const cString & name = genotype.GetName();
  //cString fileName = cString( "landscape-" );
  //((fileName += name) += "-") += cString::Stringf( "%d", dist );
  const cCodeArray & code = genotype.GetCode();
  cLandscape landscape(code, inst_lib);

  if ( *fitnessChartFileName != cString("") ) {
    static ofstream fp_mutants( *fitnessChartFileName );

    // Write the header on the mutant file:
    fp_mutants << "# Max_Landscape_Dist Base_Fitness\n" << dist ;

    // Let the landscape write out the lines of the mutant file
    landscape.Process(dist, &fp_mutants);
  } else {
    // Don't pass in a file handle
    landscape.Process( dist );
  }
  // Print the results.

  static ofstream fp("landscape.dat");
  static ofstream fp_entropy("land-entropy.dat");
  static ofstream fp_count("land-sitecount.dat");

  landscape.PrintStats(fp);
  landscape.PrintEntropy(fp_entropy);
  landscape.PrintSiteCount(fp_count);

  // Temporary debug stuff...
  //  cString base_file;
  //  cString peak_file;
  //  base_file.Set("creature.base.%d", cStats::GetUpdate());
  //  peak_file.Set("creature.peak.%d", cStats::GetUpdate());
  //  landscape.PrintBase(base_file());
  //  landscape.PrintPeakBT(peak_file());

  // return cCodeArray(const_cast(const cCodeArray, landscape.GetPeakCode()));
  return cCodeArray(const_cast(const cCodeArray &, landscape.GetPeakCode()), 0);
  }

// Returns the code-array of maximal fitness found.
cCodeArray cPopulation::SampleLandscape(int sample_size,
					const cCodeArray & code)
{
  cLandscape landscape(code, inst_lib);

  // A 0 sample size indicates that we should use the inst_lib size - 1.
  if (sample_size == 0) sample_size = inst_lib->GetSize() - 1;
  landscape.SampleProcess(sample_size);

  // Print the results.

  static ofstream fp("land-sample.dat");
  landscape.PrintStats(fp);

  return cCodeArray(landscape.GetPeakCode());
}

// Returns the code-array of maximal fitness found.
cCodeArray cPopulation::RandomLandscape(int dist, int sample_size,
        int min_found, int max_sample_size, const cCodeArray & code,
        int print_if_found)
{
  cLandscape landscape(code, inst_lib);

  landscape.RandomProcess(sample_size, dist, min_found, max_sample_size,
			  print_if_found);

  // Print the results.

  static ofstream fp("land-random.dat");
  landscape.PrintStats(fp);

  return cCodeArray(landscape.GetPeakCode());
}

void cPopulation::AnalyzeLandscape(const cCodeArray & code,
					       int sample_size)
{
  cLandscape landscape(code, inst_lib);

  static ofstream fp("land_analyze.dat");

  for (int dist = 1; dist <= 10; dist++) {
    landscape.Reset(code);
    landscape.RandomProcess(sample_size, dist);
    fp << cStats::GetUpdate()          << " "  // 1
       << dist                         << " "  // 2
       << landscape.GetProbDead()      << " "  // 3
       << landscape.GetProbNeg()       << " "  // 4
       << landscape.GetProbNeut()      << " "  // 5
       << landscape.GetProbPos()       << " "  // 6
       << landscape.GetAveFitness()    << " "  // 7
       << landscape.GetAveSqrFitness() << " "  // 8
       << endl;
  }
}

void cPopulation::PairTestLandscape(const cCodeArray & code,
						int sample_size)
{
  cLandscape landscape(code, inst_lib);

  cString filename;
  filename.Set("pairtest.%d.dat", cStats::GetUpdate());
  ofstream fp(filename());

  landscape.TestPairs(sample_size, fp);
}

void cPopulation::PairTestFullLandscape(const cCodeArray & code)
{
  cLandscape landscape(code, inst_lib);

  cString filename;
  filename.Set("pairtest.%d.dat", cStats::GetUpdate());
  ofstream fp(filename());

  landscape.TestAllPairs(fp);
}

void cPopulation::HillClimb(const cCodeArray & code)
{
  ofstream fp("hillclimb.dat");
  cLandscape landscape(code, inst_lib);
  landscape.HillClimb(fp);
}

void cPopulation::Apocalypse(double kill_prob){
  for( int i=0; i<cConfig::GetWorldX()*cConfig::GetWorldY(); ++i ){
      if(cpu_array[i].GetActiveGenotype() != NULL){
	  if( g_random.P(kill_prob) ){
	  cpu_array[i].ChangeGenotype(NULL);
	  cpu_array[i].Kill();
	  AdjustTimeSlice(i);
      }
    }
  }
}


void cPopulation::AnalyzePopulation(ofstream & fp, double sample_prob, bool landscape, bool save_genotype)
{
  cGenotype *genotype;

  cCPUTestInfo test_info;

  fp << "# (1) cell number (2) genotype name (3) length (4) fitness (5) copy true? (6) is viable? (7) -... landscape data " << endl;

  for (int i = 0; i < num_cells; i++) {
    genotype = cpu_array[i].GetActiveGenotype();
    if ( genotype != NULL && // is the creature alife?
	 g_random.P(sample_prob)  ){ // do we want to sample it?
      // run test cpu on this creature
      cCodeArray code = genotype->GetCode();
      cTestCPU::TestCode(test_info, code);


      fp << i << " "                          // 1 cell number
	 << genotype->GetName()() << " "      // 2 name
         << genotype->GetLength() << " "      // 3 length
	 << test_info.CalcFitness() << " "    // 4 fitness
         << test_info.CalcCopyTrue() << " "   // 5 copy true?
         << test_info.IsViable() << " ";      // 6 is viable?
      // create landscape object for this creature
      if ( landscape && test_info.IsViable() ){
	cLandscape landscape( code, inst_lib);
	landscape.Process(1);
	landscape.PrintStats(fp);
      }
      else fp << endl;
      if ( save_genotype && test_info.IsViable() && genotype->GetThreshold() ){
	char filename[40];
	sprintf( filename, "genebank/%s", genotype->GetName()() );
	cTestCPU::PrintCode( code, filename );
      }
    }
  }
}



