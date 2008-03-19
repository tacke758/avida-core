//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <fstream.h>
#include <vector>

#include "avida.hh"
#include "population.hh"
#include "inst_lib.hh"
#include "genebank.hh"
#include "lineage.hh"
#include "lineage_control.hh"
#include "geneology.hh"
#include "slice.hh"
#include "../cpu/cpu.hh"
#include "species.hh"
#include "../cpu/environment.hh"

#include "../cpu/head.ii"
#include "../cpu/cpu.ii"


cPopulation::cPopulation()
{
  // Initialize the instruction set
  inst_lib = cInstLib::InitInstructions(cConfig::GetInstFilename());
  cConfig::SetNumInstructions(inst_lib->GetSize());

  // Default schedule.
  schedule = NULL;

  genebank = new cGenebank(this);

  // are we logging lineages?
  if (cConfig::GetLogLineages())
    lineage_control = new cLineageControl( genebank );
  else // no
    lineage_control = NULL;

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
    KillCreature( &(cpu_array[i]) );
  }
  delete [] cpu_array;
  delete genebank;
  if ( lineage_control )
    delete lineage_control;
  delete schedule;
}


void cPopulation::InitSoup()
{
  if (cConfig::GetCloneFilename() == "") {
    Inject(cConfig::GetStartCreature()(), 0);
  } else {
    ifstream fp(cConfig::GetCloneFilename()());
    LoadClone(fp);
  }

  cString fname(cConfig::GetLoadPopFilename());
  if (fname != "") {
    fprintf(stderr,"Loding Population from %s\n", fname());

    // If last three chars of filename are ".gz" , gunzip it first
    if( fname.Find(".gz") == fname.GetSize()-3 ){
      cString cmd(fname);
      cmd.Insert("gunzip ");
      fname.ClipEnd(3);
      system(cmd);

      ifstream fp(fname);
      if( !fp.good() ){
	fprintf(stderr, "ERROR: Failed to load population file %s\n",fname());
	exit(2);
      }
      LoadPopulation(fp);

      cmd = fname;
      cmd.Insert("gzip ");
      system(cmd);
    }else{
      // load normally
      ifstream fp(fname);
      LoadPopulation(fp);
    }
  }
}


void cPopulation::DoUpdate()
{

  // Update The Resources @TCC what about multiple task libs?
  if( cConfig::GetResources() == 1 ){
    cConfig::GetDefaultTaskLib()->UpdateResources();
  }


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

  // If the creature is not an exact copy of the parent, find & initialize
  // its genotype.
  if (!child_info.copy_true) {
    child_genotype = genebank->AddGenotype(child_info.child_memory,
					   child_info.parent_genotype);
    if (!child_genotype->GetNumCPUs())
      child_genotype->SetParent(*(child_info.parent_genotype));
  }

  // Report the new birth to the stats object
  cStats::NewCreature(child_genotype->GetID(),
      child_info.parent_genotype->GetID(),
      child_genotype->FindGeneticDistance(child_info.parent_genotype),
		      child_info.parent_genotype->GetLength(),
		      child_info.child_memory.GetSize() );
  cStats::ReportDivide(child_info.parent_cpu->cpu_stats);

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


  // Do lineage tracking for the new creature, if necessary
  // (the lineage tracking of the new creature has to happen
  // before the old creature has been killed, otherwise
  // the lineage control might try to add the child to a lineage that
  // has already been removed.
  // If you change anything here, make sure that you update also
  // cPopulation::LineageAddCreature(), which contains very similar code.
  cLineage *child_lineage = child_info.lineage;
  int child_lineage_label = child_info.lineage_label;

  if ( lineage_control ){
    child_lineage = lineage_control->AddCreature( child_genotype, child_info.parent_genotype, child_lineage, child_lineage_label );
    child_lineage_label = child_lineage->GetID();
  }


  // Prep the selected cpu to be changed,
  // but don't remove the genotype or reschedule the child
  // (for performance improvements, and also because if parent and
  // child genotype are identical, and if the parent genotype is the
  // last one registered of its kind, the child would lose its genotype
  // when the parent genotype gets removed).
  KillCreature( in_cpu, false, false );

  // Setup this CPU with its new genotype!
  in_cpu->ChangeGenotype(child_genotype);

  // Setup the initial stats for this new creature.
  in_cpu->SetParentTrue(child_info.copy_true);
  in_cpu->ChangeGenotype(child_genotype);

  // Setup the initial stats for this new creature.
  in_cpu->SetParentTrue(child_info.copy_true);
  in_cpu->SetCopiedSize(child_info.copied_size);
  in_cpu->SetExecutedSize(child_info.executed_size);
  in_cpu->SetGestationTime(child_info.gestation_time);
  in_cpu->SetFitness(child_info.fitness);
  in_cpu->InitPhenotype(child_info);
  in_cpu->GetEnvironment()->SetGeneration(child_info.parent_generation+1);
  in_cpu->SetFertile(child_info.child_fertile);

  // Mutation Info
  //in_cpu->SetMutInfo(child_info.mut_info);
  //child_info.mut_info.Clear();

  in_cpu->SetNeutralMetric(child_info.neutral_metric);
  in_cpu->SetLineage(child_lineage);
  in_cpu->SetLineageLabel(child_lineage_label);

  // ReCalc Merit if MERIT_TIME is 1 (for daughter & parent)
  if( cConfig::GetMeritTime() == 1 ){
    in_cpu->GetPhenotype()->ReCalcMerit();
    if( child_info.parent_cpu != in_cpu ){
      child_info.parent_cpu->GetPhenotype()->ReCalcMerit();
      AdjustTimeSlice(child_info.parent_cpu->GetEnvironment()->GetID());
    }
  }


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

  // Finally, Notify the viewer
  cAvidaMain::NotifyBirth(child_info.parent_cpu->GetEnvironment()->GetID(),
			  in_cpu->GetEnvironment()->GetID());
}


void cPopulation::KillCreature( cBaseCPU *cpu, bool clear_genotype,
				bool adjust_scheduler )
{
  // do we actually have something to kill?
  if ( cpu->GetActiveGenotype() != NULL ){
    // do the lineage handling
    if ( lineage_control )
      lineage_control->RemoveCreature( cpu );
    // reset the genotyp
    if ( clear_genotype )
      cpu->ChangeGenotype(NULL);
    // do statistics
    cpu->Kill();
    // adjust the scheduler
    if ( adjust_scheduler )
      schedule->Adjust( cpu );
  }
}


void cPopulation::LineageAddCreature( cBaseCPU *cpu, cLineage *lineage, int lineage_label )
{
  // if you modifiy anything here, make sure to update also
  // cPopulation::ActivateChild, which contains very similar code!
  if ( lineage_control ){
    lineage = lineage_control->AddCreature( cpu->GetActiveGenotype(), 0, lineage, lineage_label );
    lineage_label = lineage->GetID();
  }

  cpu->SetLineageLabel( lineage_label );
  cpu->SetLineage( lineage );
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
  int num_no_birth = 0;
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

      // Fitness
      cStats::SumFitness().Add(cur_cpu->GetFitness());

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
	if( cur_phenotype->GetTaskCount(j) > 0 ){
	  cStats::AddTasks(j,cur_phenotype->GetTaskCount(j));
	}
      }

      // Increment the counts for all qualities the CPU has...
      if (cur_cpu->GetParentTrue()) num_breed_true++;
      if (cur_cpu->IsParasite()) num_parasites++;
      if( cur_cpu->GetNumDivides() == 0 ) num_no_birth++;
#ifdef TEST_SEARCH_SIZE
      if( cur_cpu->GetSearchApproxSize() ){ cStats::AddSearchApproxSize(); }
#endif

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

    cStats::SumGenotypeAge().Add(cur_genotype->GetAge(),num_CPUs);

    // Repro Rate
    if( cur_genotype->GetReproRate() > 0 ){  // Don't average in infinities
      cStats::SumGestation().Add(1/(cur_genotype->GetReproRate()),num_CPUs);
    }

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
  cStats::SetNumNoBirthCreatures(num_no_birth);
  cStats::SetNumParasites(num_parasites);

  cStats::SetBestGenotype(genebank->GetBestGenotype());
  cStats::SetNumCreatures(GetNumCreatures());
  cStats::SetNumGenotypes(GetNumGenotypes());
  cStats::SetNumThreshSpecies(genebank->GetNumSpecies());

  // Have stats calculate anything it now can...
  cStats::CalcEnergy();
  cStats::CalcFidelity();
}


bool cPopulation::SaveClone(ofstream & fp)
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

  return (ret_val != 0);
}

bool cPopulation::LoadClone(ifstream & fp)
{
  int ret_val = fp.good();
  int i;
  int cur_update;
  fp >> cur_update;
  cStats::SetCurrentUpdate(cur_update);

  // Clear out the population
  for (i = 0; i < num_cells; i++) {
    if(cpu_array[i].GetActiveGenotype() != NULL){
      KillCreature( &(cpu_array[i]) );
    }
  }

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
      return false;
    }
    cpu_array[i].LoadCode(genotype_array[genotype_index].GetCode());
    AdjustTimeSlice(i);
  }

  cAvidaMain::SyncEventList();
  return (ret_val != 0);
}

bool cPopulation::DumpTextSummary(ofstream & fp)
{
  return genebank->DumpTextSummary(fp);
}

bool cPopulation::DumpDetailedSummary(ofstream & fp)
{
  return genebank->DumpDetailedSummary(fp);
}

bool cPopulation::DumpHistoricSummary(ofstream & fp)
{
  return genebank->DumpHistoricSummary(fp);
}



//// Save And Load Populations ////
int cPopulation::SavePopulation(ofstream & fp){
  g_debug.Comment("Saving Population...");
  int i;
  int rv = 0;
  if( !fp.good() ){
    g_debug.Error("File Error in SavePopulation");
    return -1;
  }

  // Print the update
  fp<<cStats::GetUpdate()<<endl;

  // looping through all CPUs saving various info
  for( i=0; i<num_cells; i++ ){
    // Save the actual cpu state
    cpu_array[i].SaveState(fp);
  }
  return rv;
}


int cPopulation::LoadPopulation(ifstream & fp){
  g_debug.Comment("Loading Population...");
  int i;
  int rv = 0;
  if( !fp.good() ){
    g_debug.Error("File Error in LoadPopulation");
    return -1;
  }

  // Load Update...
   int cur_update;
   fp >> cur_update;
   cStats::SetCurrentUpdate(cur_update);

   assert(fp.good());

  // Clear out the current population
  for (i = 0; i < num_cells; i++) {
    if(cpu_array[i].GetActiveGenotype() != NULL){
      KillCreature( &(cpu_array[i]) );
    }
  }

  // looping through all CPUs
  for( i=0; i<num_cells; i++ ){
    // Load Each CPU..
    //cerr<<"Loading CPU #"<<i<<endl;
    cpu_array[i].LoadState(fp);
  }

  cAvidaMain::SyncEventList();
  return rv;
}


//// Save And Load Organsim ////
int cPopulation::SaveOrganism(cString & filename, int cpu_num){
  g_debug.Comment("Saving Organism...");
  int rv = 0;

  // Assert cpu_num is valid!
  if( cpu_num < 0 || cpu_num > cConfig::GetWorldX()*cConfig::GetWorldY() ){
    g_debug.Error("CPU number out of range in SaveOrganism");
    return -1;
  }

  // Can't save a dead cell
  if( cpu_array[cpu_num].GetActiveGenotype() == NULL ){
    g_debug.Warning("SaveOrganism called on dead or nonexistant CPU");
    return -1;
  }

  // If filename is "", default to organism name .save
  if( filename == "" ){
    cString org_name(cpu_array[cpu_num].GetActiveGenotype()->GetName());
    filename.Set("genebank/%s.save", org_name());
  }

  ofstream fp(filename());

  if( !fp.good() ){
    g_debug.Error("File Error in SaveOrganism");
    return -1;
  }

  cpu_array[cpu_num].SaveState(fp);

  return rv;
}


int cPopulation::LoadOrganism(cString & filename, int cpu_num,
			      int lineage_label){
  int rv = 0;
  g_debug.Comment("Loading Organsim...");

  // Assert cpu_num is valid!
  if( cpu_num < 0 || cpu_num > cConfig::GetWorldX()*cConfig::GetWorldY() ){
    g_debug.Error("CPU number out of range in LoadOrganism");
    return -1;
  }

  ifstream fp(filename());
  if( !fp.good() ){
    g_debug.Error("File Error in LoadOrganism");
    return -1;
  }

  KillCreature( &(cpu_array[cpu_num]) );

  cpu_array[cpu_num].LoadState(fp);
  LineageAddCreature( &(cpu_array[cpu_num]), 0, lineage_label );
  AdjustTimeSlice(cpu_num);

  return rv;
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
    KillCreature( &(cpu_array[i]) );
  }
}

void cPopulation::Inject(const char * filename, int in_cpu, double merit, int lineage_label, double neutral_metric )
{
  KillCreature( &(cpu_array[in_cpu]) );
  cpu_array[in_cpu].LoadCode(filename);
  cpu_array[in_cpu].SetNeutralMetric( neutral_metric );

  if ( merit > 0 ){
    cpu_array[in_cpu].GetPhenotype()->SetMerit( merit );
  }
  LineageAddCreature( &(cpu_array[in_cpu]), 0, lineage_label );
  AdjustTimeSlice(in_cpu);
}

void cPopulation::InjectAll(const char * filename)
{
  Inject(filename, 0);
  cGenotype * new_genotype = GetCPU(0).GetActiveGenotype();
  int lineage_label = GetCPU(0).GetLineageLabel();
  cLineage *lineage = 0;//GetCPU(0).GetLineage();
  for(int i = 1; i < GetSize(); i++) {
    KillCreature( &( GetCPU(i) ) );
    GetCPU(i).ChangeGenotype(new_genotype);
    GetCPU(i).SetParentTrue(TRUE);
    GetCPU(i).SetCopiedSize(new_genotype->GetLength());
    GetCPU(i).GetPhenotype()->Clear(new_genotype->GetLength());
    GetCPU(i).SetInjected();
    LineageAddCreature( &( GetCPU(i) ), lineage, lineage_label );
    AdjustTimeSlice(i);
  }
}

void cPopulation::InjectRandom(int mem_size)
{
  UINT cpu_used = g_random.GetUInt(num_cells);
  cCodeArray code_used(mem_size);
  code_used.Randomize();
  cpu_array[cpu_used].LoadCode(code_used);
  LineageAddCreature( &(cpu_array[cpu_used]), 0, 0 );
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

char * cPopulation::GetLineageGrid()
{
  char * lineage_grid = g_memory.Get(num_cells);

  for (int i = 0; i < num_cells; i++) {
    lineage_grid[i] = cpu_array[i].GetLineageSymbol();
  }

  return lineage_grid;
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
  if ( lines_saved >0 ) {
    fprintf(fp_abundance, "\n");
    fprintf(fp_var,       "\n");
    fprintf(fp_entropy,   "\n");
  }

  // Study the consensus creature.

  con_code.Resize(con_length);

  // Loop through genotypes again, and determine the average genetic
  // distance.

  cur_genotype = genebank->GetBestGenotype();
  cDoubleSum distance_sum;
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

  cString con_name;
  con_name.Set("genebank/%03d-consensus-u%i.gen", con_code.GetSize(), cStats::GetUpdate() );
  cTestCPU::PrintCode( con_code, con_name() );


  if (con_genotype) {
    fprintf(fp,
	    "%d %5.0f %f %5.3f %f %d %f %f %d %d %d %d %d %d %d %d %f %f %f %f\n",
	    cStats::GetUpdate(),                       //  1
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
	    test_info.GetColonyMerit(),                //  2
	    test_info.GetColonyGestation(),            //  3
	    test_info.GetColonyFitness(),                //  4
	    1.0 / (0.1  + test_info.GetColonyGestation()),     //  5
	    con_code.GetSize(),                   //  6
	    test_info.GetColonyCopiedSize(),             //  7
	    test_info.GetColonyExeSize(),           //  8
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
cCodeArray cPopulation::CalcLandscape(int dist, const cCodeArray & code)
{
  cLandscape landscape(code, inst_lib);
  landscape.Process(dist);

  // Print the results.

  static ofstream fp("landscape.dat");
  static ofstream fp_entropy("land-entropy.dat");
  static ofstream fp_count("land-sitecount.dat");

  landscape.PrintStats(fp);
  landscape.PrintEntropy(fp_entropy);
  landscape.PrintSiteCount(fp_count);

  // Repeat for Insertions...
  landscape.Reset(code);
  landscape.ProcessInsert();
  static ofstream fp_ins("landscape-ins.dat");
  static ofstream fp_ins_count("land-ins-sitecount.dat");
  landscape.PrintStats(fp_ins);
  landscape.PrintSiteCount(fp_ins_count);

  // And Deletions...
  landscape.Reset(code);
  landscape.ProcessDelete();
  static ofstream fp_del("landscape-del.dat");
  static ofstream fp_del_count("land-del-sitecount.dat");
  landscape.PrintStats(fp_del);
  landscape.PrintSiteCount(fp_del_count);

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

// Returns the code-array of maximal fitness.
cCodeArray cPopulation::PredictWLandscape(const cCodeArray & code,
					cString & filename)
{
  cLandscape landscape(code, inst_lib);
  static ofstream fp(filename);
  landscape.PredictWProcess(fp);

  // return cCodeArray(const_cast(const cCodeArray, landscape.GetPeakCode()));
  return cCodeArray(const_cast(const cCodeArray &, landscape.GetPeakCode()), 0);
}

cCodeArray cPopulation::PredictNuLandscape(const cCodeArray & code,
					cString & filename)
{
  cLandscape landscape(code, inst_lib);
  static ofstream fp(filename);
  landscape.PredictNuProcess(fp);

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

void cPopulation::AnalyzeLandscape(const cCodeArray & code, int sample_size,
				   int min_found, int max_sample_size)
{
  cLandscape landscape(code, inst_lib);

  static ofstream fp("land_analyze.dat");

  int num_found = 0;

  for (int dist = 1; dist <= 10; dist++) {
    landscape.Reset(code);
    if (dist == 1) {
      landscape.Process(dist);
      num_found = code.GetSize() * (inst_lib->GetSize() - 1);
    } else {
      num_found =
	landscape.RandomProcess(sample_size, dist, min_found, max_sample_size);
    }

    fp << cStats::GetUpdate()          << " "  // 1
       << dist                         << " "  // 2
       << landscape.GetProbDead()      << " "  // 3
       << landscape.GetProbNeg()       << " "  // 4
       << landscape.GetProbNeut()      << " "  // 5
       << landscape.GetProbPos()       << " "  // 6
       << landscape.GetNumTrials()     << " "  // 7
       << num_found                    << " "  // 8
       << landscape.GetAveFitness()    << " "  // 9
       << landscape.GetAveSqrFitness() << " "  // 10
       << endl;

    if ((dist > 1) && (num_found < min_found)) break;
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

void cPopulation::AnalyzeTasksites(const cCodeArray & code)
{
  cLandscape landscape(code, inst_lib);

  cString filename;
  filename.Set("tasksites.%d.dat", cStats::GetUpdate());
  ofstream fp(filename());

  landscape.TestTasksites(fp);
}

void cPopulation::TestCode(const cCodeArray & code)
{
  cCPUTestInfo test_info;
  cTestCPU::TestCode(test_info, code);

  static ofstream fp("dom-test.dat");

  fp << cStats::GetUpdate() << " "                      //  1
     << test_info.GetColonyMerit() << " "                    //  2
     << test_info.GetColonyGestation() << " "                //  3
     << test_info.GetColonyFitness() << " "                  //  4
     << 1.0 / (0.1  + test_info.GetColonyGestation()) << " " //  5
     << code.GetSize() << " "                           //  6
     << test_info.GetColonyCopiedSize() << " "               //  7
     << test_info.GetColonyExeSize() << endl;                //  8
}

void cPopulation::HillClimb(const cCodeArray & code)
{
  ofstream fp("hillclimb.dat");
  cLandscape landscape(code, inst_lib);
  landscape.HillClimb(fp);
}

void cPopulation::HillClimb_Neut(const cCodeArray & code)
{
  ofstream fp("hillclimb.dat");
  cLandscape landscape(code, inst_lib);
  landscape.HillClimb_Neut(fp);
}

void cPopulation::HillClimb_Rand(const cCodeArray & code)
{
  ofstream fp("hillclimb.dat");
  cLandscape landscape(code, inst_lib);
  landscape.HillClimb_Rand(fp);
}

///// Called By cEvents.... //////
void cPopulation::SetCopyMutProb(double new_cmut, int cell){
  // @TCC -- If only on cell is set, cConfig::CopyMutProb will be wrong!!!!
  if( cell < 0 ){ // cell=-1 => all
    for( int i=0; i < GetSize(); i++ ){
      GetCPU(i).GetEnvironment()->SetCopyMutProb(new_cmut);
    }
    cConfig::SetCopyMutProb(new_cmut);
  }else{
      GetCPU(cell).GetEnvironment()->SetCopyMutProb(new_cmut);
  }
}

void cPopulation::SetPointMutProb(double new_pmut, int cell){
  // @TCC -- If only on cell is set, cConfig::PointMutProb will be wrong!!!!
  if( cell < 0 ){ // cell=-1 => all
    for( int i=0; i < GetSize(); i++ ){
      GetCPU(i).GetEnvironment()->SetPointMutProb(new_pmut);
    }
    cConfig::SetPointMutProb(new_pmut);
  }else{
      GetCPU(cell).GetEnvironment()->SetPointMutProb(new_pmut);
  }
}

void cPopulation::PrintDepthHistogram(const cString & filename){
  // Output format:
  //    update min max histogram_values...
  int i;
  int min = INT_MAX;
  int max = 0;
  cGenotype * cur_genotype;
  ofstream & fp = cStats::GetDataFileOFStream(filename);
  assert(fp.good());

  // Two pass method

  // Loop through all genotypes getting min and max values
  cur_genotype = genebank->GetBestGenotype();
  for( i=0; i<genebank->GetSize(); ++i ){
    if( cur_genotype->GetDepth() < min ){ min = cur_genotype->GetDepth(); }
    if( cur_genotype->GetDepth() > max ){ max = cur_genotype->GetDepth(); }
    cur_genotype = cur_genotype->GetNext();
  }
  assert(max >= min);

  // Allocate the array for the bins (& zero)
  int * n = new int[max-min+1];
  for( i=0; i<max-min+1; ++i ){
    n[i] = 0;
  }

  // Loop through all genotypes binning the values
  cur_genotype = genebank->GetBestGenotype();
  for( i=0; i<genebank->GetSize(); ++i ){
    n[cur_genotype->GetDepth() - min] += cur_genotype->GetNumCPUs();
    cur_genotype = cur_genotype->GetNext();
  }

  // Actual output
  fp<< cStats::GetUpdate() <<" "
    << min <<" "
    << max ;

  for( i=0; i<max-min+1; ++i ){
    fp<<" "<< n[i];
  }
  fp<<endl;
}


void cPopulation::PrintGenotypeAbundanceHistogram(const cString & filename){
  int i;
  cGenotype * cur_genotype;
  ofstream & fp = cStats::GetDataFileOFStream(filename);
  assert(fp.good());

  // Allocate array for the histogram & zero it
  tArray <int> hist(genebank->GetBestGenotype()->GetNumCPUs());
  for( i=0; i<hist.GetSize(); ++i ){
    hist[i] = 0;
  }

  // Loop through all genotypes binning the values
  cur_genotype = genebank->GetBestGenotype();
  for( i=0; i<genebank->GetSize(); ++i ){

    assert( cur_genotype->GetNumCPUs() - 1 >= 0 );
    assert( cur_genotype->GetNumCPUs() - 1 < hist.GetSize() );
    ++ hist[cur_genotype->GetNumCPUs() - 1];
    cur_genotype = cur_genotype->GetNext();
  }

  // Actual output
  fp<< cStats::GetUpdate() <<" ";
  for( i=0; i<hist.GetSize(); ++i ){
    fp<<hist[i]<<" ";
  }
  fp<<endl;
}


void cPopulation::PrintSpeciesAbundanceHistogram(const cString & filename){
  int i;
  int max = 0;
  cSpecies * cur_species;
  ofstream & fp = cStats::GetDataFileOFStream(filename);
  assert(fp.good());

  // Find max species abundance...
  cur_species = genebank->GetFirstSpecies();
  for (i = 0; i < genebank->GetNumSpecies(); i++) {
    if( max < cur_species->GetNumCreatures() ){
      max = cur_species->GetNumCreatures();
    }
    cur_species = cur_species->GetNext();
  }

  // Allocate array for the histogram & zero it
  tArray <int> hist(max);
  for( i=0; i<hist.GetSize(); ++i ){
    hist[i] = 0;
  }

  // Loop through all species binning the values
  cur_species = genebank->GetFirstSpecies();
  for (i = 0; i < genebank->GetNumSpecies(); i++) {
    assert( cur_species->GetNumCreatures() - 1 >= 0 );
    assert( cur_species->GetNumCreatures() - 1 < hist.GetSize() );
    ++hist[cur_species->GetNumCreatures() -1];
    cur_species = cur_species->GetNext();
  }

  // Actual output
  fp<< cStats::GetUpdate() <<" ";
  for( i=0; i<hist.GetSize(); ++i ){
    fp<<hist[i]<<" ";
  }
  fp<<endl;
}


void cPopulation::TestInsSizeChangeRobustness(const cString & filename,
					      const cCodeArray & in_code,
					      int num_trials){
  ofstream & fp = cStats::GetDataFileOFStream(filename);
  cCPUTestInfo test_info;
  cInstruction inst_none = inst_lib->GetInst("instruction_none");
  int num_nops;
  int ins_pos;
  int i,j;

  // Stats
  int num_viable = 0;
  int num_new_size = 0;
  int num_parent_size = 0;

  for( i=0; i<num_trials; ++i ){

    cCodeArray code(in_code);
    // Should check to only insert infront of an instruction (not a Nop)
    ins_pos = -1;
    while( ins_pos < 0 ){
      ins_pos = g_random.GetUInt(code.GetSize());
      if( inst_lib->IsNop(code.Get(ins_pos)) ){
	ins_pos = -1;
      }
    }

    // Insert some "instruction_none" into the code
    num_nops = g_random.GetUInt(5) + 5;
    for( j=0; j<num_nops; ++j ){
      code.Insert(ins_pos, inst_none);
    }

    // Test the code and output stats
    double child_size = 0.0;
    if( cTestCPU::TestCode(test_info, code) ){ // Daughter viable...
      ++num_viable;
      child_size = test_info.GetColonyCodeSize();

      if( child_size == (double) in_code.GetSize() ){
	++num_parent_size;
      } else if( child_size == (double) code.GetSize() ){
	++num_new_size;
      }
    }

    // cerr<< num_viable <<" "
	// << in_code.GetSize() <<" "
	// << num_nops <<" "
	// << code.GetSize() <<" "
	// << (in_code.GetSize()+num_nops) <<" "
	// << child_size <<" "
	// << test_info.GetGenotypeCopyTrue() <<" "
	// <<endl;


  } // for num_trials

  fp<< cStats::GetUpdate() <<" "
    << (double) num_viable / num_trials <<" "
    << (double) num_new_size / num_trials <<" "
    << (double) num_parent_size / num_trials <<endl;

}


void cPopulation::PrintTreeDepths()
{
  cString filename;
  filename.Set("tree_depth.%d.dat", cStats::GetUpdate());
  ofstream fp(filename);

  // cycle over all genotypes
  cGenotype * genotype = genebank->GetBestGenotype();
  for (int i = 0; i < genebank->GetSize(); i++) {
    fp << genotype->GetID() << " "
       << genotype->GetTestFitness() << " "
       << genotype->GetNumCPUs() << " "
       << genotype->GetDepth() << " "
       << endl;

    // ...and advance to the next genotype...
    genotype = genotype->GetNext();
  }
}

void cPopulation::Apocalypse(double kill_prob){
  for( int i=0; i<cConfig::GetWorldX()*cConfig::GetWorldY(); ++i ){
    if(cpu_array[i].GetActiveGenotype() != NULL){
      if( g_random.P(kill_prob) ){
	KillCreature( &(cpu_array[i]) );
      }
    }
  }
}


void cPopulation::RateKill(double kill_rate){
  double ave = cStats::SumMerit().Average();
  double kill_prob = cConfig::GetAveTimeslice()*(kill_rate/ave);
  for( int i=0; i<cConfig::GetWorldX()*cConfig::GetWorldY(); ++i ){
    if(cpu_array[i].GetActiveGenotype() != NULL){
      if( g_random.GetDouble() < kill_prob ){
	KillCreature( &(cpu_array[i]) );
      }
    }
  }
}


void cPopulation::AnalyzePopulation(ofstream & fp, double sample_prob, bool landscape, bool save_genotype)
{
  cGenotype *genotype;

  fp << "# (1) cell number (2) genotype name (3) length (4) fitness [test-cpu] (5) fitness (actual) (6) merit (7) no of breed trues occurred (8) lineage label (9) neutral metric (10) -... landscape data" << endl;

  for (int i = 0; i < num_cells; i++) {
    genotype = cpu_array[i].GetActiveGenotype();
    if ( genotype != NULL && // is the creature alive?
         g_random.P(sample_prob)  ){ // do we want to sample it?
      // run test cpu on this creature
      cCodeArray code = genotype->GetCode();

      cString creature_name;
      if ( genotype->GetThreshold() )
	creature_name = genotype->GetName();
      else // we put the current update and the cell number into the name,
	// so that it becomes unique.
	creature_name.Set("%03d-no_name-u%i-c%i", genotype->GetLength(),
		    cStats::GetUpdate(), i );
      fp << i                              << " "  // 1 cell number
         << creature_name()                << " "  // 2 name
         << genotype->GetLength()          << " "  // 3 length
         << genotype->GetTestFitness()     << " "  // 4 fitness (test-cpu)
	 << cpu_array[i].GetFitness()      << " "  // 5 fitness (actual)
	 << cpu_array[i].GetMerit()        << " "  // 6 merit
         << genotype->GetBreedTrue()       << " "  // 7 breed true?
	 << cpu_array[i].GetLineageLabel() << " "  // 8 lineage label
	 << cpu_array[i].GetNeutralMetric() << " "; // 9 neutral metric

      // create landscape object for this creature
      if ( landscape &&  genotype->GetTestFitness()> 0 ) {
        cLandscape landscape( code, inst_lib);
        landscape.Process(1);
        landscape.PrintStats(fp);
      }
      else fp << endl;
      if ( save_genotype ){
        char filename[40];
        sprintf( filename, "genebank/%s", creature_name() );
        cTestCPU::PrintCode( code, filename );
      }
    }
  }
}


void cPopulation::PrintDetailedFitnessData(ofstream & datafp, ofstream & histofp, ofstream & histo_testCPU_fp, bool save_max_f_genotype, bool print_fitness_histo, double hist_fmax, double hist_fstep)
{
  cGenotype *genotype, *max_f_genotype;

  double f, f_testCPU, fave, fave_testCPU;
  int n, nhist_tot, nhist_tot_testCPU;
  double max_fitness;

  // the histogram variables
  vector<int> histo;
  vector<int> histo_testCPU;
  int bins;

  if ( print_fitness_histo ){
    bins = (int) (hist_fmax / hist_fstep) + 1;
    histo.resize( bins, 0.0 ); // resize and clear
    histo_testCPU.resize( bins, 0.0 );
  }

  n = nhist_tot = nhist_tot_testCPU = 0;
  fave = fave_testCPU = 0;
  max_fitness = -1; // we set this to -1, so that even 0 is larger...
  max_f_genotype = 0;

  for (int i = 0; i < num_cells; i++) {
    genotype = cpu_array[i].GetActiveGenotype();
    if ( genotype != NULL ) { // is the creature alife?

      cCPUTestInfo test_info;
      cTestCPU::TestCode( test_info, genotype->GetCode() );
      // We calculate the fitness based on the current merit,
      // but with the true gestation time. Also, we set the fitness
      // to zero if the creature is not viable.
      if ( test_info.IsViable() )
	f = cpu_array[i].GetMerit().CalcFitness( test_info.GetGenotypeGestation() );
      else f = 0;
      f_testCPU = test_info.GetColonyFitness();

      // get the maximum fitness in the population
      // here, we want to count only creatures that can truly replicate,
      // to avoid complications
      if ( (f_testCPU > max_fitness) && test_info.GetGenotypeCopyTrue() ){
	max_fitness = f_testCPU;
	max_f_genotype = genotype;
      }

      fave += f;
      fave_testCPU += f_testCPU;
      n += 1;


      // histogram
      if ( print_fitness_histo && f < hist_fmax ){
	histo[(int) (f / hist_fstep)] += 1;
	nhist_tot += 1;
      }

      if ( print_fitness_histo && f_testCPU < hist_fmax ){
	histo_testCPU[(int) (f_testCPU / hist_fstep)] += 1;
	nhist_tot_testCPU += 1;
      }

    }
  }

  // determine the name of the maximum fitness genotype
  cString max_f_name;
  if ( max_f_genotype->GetThreshold() )
    max_f_name = max_f_genotype->GetName();
  else // we put the current update into the name, so that it becomes unique.
     max_f_name.Set("%03d-no_name-u%i", max_f_genotype->GetLength(),
		    cStats::GetUpdate() );

  datafp << cStats::GetUpdate()            << " "  // 1 update
     << cStats::GetAveGeneration()     << " "  // 2 generation
     << fave/ (double) n               << " "  // 3 average fitness total
     << fave_testCPU/ (double) n       << " "  // 4 average fitness (test CPU)
     << n 			       << " "  // 5 creatures total
     << max_fitness                    << " "  // 6 maximum fitness
     << max_f_name()		       << " "  // 7 name of the max. fitness genotype
     << endl;

  if ( save_max_f_genotype ){
    char filename[40];
    sprintf( filename, "genebank/%s", max_f_name() );
    cTestCPU::PrintCode( max_f_genotype->GetCode(), filename );
  }

  if ( print_fitness_histo ) {
    histofp << cStats::GetUpdate()            << " "  // 1 update
	    << cStats::GetAveGeneration()     << " "  // 2 generation
	    << fave/ (double) n               << " ";  // 3 average fitness total
    // now output the fitness histo
    vector<int>::const_iterator it = histo.begin();
    for ( ; it != histo.end(); it++ )
      histofp << (double) (*it) / (double) nhist_tot << " ";
    histofp << endl;

    histo_testCPU_fp << cStats::GetUpdate()            << " "  // 1 update
	    << cStats::GetAveGeneration()     << " "  // 2 generation
	    << fave_testCPU/ (double) n               << " ";  // 3 average fitness total
    // now output the fitness histo
    it = histo_testCPU.begin();
    for ( ; it != histo_testCPU.end(); it++ )
      histo_testCPU_fp << (double) (*it) / (double) nhist_tot_testCPU << " ";
    histo_testCPU_fp << endl;
  }
}


void cPopulation::PrintGeneticDistanceData(ofstream &fp, const char* creature_name)
{
  cGenotype *cur_genotype;
  double hamming_m1 = 0;
  double hamming_m2 = 0;
  int count = 0;
  int dom_dist = 0;

  // load the reference code
  cCodeArray reference_code( cBaseCPU::LoadCode( creature_name, *inst_lib ) );

  // get the info for the dominant genotype
  cur_genotype = genebank->GetBestGenotype();
  cCodeArray code = cur_genotype->GetCode();
  dom_dist = reference_code.FindHammingDistance( code );
  hamming_m1 += dom_dist;
  hamming_m2 += dom_dist*dom_dist;
  count += cur_genotype->GetNumCPUs();
  // now cycle over the remaining genotypes
  for (int i = 1; i < genebank->GetSize(); i++) {
    cur_genotype = cur_genotype->GetNext();
    cCodeArray code = cur_genotype->GetCode();

    int dist = reference_code.FindHammingDistance( code );
    hamming_m1 += dist;
    hamming_m2 += dist*dist;
    count += cur_genotype->GetNumCPUs();
  }

  hamming_m1 /= (double) count;
  hamming_m2 /= (double) count;

  fp << cStats::GetUpdate()                  << " "  // 1 update
     << cStats::GetAveGeneration()           << " "  // 2 generation
     << hamming_m1 			     << " "  // ave. Hamming dist
     << sqrt( ( hamming_m2 - hamming_m1*hamming_m1 )/ (double) count )
                                             << " "  // std. error
     << reference_code.FindHammingDistance( genebank->GetBestGenotype()->GetCode() ) << " " // Hamming dist. to dominant
     << endl;
}


void cPopulation::GeneticDistancePopDump(ofstream &fp, const char* creature_name, bool save_creatures)
{
  cGenotype *cur_genotype;
  double sum_fitness;
  int sum_num_CPUs;

  sum_fitness = 0;
  sum_num_CPUs = 0;

  // load the reference code
  cCodeArray reference_code( cBaseCPU::LoadCode( creature_name, *inst_lib ) );

  // first, print out some docu...
  fp << "# (1) genotype name (2) fitness [test-cpu] (3) abundance (4) Hamming distance to reference (5) Levenstein distance to reference" << endl;
  fp << "# reference code is the START_CREATURE" << endl;

  // cycle over all genotypes
  cur_genotype = genebank->GetBestGenotype();
  for (int i = 0; i < genebank->GetSize(); i++) {
    cCodeArray code = cur_genotype->GetCode();

    // now output

    sum_fitness += cur_genotype->GetTestFitness()*cur_genotype->GetNumCPUs();
    sum_num_CPUs += cur_genotype->GetNumCPUs();

    fp << cur_genotype->GetName()() << " "      // 1 name
       << cur_genotype->GetTestFitness() << " "    // 2 fitness
       << cur_genotype->GetNumCPUs() << " " // 3 abundance
       << reference_code.FindHammingDistance( code ) << " " // 4 hamming distance
       << reference_code.FindLevensteinDistance( code ) << " "// 5 levenschtein dist.
       << cur_genotype->GetCode().AsString()() << " " // 6 code
       << endl;

    // save into genebank
    if ( save_creatures ) {
      char filename[40];
      sprintf( filename, "genebank/%s", cur_genotype->GetName()() );
      cTestCPU::PrintCode( code, filename );
    }

    // ...and advance to the next genotype...
    cur_genotype = cur_genotype->GetNext();
  }
  fp << "# ave fitness from Test CPU's: " << sum_fitness/sum_num_CPUs << endl;
}




void cPopulation::TaskSnapshot(ofstream & fp)
{
  cPhenotype *phenotype, *test_phenotype;

  fp << "# (1) cell number\n# (2) number of rewarded tasks done so far\n# (3) total number of tasks done so far\n# (4) same as 2, but right before divide\n# (5) same as 3, but right before divide\n# (6) same as 2, but for parent\n# (7) same as 3, but for parent\n# (8) genotype fitness\n# (9) genotype name" << endl;

  for (int i = 0; i < num_cells; i++) {
    // create a test-cpu for the current creature
    cCPUTestInfo test_info;
    cTestCPU::TestCode(
       test_info, cpu_array[i].GetActiveGenotype()->GetCode() );
    test_phenotype = cTestCPU::GetPhenotype( 0 );

    phenotype = cpu_array[i].GetPhenotype();

    int num_tasks = phenotype->GetNumTasks();

    int sum_tasks_all = 0;
    int sum_tasks_rewarded = 0;
    int divide_sum_tasks_all = 0;
    int divide_sum_tasks_rewarded = 0;
    int parent_sum_tasks_all = 0;
    int parent_sum_tasks_rewarded = 0;
    for ( int j=0; j<num_tasks; j++ ){
      // get the number of bonuses for this task
      int bonuses = phenotype->GetTaskLib()->GetTaskNumBonus(j);

      int task_count = ( phenotype->GetTaskCount( j ) == 0 ) ? 0 : 1;
      int divide_tasks_count = ( test_phenotype->GetTaskCount( j ) == 0 )
			       ? 0 : 1;
      int parent_task_count = ( phenotype->GetParentTaskCount( j ) == 0 )
			      ? 0 : 1;

      // if there is only one bonus, this task is not rewarded,
      // as the last bonus is always + 0.
      if ( bonuses > 1 ){
	sum_tasks_rewarded += task_count;
	divide_sum_tasks_rewarded += divide_tasks_count;
	parent_sum_tasks_rewarded += parent_task_count;
      }
      sum_tasks_all += task_count;
      divide_sum_tasks_all += divide_tasks_count;
      parent_sum_tasks_all += parent_task_count;
    }

    fp << i                              << " "  // 1 cell number
       << sum_tasks_rewarded             << " "  // 2 number of tasks rewarded
       << sum_tasks_all                  << " "  // 3 total number of tasks done
       << divide_sum_tasks_rewarded      << " "  // 4 number of rewarded tasks on divide
       << divide_sum_tasks_all           << " "  // 5 number of total tasks on divide
       << parent_sum_tasks_rewarded      << " "  // 6 parent number of tasks rewared
       << parent_sum_tasks_all 		 << " "  // 7 parent total number of tasks done

       << test_info.GetColonyFitness()   << " "  // 8 genotype fitness
       << cpu_array[i].GetActiveGenotype()->GetName()() << " " // 9 genotype name
       << endl;
  }
}


void cPopulation::PrintViableTasksData(ofstream & fp)
{
  cPhenotype *phenotype;

  static vector<int> tasks;
  vector<int>::iterator it;

  int num_tasks = cConfig::GetNumTasks();
  
  tasks.resize( num_tasks );
  
  // clear task vector
  for ( it = tasks.begin(); it != tasks.end(); it++ )
    (*it) = 0;
  
  for (int i = 0; i < num_cells; i++) {
    
    // creature is viable?
    if ( cpu_array[i].GetActiveGenotype()->GetTestFitness() > 0 ) {
      phenotype = cpu_array[i].GetPhenotype();

      for ( int j=0; j<num_tasks; j++ ){
	if ( phenotype->GetTaskCount( j ) > 0 ) 
	  tasks[j] += 1;
      }
    }
  }
  
  fp << cStats::GetUpdate();
  for( it = tasks.begin(); it != tasks.end(); it++ )
    fp <<" "<< (*it);
  
  fp<<endl;
  
}
