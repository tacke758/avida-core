//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "population.hh"
#include "inst_lib.hh"
#include "genebank.hh"
#include "slice.hh"
#include "../cpu/cpu.hh"
#include "species.hh"
#include "../cpu/environment.hh"

#include "../cpu/head.ii"

cPopulation::cPopulation(const cGenesis & in_genesis) : local_stats(in_genesis)
{
  g_memory.Add(C_POPULATION);

  // Save a pointer to the genesis object.
  genesis = (cGenesis *) &in_genesis;

  // Load in the needed stats.
  int size_merit_method = Genesis().ReadInt("SIZE_MERIT_METHOD", 0);
  int task_merit_method = Genesis().ReadInt("TASK_MERIT_METHOD",
					    TASK_MERIT_EXPONENTIAL);
  slicing_method = Genesis().ReadInt("SLICING_METHOD", SLICE_CONSTANT);
  birth_method   = Genesis().ReadInt("BIRTH_METHOD", POSITION_CHILD_AGE);
  death_method   = Genesis().ReadInt("DEATH_METHOD", DEATH_METHOD_OFF);
  age_limit      = Genesis().ReadInt("AGE_LIMIT", -1);
  ave_time_slice = Genesis().ReadInt("AVE_TIME_SLICE", 30);

  // Initialize the instruction set and event list.
  stats.SetInstFilename(Genesis().ReadString("INST_SET", "inst_set"));
  stats.SetTaskFilename(Genesis().ReadString("TASK_SET", "task_set"));
  stats.SetEventFilename(Genesis().ReadString("EVENT_FILE", "event_list"));
  inst_lib = InitInstructions(stats.GetInstFilename()());
  event_list = new cEventList(stats.GetEventFilename()());

  // Setup the stats.
  stats.SetAveTimeslice(ave_time_slice);
  stats.SetSizeMeritMethod(size_merit_method);
  stats.SetTaskMeritMethod(task_merit_method);
  stats.SetBirthMethod(birth_method);
  stats.SetDeathMethod(death_method);
  stats.SetAgeLimit(age_limit);
  stats.SetNumInstructions(inst_lib->GetSize());

  // Default the viewer and schedule.
  viewer = NULL;
  schedule = NULL;
}

cPopulation::~cPopulation()
{
  g_memory.Remove(C_POPULATION);
}

void cPopulation::InitSoup()
{
  Inject(Genesis().ReadString("START_CREATURE")(), 0);
}

void cPopulation::DoUpdate()
{
  // Trigger all needed events.

  // Scrap all events whose time has past...
  while (event_list->GetEvent() &&
	 event_list->GetEvent()->GetUpdate() < stats.GetUpdate()) {
    g_debug.Warning ("Event set for update %d skipped", 
		     event_list->GetEvent()->GetUpdate());
    event_list->Next();
  }

  // Activate all events meant to be done this update.
  while (event_list->GetEvent() &&
	 event_list->GetEvent()->GetUpdate() == stats.GetUpdate()) {
    ProcessEvent(*(event_list->GetEvent()));
    event_list->Next();
  }
}

void cPopulation::ActivateChild(sReproData * child_info, cBaseCPU * in_cpu)
{
  // VIRTUAL FUNCTION

  g_debug.Error("In virtual function cPopulation::ActivateChild()");
  g_debug.Error("child_info = [%p], in_cpu = [%p]", child_info, in_cpu);
}

void cPopulation::AdjustTimeSlice(int in_cpu)
{
  // VIRTUAL FUNCTION

  g_debug.Error("In virtual function cPopulation::AdjustTimeSlice()");
  g_debug.Error("in_cpu = [%d]", in_cpu);
}

// This function will scale the best merit to be between 128 and 256...
// the rest will follow...
#define RANDOM_MERIT_FACTOR 8
int cPopulation::ScaleMerit(const cMerit & in_merit)
{
  int out_merit = (int) in_merit.GetBase();
  out_merit >>= (int) (max_merit.GetShift() - in_merit.GetShift());

  // Add or subtract up to RANDOM_MERIT_FACTOR.
  out_merit += g_random.GetUInt(2*RANDOM_MERIT_FACTOR);

  return out_merit;
}

void cPopulation::CalcUpdateStats()
{
  // VIRTUAL FUNCTION
}

void cPopulation::ProcessEvent(const cEvent & event)
{
  g_debug.Warning("Unknown Event (in virtual function!): [%s]",
		  event.GetName()());
  // VIRTUAL FUNCTION
}


int cPopulation::OK()
{
  int ret_value = TRUE;

  // First check all sub-objects...

  if (!genebank->OK() || !inst_lib->OK() || 
      !local_stats.OK() || !schedule->OK()) {
    ret_value = FALSE;
  }

  return ret_value;
}

void cPopulation::Clear()
{
}

void cPopulation::Inject(char * /* filename */, int /* in_cpu */)
{
  // VIRTUAL FUNCTION
}


cGenotype * cPopulation::GetGenotype(int thread)
{
  return genebank->GetGenotype(thread);
}

// cBaseCPU * cPopulation::GetCPU(int in_num)
// {
//   return NULL;
// }

cBaseCPU * cPopulation::GetRandomCPU()
{
  return NULL;
}

unsigned int cPopulation::GetTotalMemory()
{
  return 0;
}

int cPopulation::GetNumGenotypes()
{
  return genebank->GetSize();
}

int cPopulation::GetNumSpecies()
{
  return genebank->GetNumSpecies();
}


///////////////////////
//  cGAPopulation
///////////////////////

cGAPopulation::cGAPopulation(const cGenesis & in_genesis)
  : cPopulation(in_genesis)
{
  cGenotype * new_genotype;
  int i;

  num_cells = Genesis().ReadInt("WORLD_SIZE");
  local_stats.SetNumCells(num_cells);

  genebank = new cGenebank(this);
  for (i = 0; i < num_cells; i++) {
    new_genotype = new cGenotype; //......
  }
}

cGAPopulation::~cGAPopulation()
{
}

void cGAPopulation::DoUpdate()
{
  // Handle anything the cPopulation needs to for this update.
  
  cPopulation::DoUpdate();

  for (int i = 0; i < num_cells; i++) {
    cpu.ChangeGenotype(genebank->GetGenotype(0));
    genebank->NextGenotype(0);
    for (int j = 0; j < ave_time_slice; j++) {
      cpu.SingleProcess();
    }
  }
}

void cGAPopulation::ActivateChild(sReproData * /* child_info */,
				  cBaseCPU *   /* in_cpu     */)
{
  // Place it on a list for the next generation.
}

int cGAPopulation::OK()
{
  int ret_value = TRUE;

  if (!cPopulation::OK()) ret_value = FALSE;
  if (!cpu.OK()) ret_value = FALSE;

  return ret_value;
}

void cGAPopulation::Inject(char * filename, int /* in_cpu */)
{
  cpu.LoadCode(filename);

  // Must now place the genotype somewhere...?
}

unsigned int cGAPopulation::GetTotalMemory()
{
  unsigned int total_memory = 0;
  int i;

  for (i = 0; i < num_cells; i++) {
    total_memory += genebank->GetGenotype(0)->GetLength();
    genebank->NextGenotype(0);
  }

  return total_memory;
}


//////////////////////////////
//  cAutoAdaptivePopulation
//////////////////////////////

cAutoAdaptivePopulation::cAutoAdaptivePopulation(const cGenesis & in_genesis)
  : cPopulation(in_genesis)
{
  genebank = new cGenebank(this);
  num_creatures = 0;

  // The cpu_array and all other details must be created in the individual
  // population type.
}

cAutoAdaptivePopulation::~cAutoAdaptivePopulation()
{
  for (int i = 0; i < num_cells; i++) {
    cpu_array[i].ChangeGenotype(NULL);
  }
  delete [] cpu_array;  
}

void cAutoAdaptivePopulation::BuildTimeSlicer()
{
  switch (slicing_method) {
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

void cAutoAdaptivePopulation::DoUpdate()
{
  // Handle anything the cPopulation needs to for this update.
  
  cPopulation::DoUpdate();

  // Execute the CPUs

  int UD_size = ave_time_slice * GetNumCreatures();
  schedule->Process(UD_size);
  if (GetNumCreatures() == 0) return;
  
  // Do Point Mutations

  static int num_mutations = num_cells * MAX_CREATURE_SIZE *
    local_stats.GetPointMutRate() / POINT_MUT_BASE;

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

void cAutoAdaptivePopulation::AdjustTimeSlice(int in_cpu)
{
  schedule->Adjust(&cpu_array[in_cpu]);
}

void cAutoAdaptivePopulation::Clear()
{
  for (int i = 0; i < num_cells; i++) {
    cpu_array[i].ChangeGenotype(NULL);
  }
}

void cAutoAdaptivePopulation::Inject(char * filename, int in_cpu)
{
  cBaseCPU * cur_cpu = &(cpu_array[in_cpu]);
  cur_cpu->LoadCode(filename);
  AdjustTimeSlice(in_cpu);
}

void cAutoAdaptivePopulation::CalcUpdateStats()
{
  int i, j;

  // Declare all variables to be calculated.

  int total_gestation = 0;
  int num_gestations = 0;
  int num_threshold_CPUs = 0;
  double total_repro_rate = 0;
  double entropy = 0.0;
  double species_entropy = 0.0;
  double energy = 0.0;

  double total_fitness = 0;
  double max_fitness = 0;
  cLongMerit total_merit;
  max_merit = 0;
  
  int total_creature_age = 0;
  int total_genotype_age = 0;
  int total_threshold_age = 0;
  int total_species_age = 0;

  int total_size = 0;
  int total_copy_size = 0;
  int total_exe_size = 0;
  int total_mem_size = 0;
  int total_cells = 0;

  int num_breed_true = 0;
  int num_parasites = 0;

  int num_gets = 0;
  int num_puts = 0;
  int num_ggps = 0;
  int num_tasks[NUM_TASKS];
  for (i = 0; i < NUM_TASKS; i++) num_tasks[i] = 0;

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

  // Loop through all the *cells* getting stats and doing calculations
  // which must be done on a creature by creature basis.

  for (i = 0; i < num_cells; i++) {
    // Only look at cells with creatures in them.
    cur_cpu = &(cpu_array[i]);

    if (cur_cpu->GetActiveGenotype()) {
      total_cells++;  // Keep a count of the current number of living cells.

      // Collect all general stats.
      total_merit += cur_cpu->GetMerit();
      total_copy_size += cur_cpu->GetCopiedSize();
      total_exe_size += cur_cpu->GetExecutedSize();
      total_mem_size += cur_cpu->GetMemorySize();
      total_creature_age += cur_cpu->GetAge();
      if (cur_cpu->GetMerit() > max_merit)
	max_merit = cur_cpu->GetMerit();

      // Test what tasks this creatures has completed.
      cur_phenotype = cur_cpu->GetPhenotype();
      if (cur_phenotype->GetGetFlag()) num_gets++;
      if (cur_phenotype->GetPutFlag()) num_puts++;
      if (cur_phenotype->GetGGPFlag()) num_ggps++;
      for (j = 0; j < NUM_TASKS; j++) {
	if (cur_phenotype->GetTaskFlag(j)) num_tasks[j]++;
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
    const int gest_time = cur_genotype->GetGestationTime();
    const double approx_fitness = cur_genotype->GetFitness();

    // Get the gestation times and fitness for each genotype.
    total_gestation += gest_time * num_CPUs;
    total_fitness += cur_genotype->GetFitness() * num_CPUs;
    if (approx_fitness > max_fitness) max_fitness = approx_fitness;
    if (gest_time) {
      total_repro_rate += (1.0 / (double) gest_time) * num_CPUs;
      num_gestations += num_CPUs;
    }
    total_genotype_age += cur_genotype->GetAge() * num_CPUs;
    total_size += cur_genotype->GetLength() * num_CPUs;

    // Calculate this genotype's contribution to entropy
    if (num_CPUs) {
      double partial_ent = log((double) num_cells / (double) num_CPUs) *
	((double) num_CPUs / (double) num_cells);
      entropy += partial_ent;
    }

    // Do any special calculations for threshold genotypes.
    if (cur_genotype->GetThreshold()) {
      total_threshold_age += cur_genotype->GetAge() * num_CPUs;
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
    total_species_age += cur_species->GetAge() * num_CPUs;

    // Caculate entropy on the species level...
    double partial_ent = 0.0;
    if (num_CPUs) partial_ent = log((double) num_cells / (double) num_CPUs) *
      ((double) num_CPUs / (double) num_cells);
    species_entropy += partial_ent;

    // ...and advance to the next species...
    cur_species = cur_species->GetNext();
  }

  // Finally, do any remaining calculations, and give the stats over to
  // the stats object.
  
  local_stats.SetAveMerit(total_merit.GetAverage(total_cells));
  if (num_gestations) {
    local_stats.SetAveGestation((double) total_gestation / (double) num_gestations);
  }
  local_stats.SetAveFitness(total_fitness / (double) total_cells);
  local_stats.SetAveReproRate(total_repro_rate / (double) total_cells);
  local_stats.SetEntropy(entropy);
  local_stats.SetSpeciesEntropy(species_entropy);
  local_stats.SetMaxFitness(max_fitness);
  local_stats.SetMaxMerit(max_merit.GetDouble());
  local_stats.SetBreedTrue(num_breed_true);
  local_stats.SetNumParasites(num_parasites);
  local_stats.SetAveSize((double) total_size / (double) total_cells);
  local_stats.SetAveCopySize((double) total_copy_size / (double) total_cells);
  local_stats.SetAveExeSize((double) total_exe_size / (double) total_cells);
  local_stats.SetAveMemSize((double) total_mem_size / (double) total_cells);

  local_stats.SetAveCreatureAge((double) total_creature_age / 
				(double) total_cells);
  local_stats.SetAveGenotypeAge((double) total_genotype_age /
				(double) total_cells);
  if (num_threshold_CPUs) 
    local_stats.SetAveThresholdAge((double) total_threshold_age /
				   (double) num_threshold_CPUs);
  else local_stats.SetAveThresholdAge(0.0);

  local_stats.SetAveSpeciesAge((double) total_species_age /
			       (double) total_cells);

  local_stats.SetGets(num_gets);
  local_stats.SetPuts(num_puts);
  local_stats.SetGGPs(num_ggps);
  for (i = 0; i < NUM_TASKS; i++) {
    local_stats.SetTasks(i, num_tasks[i]);
  }

  local_stats.SetBestGenotype(genebank->GetBestGenotype());
  local_stats.SetNumCreatures(GetNumCreatures());
  local_stats.SetNumGenotypes(GetNumGenotypes());
  local_stats.SetNumThreshSpecies(genebank->GetNumSpecies());

  // Have stats calculate anything it now can...
  local_stats.CalcEnergy();
  local_stats.CalcFidelity();
}

void cAutoAdaptivePopulation::ProcessEvent(const cEvent & event)
{
  if (event.GetName() == "inject") {
    cString filename = event.GetArgs().GetWord(0);
    int cpu_num = event.GetArgs().GetWord(1).AsInt();
    Inject(filename(), cpu_num);
  } else if (event.GetName() == "set_copy_mut") {
    int new_cmut = event.GetArgs().GetWord(0).AsInt();
    for (int i = 0; i < num_cells; i++) cpu_array[i].GetEnvironment()->SetCopyMutRate(new_cmut);
    local_stats.SetCopyMutRate(new_cmut);
  } else if (event.GetName() == "cycle") {
    // Add the cycle command again, next time the command is to be run.
    event_list->AddEvent(event.GetUpdate() + event.GetArgs().GetWord().AsInt(),
			 event.GetName(), event.GetArgs());
    // Add the command to be executed...
    cString new_args(event.GetArgs());
    new_args.PopWord();  // Get rid of the cycle length.
    event_list->AddEvent(event.GetUpdate(), new_args.PopWord(), new_args);
  } else if (event.GetName() == "pause") {
    Pause();
  } else if (event.GetName() == "print") {
    // Not implemented yet...
  } else {
    g_debug.Warning("Unknown Event: [%s]", event.GetName()());
  }
}

cBaseCPU * cAutoAdaptivePopulation::GetRandomCPU()
{
  return &(cpu_array[g_random.GetUInt(num_cells)]);
}

unsigned int cAutoAdaptivePopulation::GetTotalMemory()
{
  return genebank->GetTotalMemory();
}

void cAutoAdaptivePopulation::ActivateChild(sReproData * child_info,
					    cBaseCPU * in_cpu)
{
  cGenotype * child_genotype = child_info->parent_genotype;

  // Prep the selected cpu to be changed.

  in_cpu->Kill();

  // If the creature is not an exact copy of the parent, find & initialize
  // its genotype.
  if (!child_info->copy_true) {
    child_genotype = genebank->AddGenotype(&(child_info->child_memory),
			     child_info->parent_genotype->GetSpecies());
    if (!child_genotype->GetNumCPUs())
      child_genotype->SetParent(child_info->parent_genotype);
  }

  // Report the new birth to the stats object
  local_stats.NewCreature(child_genotype->GetID(),
      child_info->parent_genotype->GetID(), 
      child_genotype->FindGeneticDistance(child_info->parent_genotype));

  // Setup this CPU with its new genotype!
  in_cpu->ChangeGenotype(child_genotype);

  // Setup the initial stats for this new creature.
  in_cpu->SetParentTrue(child_info->copy_true);
  in_cpu->SetCopiedSize(child_info->copied_size);
  in_cpu->SetExecutedSize(child_info->executed_size);
  in_cpu->SetGestationTime(child_info->gestation_time);
  in_cpu->SetFitness(child_info->fitness);
  in_cpu->InitPhenotype(child_info->parent_phenotype);

  // Finally, initialize the time-slice for this new creature.
  AdjustTimeSlice(in_cpu->GetEnvironment()->GetID());
}

int cAutoAdaptivePopulation::OK()
{
  int ret_value = TRUE;

  if (!cPopulation::OK()) ret_value = FALSE;

  for (int i = 0; i < num_cells; i++) {
    if (!cpu_array[i].OK()) {
      g_debug.Error("AAPop OK() failed in CPU #%d", i);
      ret_value = FALSE;
    }
  }

  return ret_value;
}


///////////////////////
//  cTierraPopulation
///////////////////////

cTierraPopulation::cTierraPopulation(const cGenesis & in_genesis)
 : cAutoAdaptivePopulation(in_genesis)
{
  int i;

  num_cells = Genesis().ReadInt("WORLD_SIZE");
  local_stats.SetNumCells(num_cells);

  printf("...Creating CPU's...\n");

  cpu_array = new cBaseCPU[num_cells];
  reaper_queue = new cList;
  
  for (i = 0; i < num_cells; i++) {
    cpu_array[i].GetEnvironment()->SetID(this, i);
    reaper_queue->InsertRear(&(cpu_array[i]));
  }

  BuildTimeSlicer();
}

cTierraPopulation::~cTierraPopulation()
{
}

void cTierraPopulation::ActivateChild(sReproData * child_info,
				      cBaseCPU * in_cpu)
{
  // Pick the CPU to be used.

  if (!in_cpu) in_cpu = (cBaseCPU *) reaper_queue->RemoveRear();

  cAutoAdaptivePopulation::ActivateChild(child_info, in_cpu);

  reaper_queue->InsertFront(in_cpu);
}

int cTierraPopulation::OK()
{
  int ret_value = TRUE;

  if (!cAutoAdaptivePopulation::OK()) ret_value = FALSE;
  if (!reaper_queue->OK()) ret_value = FALSE;

  return ret_value;
}


///////////////////////
//  cAvidaPopulation
///////////////////////

cAvidaPopulation::cAvidaPopulation(const cGenesis & in_genesis)
 : cAutoAdaptivePopulation(in_genesis)
{
  default_environment = new cTestEnvironment(NULL);
  default_environment->SetCopyMutRate(Genesis().ReadInt("COPY_MUT_RATE"));
  default_environment->SetPointMutRate(Genesis().ReadInt("POINT_MUT_RATE"));
  default_environment->SetDivideMutRate(Genesis().ReadInt("DIVIDE_MUT_RATE"));
  default_environment->SetDivideInsRate(Genesis().ReadInt("DIVIDE_INS_RATE"));
  default_environment->SetDivideDelRate(Genesis().ReadInt("DIVIDE_DEL_RATE"));

  // Genesis().ReadInt("GEN_RESOURCES");

  world_x = Genesis().ReadInt("WORLD-X");
  world_y = Genesis().ReadInt("WORLD-Y");
  num_cells = world_x * world_y;
  local_stats.SetNumCells(num_cells);

  printf("...Creating CPU's...\n");

  cpu_array = new cBaseCPU[num_cells];

  printf("...Establishing Connection Lists...\n");

  for (int cpu_id = 0; cpu_id < num_cells; cpu_id++) {
    cpu_array[cpu_id].GetEnvironment()->SetID(this, cpu_id);
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

cAvidaPopulation::~cAvidaPopulation()
{
}

void cAvidaPopulation::ActivateChild(sReproData * child_info,
				     cBaseCPU * in_cpu)
{
  // If no cpu was passed in, pick the one to be used here.

  if (!in_cpu) in_cpu = PositionChild((cBaseCPU *) child_info->parent_cpu);

  cAutoAdaptivePopulation::ActivateChild(child_info, in_cpu);

  // Face the child towards the parent (if it didn't kill it)

  if (in_cpu != child_info->parent_cpu) {
    in_cpu->GetEnvironment()->Rotate(child_info->parent_cpu);
  }
}


// This function directs which position function should be used.  It
// could have also been done with a function pointer, but the dividing
// of a CPU takes enough time that this will be a negligible addition,
// and it gives a centralized function to work with.
cBaseCPU * cAvidaPopulation::PositionChild(cBaseCPU * parent_cpu)
{
  // Save initial facing.
  cBaseCPU * facing = parent_cpu->GetEnvironment()->GetFacing();
  cList found_list;

  FindEmptyCell(parent_cpu->GetEnvironment()->GetConnections(), &found_list);

  // If we have not found an empty CPU, we must use the specified function
  // to determine how to choose among the filled CPU's.
  if (!found_list.GetSize()) {
    switch(birth_method) {
    case POSITION_CHILD_AGE:
      PositionAge(parent_cpu, &found_list);
      break;
    case POSITION_CHILD_MERIT:
      PositionMerit(parent_cpu, &found_list);
      break;
    case POSITION_CHILD_RANDOM:
      found_list = *(parent_cpu->GetEnvironment()->GetConnections());
      found_list.InsertFront(parent_cpu);
      break;
    case POSITION_CHILD_EMPTY:
      found_list.InsertFront(parent_cpu);
      break;
    }
  }

  // Restore the old facing
  parent_cpu->GetEnvironment()->Rotate(facing);

  // Choose the CPU randomly from those in the list, and return it.
  int choice = g_random.GetUInt(found_list.GetSize());

  cBaseCPU * found_cpu =  (cBaseCPU *) found_list[choice];

  // Print the relative location of the cpu being born.
  // static FILE * fp = fopen("birth_pos.dat", "w");
  // int offset = parent_cpu->GetID() - found_cpu->GetID();
  // if (offset < -1000) offset += population->GetSize();
  // if (offset > 1000) offset -= population->GetSize();
  // fprintf(fp, "%d\n", offset);

  return found_cpu;
}


void cAvidaPopulation::PositionAge(cBaseCPU * parent_cpu, cList * found_list)
{
  int max_age = parent_cpu->GetAge();
  cList * connection_list = parent_cpu->GetEnvironment()->GetConnections();
  cBaseCPU * test_cpu;

  found_list->InsertFront(parent_cpu);
  for (connection_list->Reset();
       connection_list->GetCurrent();
       connection_list->Next()) {
    test_cpu = (cBaseCPU *) connection_list->GetCurrent();
    
    if (test_cpu->GetAge() > max_age &&
	test_cpu->GetEnvironment()->TestResources()) {
      max_age = test_cpu->GetAge();
      found_list->Clear();
      found_list->InsertFront(test_cpu);
    }
    else if (test_cpu->GetAge() == max_age &&
	     test_cpu->GetEnvironment()->TestResources()) {
      found_list->InsertRear(test_cpu);
    }
  }
}

void cAvidaPopulation::PositionMerit(cBaseCPU * parent_cpu, cList * found_list)
{
  double min_ratio = parent_cpu->CalcMeritRatio();
  cList * connection_list = parent_cpu->GetEnvironment()->GetConnections();
  cBaseCPU * test_cpu;

  found_list->InsertFront(parent_cpu);
  for (connection_list->Reset();
       connection_list->GetCurrent();
       connection_list->Next()) {
    test_cpu = (cBaseCPU *) connection_list->GetCurrent();
    
    if (test_cpu->CalcMeritRatio() < min_ratio &&
	test_cpu->GetEnvironment()->TestResources()) {
      found_list->Clear();
      found_list->InsertFront(test_cpu);
    }
    else if (test_cpu->CalcMeritRatio() == min_ratio &&
	     test_cpu->GetEnvironment()->TestResources()) {
      found_list->InsertRear(test_cpu);
    }
  }
}

void cAvidaPopulation::FindEmptyCell(cList * cpu_list, cList * found_list)
{
  cBaseCPU * test_cpu;

  // See which cells are empty, and add them to the list.
  for (cpu_list->Reset();
       cpu_list->GetCurrent();
       cpu_list->Next()) {
    test_cpu = (cBaseCPU *) cpu_list->GetCurrent();
    
    // If this cell is empty, add it to the list...
    if (!test_cpu->GetActiveGenotype()) {
      found_list->InsertFront(test_cpu);
    }
  }
}  


char * cAvidaPopulation::GetBasicGrid() {
  int i;
  char * basic_grid;
  basic_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    basic_grid[i] = cpu_array[i].GetBasicSymbol();
  }

  return basic_grid;
}

char * cAvidaPopulation::GetSpeciesGrid() {
  int i;
  char * species_grid;
  species_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    species_grid[i] = cpu_array[i].GetSpeciesSymbol();
  }

  return species_grid;
}

char * cAvidaPopulation::GetInjectGrid() {
  int i;
  char * inject_grid;
  inject_grid =  g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    inject_grid[i] = cpu_array[i].GetInjectSymbol();
  }

  return inject_grid;
}

char * cAvidaPopulation::GetResourceGrid() {
  int i;
  char * resource_grid;
  resource_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    resource_grid[i] = cpu_array[i].GetResourceSymbol();
  }

  return resource_grid;
}

char * cAvidaPopulation::GetAgeGrid() {
  int i;
  char * age_grid;
  age_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    age_grid[i] = cpu_array[i].GetAgeSymbol();
  }

  return age_grid;
}

char * cAvidaPopulation::GetBreedGrid() {
  int i;
  char * breed_grid;
  breed_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    breed_grid[i] = cpu_array[i].GetBreedSymbol();
  }

  return breed_grid;
}

char * cAvidaPopulation::GetParasiteGrid() {
  int i;
  char * parasite_grid;
  parasite_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    parasite_grid[i] = cpu_array[i].GetParasiteSymbol();
  }

  return parasite_grid;
}

char * cAvidaPopulation::GetPointMutGrid() {
  int i;
  char * point_mut_grid;
  point_mut_grid = g_memory.Get(num_cells);

  for (i = 0; i < num_cells; i++) {
    point_mut_grid[i] = cpu_array[i].GetPointMutSymbol();
  }

  return point_mut_grid;
}

int cAvidaPopulation::OK()
{
  int ret_value = TRUE;

  if (!cAutoAdaptivePopulation::OK()) ret_value = FALSE;
  if (world_x * world_y != num_cells) {
    g_debug.Error("world_x * wolrd_y != num_cells");
    ret_value = FALSE;
  }
  
  return ret_value;
}
