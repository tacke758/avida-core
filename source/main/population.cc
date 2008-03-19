//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../cpu/hardware_method.hh"
#include "population.hh"

#include <fstream>

#include <vector>
#include <algorithm>

#include "../tools/functions.hh"
#include "../tools/slice.hh"

#include "config.hh"
#include "genebank.hh"
#include "genome_util.hh"
#include "genotype.hh"
#include "inst_util.hh"
#include "lineage.hh"
#include "lineage_control.hh"
#include "organism.hh"
#include "phenotype.hh"
#include "population_cell.hh"
#include "species.hh"
#include "stats.hh"
#include "tasks.hh"

#include "../cpu/hardware_base.hh"
#include "../cpu/hardware_factory.hh"
#include "../cpu/hardware_util.hh"


using namespace std;


cPopulation::cPopulation(const cPopulationInterface & in_interface,
			 cEnvironment & in_environment)
  : schedule(NULL)
  , resource_count(in_environment.GetResourceLib().GetSize())
  , environment(in_environment)
  , default_interface(in_interface)
  , num_organisms(0)
  , sync_events(false)
{
  cout << "<cPopulation>" << endl;

  // Setup the genebank.
  genebank = new cGenebank(stats);

  // are we logging lineages?
  if (cConfig::GetLogLineages()) {
    lineage_control = new cLineageControl( *genebank, stats );
  }
  else lineage_control = NULL;    // no lineage logging

  // Setup the default mutation rates...
  cMutationRates & default_mut_rates = environment.GetMutRates();
  default_mut_rates.SetCopyMutProb  ( cConfig::GetCopyMutProb()   );
  default_mut_rates.SetInsMutProb   ( cConfig::GetInsMutProb()    );
  default_mut_rates.SetDelMutProb   ( cConfig::GetDelMutProb()    );
  default_mut_rates.SetDivMutProb   ( cConfig::GetDivMutProb()    );
  default_mut_rates.SetPointMutProb ( cConfig::GetPointMutProb()  );
  default_mut_rates.SetDivideMutProb( cConfig::GetDivideMutProb() );
  default_mut_rates.SetDivideInsProb( cConfig::GetDivideInsProb() );
  default_mut_rates.SetDivideDelProb( cConfig::GetDivideDelProb() );
  default_mut_rates.SetParentMutProb( cConfig::GetParentMutProb() );

  // Setup the default population interface...
  default_interface.SetPopulation(this);

  // Avida specific information.
  world_x = cConfig::GetWorldX();
  world_y = cConfig::GetWorldY();
  const int num_cells = world_x * world_y;
  cout << "Building world " << world_x << "x" << world_y
       << " = " << num_cells << " organisms." << endl;

  cell_array.Resize(num_cells);
  resource_count.ResizeSpatialGrids(world_x, world_y);

  for (int cell_id = 0; cell_id < num_cells; cell_id++) {
    cell_array[cell_id].Setup(cell_id, default_mut_rates);

    // Setup the connection list for each cell. (Clockwise from -1 to 1)
    tList<cPopulationCell> & conn_list = cell_array[cell_id].ConnectionList();
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y, -1, -1)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y,  0, -1)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y, +1, -1)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y, +1,  0)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y, +1, +1)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y,  0, +1)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y, -1, +1)]));
    conn_list.Push(&(cell_array[Neighbor(cell_id, world_x, world_y, -1,  0)]));

    // Setup the reaper queue...
    if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_ELDEST) {
      reaper_queue.Push(&(cell_array[cell_id]));
    }
  }

  BuildTimeSlicer();

  // Setup the resources...
  const cResourceLib & resource_lib = environment.GetResourceLib();
  for (int i = 0; i < resource_lib.GetSize(); i++) {
    cResource * res = resource_lib.GetResource(i);
    const double decay = 1.0 - res->GetOutflow();
    resource_count.Setup(i, res->GetName(), res->GetInitial(), 
                           res->GetInflow(), decay,
                           res->GetGeometry(), res->GetXDiffuse(),
                           res->GetXGravity(), res->GetYDiffuse(), 
                           res->GetYGravity(), res->GetInflowX1(), 
                           res->GetInflowX2(), res->GetInflowY1(), 
                           res->GetInflowY2(), res->GetOutflowX1(), 
                           res->GetOutflowX2(), res->GetOutflowY1(), 
                           res->GetOutflowY2() );
    stats.SetResourceName(i, res->GetName());
  }

  // Give stats information about the environment...
  const cTaskLib & task_lib = environment.GetTaskLib();
  for (int i = 0; i < task_lib.GetSize(); i++) {
    const cTaskEntry & cur_task = task_lib.GetTask(i);
    stats.SetTaskName(i, cur_task.GetDesc());
  }

  const cInstLib & inst_lib = environment.GetInstLib();
  for (int i = 0; i < inst_lib.GetSize(); i++) {
    stats.SetInstName(i, inst_lib.GetName(i));
  }

  // Load a clone if one is provided, otherwise setup start organism.
  if (cConfig::GetCloneFilename() == "") {
    Inject( cInstUtil::LoadGenome(cConfig::GetStartCreature(),
				  environment.GetInstLib()) );
  } else {
    ifstream fp(cConfig::GetCloneFilename()());
    LoadClone(fp);
  }

  // Load a saved population if one is provided.
  cString fname(cConfig::GetLoadPopFilename());
  if (fname != "") {
    fprintf(stderr,"Loding Population from %s\n", fname());

    // If last three chars of filename are ".gz" , gunzip it first
    if (fname.Find(".gz") == fname.GetSize() - 3) {
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
    } else {
      // load normally
      ifstream fp(fname);
      LoadPopulation(fp);
    }
  }

}


cPopulation::~cPopulation()
{
  for (int i = 0; i < cell_array.GetSize(); i++) KillOrganism(cell_array[i]);

  if ( lineage_control != NULL ) delete lineage_control;
  delete genebank;
  delete schedule;
}


// Activate the child, given information from the parent.
// Return true if parent lives through this process.

bool cPopulation::ActivateOffspring(cGenome & child_genome,
				    cOrganism & parent_organism)
{
  assert(&parent_organism != NULL);

  tArray<cOrganism *> child_array;
  birth_chamber.SubmitOffspring(child_genome, parent_organism, child_array);

  cPhenotype & parent_phenotype = parent_organism.GetPhenotype();

  if (child_array.GetSize() == 0) {
    // No children to be born.  Allow parent to think it happened anyway.
    parent_phenotype.DivideReset(parent_organism.GetGenome().GetSize());
    return true;
  }

  bool parent_alive = true;  // Will the parent live through this process?

  // First, setup the genotype of all of the offspring.
  cGenotype * parent_genotype = parent_organism.GetGenotype();
  const int parent_id = parent_organism.PopInterface().GetCellID();
  assert(parent_id >= 0 && parent_id < cell_array.GetSize());
  cPopulationCell & parent_cell = cell_array[ parent_id ];

  tArray<int> target_cells(child_array.GetSize());

  for (int i = 0; i < child_array.GetSize(); i++) {
    cGenotype * child_genotype = parent_genotype;

    // If the parent genotype is not correct for the child, adjust it.
    if (parent_phenotype.CopyTrue() == false ||
	parent_phenotype.DivideSex() == true) {
      child_genotype =
	genebank->AddGenotype(child_array[i]->GetGenome(), parent_genotype);
    }

    // And set the genotype now that we know it.
    child_array[i]->SetGenotype(child_genotype);
    parent_genotype->SetBreedStats(*child_genotype);

    // We want to make sure that the child's genotype is not delete from the
    // genebank before the child is placed.
    child_genotype->IncDeferAdjust();


    // Do lineage tracking for the new creature, if necessary.  Must occur
    // before old organism is removed.
    LineageSetupOrganism( child_array[i], parent_organism.GetLineage(),
			  parent_organism.GetLineageLabel(), parent_genotype );

    // If we are not on the last offspring, don't replace the parent!
    if (i < child_array.GetSize() - 1) {
      target_cells[i] = PositionChild( parent_cell, false ).GetID();
    } else {
      target_cells[i] = PositionChild( parent_cell, true ).GetID();
      // If we did, in fact, replace the parent, make a note of this.
      if (target_cells[i] == parent_cell.GetID()) parent_alive = false;      
    }

    child_array[i]->MutationRates().
      Copy(GetCell(target_cells[i]).MutationRates());
  }

  // Update the parent's and child's phenotype.  Previous work in this function
  // may require old phenotype information; don't unpdate sooner!
  parent_phenotype.DivideReset(parent_organism.GetGenome().GetSize());

  // Go back into a for-loop and continue to deal with the children.
  for (int i = 0; i < child_array.GetSize(); i++) {
    const int child_length = child_array[i]->GetGenome().GetSize();
    child_array[i]->GetPhenotype().SetupOffspring(parent_phenotype,child_length);
  }

  // If we're not about to kill the parent, do some extra work on it.
  if (parent_alive == true) {
    schedule->Adjust(parent_cell.GetID(), parent_phenotype.GetMerit());

    // In a local run, face the child toward the parent. 
    if (cConfig::GetBirthMethod() < NUM_LOCAL_POSITION_CHILD) {
      for (int i = 0; i < child_array.GetSize(); i++) {
	GetCell(target_cells[i]).Rotate(parent_cell);
      }
    }
  }

  // Do any statistics on the parent that just gave birth...
  parent_genotype->AddGestationTime( parent_phenotype.GetGestationTime() );
  parent_genotype->AddFitness(       parent_phenotype.GetFitness()       );
  parent_genotype->AddMerit(         parent_phenotype.GetMerit()         );
  parent_genotype->AddCopiedSize(    parent_phenotype.GetCopiedSize()    );
  parent_genotype->AddExecutedSize(  parent_phenotype.GetExecutedSize()  );

  // Place all of the offspring...
  for (int i = 0; i < child_array.GetSize(); i++) {
    ActivateOrganism(child_array[i], GetCell(target_cells[i]));
    cGenotype * child_genotype = child_array[i]->GetGenotype();
    child_genotype->DecDeferAdjust();
    genebank->AdjustGenotype(*child_genotype);
  }

  return parent_alive;
}


void cPopulation::ActivateOrganism(cOrganism * in_organism,
				   cPopulationCell & target_cell)
{
  assert(in_organism != NULL);
  assert(in_organism->GetGenome().GetSize() > 1);

  // If the organism does not have a genotype, give it one!
  if (in_organism->GetGenotype() == NULL) {
    cGenotype * new_genotype = genebank->AddGenotype(in_organism->GetGenome());
    in_organism->SetGenotype(new_genotype);
  }
  cGenotype * in_genotype = in_organism->GetGenotype();

  // Save the old genotype from this cell...
  cGenotype * old_genotype = NULL;
  if (target_cell.IsOccupied()) {
    old_genotype = target_cell.GetOrganism()->GetGenotype();

    // Sometimes a new organism will kill off the last member of its genotype
    // in the population.  Normally this would remove the genotype, so we 
    // want to defer adjusting that genotype until the new one is placed.
    old_genotype->IncDeferAdjust();
  }

  // Update the contents of the target cell.
  KillOrganism(target_cell);
  target_cell.InsertOrganism(*in_organism);

  // Setup the inputs in the target cell.
  environment.SetupInputs(target_cell.input_array);

  // Update the genebank...
  in_genotype->AddOrganism();

  if (old_genotype != NULL) {
    old_genotype->DecDeferAdjust();
    genebank->AdjustGenotype(*old_genotype);
  }
  genebank->AdjustGenotype(*in_genotype);

  // Initialize the time-slice for this new organism.
  schedule->Adjust(target_cell.GetID(),in_organism->GetPhenotype().GetMerit());

  // Special handling for certain birth methods.
  if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_ELDEST) {
    reaper_queue.Push(&target_cell);
  }

  num_organisms++;

  // Statistics...
  stats.RecordBirth(target_cell.GetID(), in_genotype->GetID(),
		      in_organism->GetPhenotype().ParentTrue());
}


void cPopulation::KillOrganism(cPopulationCell & in_cell)
{
  // do we actually have something to kill?
  if (in_cell.IsOccupied() == false) {
    return;
  }

  // Statistics...
  cOrganism * organism = in_cell.GetOrganism();
  cGenotype * genotype = organism->GetGenotype();
  stats.RecordDeath(in_cell.GetID(), genotype->GetID(),
		    organism->GetPhenotype().GetAge());


  // Do the lineage handling
  if (lineage_control != NULL) {
    lineage_control->RemoveCreature( organism );
  }

  // Do statistics
  num_organisms--;

  if (organism->GetPhenotype().IsParasite() == true) {
    genotype->AddParasite();
  }
  genotype->RemoveOrganism();

  // And clear it!
  in_cell.RemoveOrganism();
  delete organism;

  // Alert the scheduler that this cell has a 0 merit.
  schedule->Adjust( in_cell.GetID(), cMerit(0) );

  // Update the genebank (note: genotype adjustment may be defered)
  genebank->AdjustGenotype(*genotype);
}


/**
 * This function is responsible for adding an organism to a given lineage,
 * and setting the organism's lineage label and the lineage pointer.
 **/

void cPopulation::LineageSetupOrganism(cOrganism * organism, cLineage * lin,
				     int lin_label, cGenotype *parent_genotype)
{
  // If we have some kind of lineage control, adjust the default values
  // passed in.
  if ( lineage_control ){
    lin = lineage_control->
      AddCreature(organism->GetGenotype(), parent_genotype, lin, lin_label);
    lin_label = lin->GetID();
  }

  organism->SetLineageLabel( lin_label );
  organism->SetLineage( lin );
}


/**
 * This function directs which position function should be used.  It
 * could have also been done with a function pointer, but the dividing
 * of an organism takes enough time that this will be a negligible addition,
 * and it gives a centralized function to work with.  The parent_ok flag asks
 * if it is okay to replace the parent.
 **/

cPopulationCell & cPopulation::PositionChild(cPopulationCell & parent_cell,
					     bool parent_ok)
{
  assert(parent_cell.IsOccupied());
  
  // Try out global birth methods first...

  if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_RANDOM) {
    int out_pos = g_random.GetUInt(cell_array.GetSize());
    while (parent_ok == false && out_pos == parent_cell.GetID()) {
      out_pos = g_random.GetUInt(cell_array.GetSize());
    }
    return GetCell(out_pos);
  }
  else if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_ELDEST) {
    cPopulationCell * out_cell = reaper_queue.PopRear();
    if (parent_ok == false && out_cell->GetID() == parent_cell.GetID()) {
      out_cell = reaper_queue.PopRear();
      reaper_queue.PushRear(&parent_cell);
    }
    return *out_cell;
  }

  // Construct a list of equally viable locations to place the child...
  tList<cPopulationCell> found_list;

  // First, check if there is an empty organism to work with (always preferred)
  tList<cPopulationCell> & conn_list = parent_cell.ConnectionList();
  FindEmptyCell(conn_list, found_list);

  // If we have not found an empty organism, we must use the specified function
  // to determine how to choose among the filled organisms.
  if (found_list.GetSize() == 0) {
    switch(cConfig::GetBirthMethod()) {
    case POSITION_CHILD_AGE:
      PositionAge(parent_cell, found_list, parent_ok);
      break;
    case POSITION_CHILD_MERIT:
      PositionMerit(parent_cell, found_list, parent_ok);
      break;
    case POSITION_CHILD_RANDOM:
      found_list.Append(conn_list);
      if (parent_ok == true) found_list.Push(&parent_cell);
      break;
    case POSITION_CHILD_EMPTY:
      // Nothing is in list if no empty cells are found...
      break;
    }
  }

  // If there are no possibilities, return NULL.
  if (found_list.GetSize() == 0) return parent_cell;

  // Choose the organism randomly from those in the list, and return it.
  int choice = g_random.GetUInt(found_list.GetSize());
  return *( found_list.GetPos(choice) );
}


int cPopulation::ScheduleOrganism()
{
  return schedule->GetNextID();
}

void cPopulation::ProcessStep(double step_size, int cell_id)
{
  assert(step_size > 0.0);
  assert(cell_id >= 0 && cell_id < cell_array.GetSize());

  cPopulationCell & cell = GetCell(cell_id);
  assert(cell.IsOccupied()); // Unoccupied cell getting processor time!

//    static ofstream debug_fp("debug.trace");
//    debug_fp << stats.GetUpdate() << " "
//  	   << cell.GetOrganism()->GetCellID() << " "
//  	   << cell.GetOrganism()->GetGenotype()->GetID() << " "
//  	   << g_random.GetDouble() << " "
//      	   << cell.GetOrganism()->GetHardware().GetMemory().AsString() << " "
//  	   << endl;

  cell.GetOrganism()->GetHardware().SingleProcess();
  stats.IncExecuted();
  resource_count.Update(step_size);
}


void cPopulation::ProcessStep(double step_size)
{
  ProcessStep( step_size, ScheduleOrganism() );
}


void cPopulation::UpdateOrganismStats()
{
  // Loop through all the cells getting stats and doing calculations
  // which must be done on a creature by creature basis.

  // Clear out organism sums...
  stats.SumFitness().Clear();
  stats.SumGestation().Clear();
  stats.SumMerit().Clear();
  stats.SumCreatureAge().Clear();
  stats.SumGeneration().Clear();
  stats.SumNeutralMetric().Clear();
  stats.SumLineageLabel().Clear();
  stats.SumCopyMutRate().Clear();
  stats.SumDivMutRate().Clear();
  stats.SumCopySize().Clear();
  stats.SumExeSize().Clear();
  stats.SumMemSize().Clear();


  stats.ZeroTasks();

#ifdef INSTRUCTION_COUNT
  stats.ZeroInst();
#endif

  // Counts...
  int num_breed_true = 0;
  int num_parasites = 0;
  int num_no_birth = 0;

  // Maximums...
  cMerit max_merit(0);
  double max_fitness = 0;


  for (int i = 0; i < cell_array.GetSize(); i++) {
    // Only look at cells with organisms in them.
    if (cell_array[i].IsOccupied() == false) continue;

    cOrganism * organism = cell_array[i].GetOrganism();
    const cPhenotype & phenotype = organism->GetPhenotype();
    const cMerit cur_merit = phenotype.GetMerit();
    const double cur_fitness = phenotype.GetFitness();

    stats.SumFitness().Add(cur_fitness);
    stats.SumMerit().Add(cur_merit.GetDouble());
    stats.SumGestation().Add(phenotype.GetGestationTime());
    stats.SumCreatureAge().Add(phenotype.GetAge());
    stats.SumGeneration().Add(phenotype.GetGeneration());
    stats.SumNeutralMetric().Add(phenotype.GetNeutralMetric());
    stats.SumLineageLabel().Add(organism->GetLineageLabel());
    stats.SumCopyMutRate().Add(organism->MutationRates().GetCopyMutProb());
    stats.SumLogCopyMutRate().Add(log(organism->MutationRates().GetCopyMutProb()));
    stats.SumDivMutRate().Add(organism->MutationRates().GetDivMutProb() / organism->GetPhenotype().GetDivType());
    stats.SumLogDivMutRate().Add(log(organism->MutationRates().GetDivMutProb() /organism->GetPhenotype().GetDivType()));
    stats.SumCopySize().Add(phenotype.GetCopiedSize());
    stats.SumExeSize().Add(phenotype.GetExecutedSize());

#ifdef INSTRUCTION_COUNT
//    for (int j=0; j < environment.GetInstLib().GetSize(); j++) {
    for (int j=0; j < cConfig::GetNumInstructions(); j++) {
	stats.SumExeInst()[j].Add(organism->GetPhenotype().GetLastInstCount()[j]);
    }
#endif

    if (cur_merit > max_merit) max_merit = cur_merit;
    if (cur_fitness > max_fitness) max_fitness = cur_fitness;

    // Test what tasks this creatures has completed.
    for (int j=0; j < phenotype.GetEnvironment().GetTaskLib().GetSize(); j++) {
      if (phenotype.GetCurTaskCount()[j] > 0)  stats.AddCurTask(j);
      if (phenotype.GetLastTaskCount()[j] > 0) stats.AddLastTask(j);
    }

    // Increment the counts for all qualities the organism has...
    if (phenotype.ParentTrue()) num_breed_true++;
    if (phenotype.IsParasite()) num_parasites++;
    if( phenotype.GetNumDivides() == 0 ) num_no_birth++;

    // Hardware specific collections...
    if (organism->GetHardware().GetType() == HARDWARE_TYPE_VIRTUAL_CPU) {
      cHardwareBase & hardware = organism->GetHardware();
      stats.SumMemSize().Add(hardware.GetMemory().GetSize());
    }

    // Increment the age of this organism.
    organism->GetPhenotype().IncAge();
  }

  stats.SetBreedTrueCreatures(num_breed_true);
  stats.SetNumNoBirthCreatures(num_no_birth);
  stats.SetNumParasites(num_parasites);

  stats.SetMaxMerit(max_merit.GetDouble());
  stats.SetMaxFitness(max_fitness);

  stats.SetResources(resource_count.GetResources());
  stats.SetSpatialRes(resource_count.GetSpatialRes());
  stats.SetResourcesGeometry(resource_count.GetResourcesGeometry());
}


void cPopulation::UpdateGenotypeStats()
{
  // Loop through all genotypes, finding stats and doing calcuations.

  // Clear out genotype sums...
  stats.SumGenotypeAge().Clear();
  stats.SumAbundance().Clear();
  stats.SumGenotypeDepth().Clear();
  stats.SumSize().Clear();
  stats.SumThresholdAge().Clear();

  double entropy = 0.0;

  cGenotype * cur_genotype = genebank->GetBestGenotype();
  for (int i = 0; i < genebank->GetSize(); i++) {
    const int abundance = cur_genotype->GetNumOrganisms();

    // If we're at a dead genotype, we've hit the end of the list!
    if (abundance == 0) break;

    // Update stats...
    const int age = stats.GetUpdate() - cur_genotype->GetUpdateBorn();
    stats.SumGenotypeAge().Add(age, abundance);
    stats.SumAbundance().Add(abundance);
    stats.SumGenotypeDepth().Add(cur_genotype->GetDepth(), abundance);
    stats.SumSize().Add(cur_genotype->GetLength(), abundance);

    // Calculate this genotype's contribution to entropy
    const double p = ((double) abundance) / (double) num_organisms;
    const double partial_ent = -(p * Log(p));
    entropy += partial_ent;

    // Do any special calculations for threshold genotypes.
    if (cur_genotype->GetThreshold()) {
      stats.SumThresholdAge().Add(age, abundance);
    }

    // ...and advance to the next genotype...
    cur_genotype = cur_genotype->GetNext();
  }

  stats.SetEntropy(entropy);
}


void cPopulation::UpdateSpeciesStats()
{
  double species_entropy = 0.0;

  stats.SumSpeciesAge().Clear();

  // Loop through all species that need to be reset prior to calculations.
  cSpecies * cur_species = genebank->GetFirstSpecies();
  for (int i = 0; i < genebank->GetNumSpecies(); i++) {
    cur_species->ResetStats();
    cur_species = cur_species->GetNext();
  }

  // Collect info from genotypes and send it to their species.
  cGenotype * genotype = genebank->GetBestGenotype();
  for (int i = 0; i < genebank->GetSize(); i++) {
    if (genotype->GetSpecies() != NULL) {
      genotype->GetSpecies()->AddOrganisms(genotype->GetNumOrganisms());
    }
    genotype = genotype->GetNext();
  }

  // Loop through all of the species in the soup, taking info on them.
  cur_species = genebank->GetFirstSpecies();
  for (int i = 0; i < genebank->GetNumSpecies(); i++) {
    const int abundance = cur_species->GetNumOrganisms();
    // const int num_genotypes = cur_species->GetNumGenotypes();

    // Basic statistical collection...
    const int species_age = stats.GetUpdate() - cur_species->GetUpdateBorn();
    stats.SumSpeciesAge().Add(species_age, abundance);

    // Caculate entropy on the species level...
    if (abundance > 0) {
      double p = ((double) abundance) / (double) num_organisms;
      double partial_ent = -(p * Log(p));
      species_entropy += partial_ent;
    }

    // ...and advance to the next species...
    cur_species = cur_species->GetNext();
  }

  stats.SetSpeciesEntropy(species_entropy);
}

void cPopulation::UpdateDominantStats()
{
  cGenotype * dom_genotype = genebank->GetBestGenotype();
  if (dom_genotype == NULL) return;

  stats.SetDomGenotype(dom_genotype);
  stats.SetDomMerit(dom_genotype->GetMerit());
  stats.SetDomGestation(dom_genotype->GetGestationTime());
  stats.SetDomReproRate(dom_genotype->GetReproRate());
  stats.SetDomFitness(dom_genotype->GetFitness());
  stats.SetDomCopiedSize(dom_genotype->GetCopiedSize());
  stats.SetDomExeSize(dom_genotype->GetExecutedSize());

  stats.SetDomSize(dom_genotype->GetLength());
  stats.SetDomID(dom_genotype->GetID());
  stats.SetDomName(dom_genotype->GetName());
  stats.SetDomBirths(dom_genotype->GetThisBirths());
  stats.SetDomBreedTrue(dom_genotype->GetThisBreedTrue());
  stats.SetDomBreedIn(dom_genotype->GetThisBreedIn());
  stats.SetDomBreedOut(dom_genotype->GetThisBreedOut());
  stats.SetDomAbundance(dom_genotype->GetNumOrganisms());
  stats.SetDomGeneDepth(dom_genotype->GetDepth());
  stats.SetDomSequence(dom_genotype->GetGenome().AsString());
}

void cPopulation::CalcUpdateStats()
{
  // Reset the Genebank to prepare it for stat collection.
  genebank->UpdateReset();

  UpdateOrganismStats();
  UpdateGenotypeStats();
  UpdateSpeciesStats();
  UpdateDominantStats();

  // Do any final calculations...
  stats.SetNumCreatures(GetNumOrganisms());
  stats.SetNumGenotypes(genebank->GetSize());
  stats.SetNumThreshSpecies(genebank->GetNumSpecies());

  // Have stats calculate anything it now can...
  stats.CalcEnergy();
  stats.CalcFidelity();
}


bool cPopulation::SaveClone(ofstream & fp)
{
  if (fp.good() == false) return false;

  // Save the current update
  fp << stats.GetUpdate() << " ";

  // Save the genebank info.
  genebank->SaveClone(fp);

  // Save the genotypes manually.
  fp << genebank->GetSize() << " ";

  cGenotype * cur_genotype = genebank->GetBestGenotype();
  for (int i = 0; i < genebank->GetSize(); i++) {
    cur_genotype->SaveClone(fp);

    // Advance...
    cur_genotype = cur_genotype->GetNext();
  }

  // Save the organim layout...
  fp << cell_array.GetSize() << " ";
  for (int i = 0; i < cell_array.GetSize(); i++) {
    if (cell_array[i].IsOccupied() == true) {
      fp <<  cell_array[i].GetOrganism()->GetGenotype()->GetID() << " ";
    }
    else fp << "-1 ";
  }

  return true;
}


bool cPopulation::LoadClone(ifstream & fp)
{
  if (fp.good() == false) return false;

  // Pick up the update where it was left off.
  int cur_update;
  fp >> cur_update;

  stats.SetCurrentUpdate(cur_update);

  // Clear out the population
  for (int i = 0; i < cell_array.GetSize(); i++) KillOrganism(cell_array[i]);

  // Load the genebank info.
  genebank->LoadClone(fp);

  // Load up the genotypes.
  int num_genotypes = 0;
  fp >> num_genotypes;

  cGenotype * genotype_array = new cGenotype[num_genotypes];
  for (int i = 0; i < num_genotypes; i++) genotype_array[i].LoadClone(fp);

  // Now load them into the organims.  @CAO make sure cell_array.GetSize() is right!
  int in_num_cells;
  int genotype_id;
  fp >> in_num_cells;
  if (cell_array.GetSize() != in_num_cells) return false;

  for (int i = 0; i < cell_array.GetSize(); i++) {
    fp >> genotype_id;
    if (genotype_id == -1) continue;
    int genotype_index = -1;
    for (int j = 0; j < num_genotypes; j++) {
      if (genotype_array[j].GetID() == genotype_id) {
	genotype_index = j;
	break;
      }
    }

    assert(genotype_index != -1);
    InjectGenome(i, genotype_array[genotype_index].GetGenome());
  }

  sync_events = true;

  return true;
}

// This class is needed for the next function
class cTmpGenotype {
public:
  int id_num;
  int parent_id;
  int num_cpus;
  int total_cpus;
  double merit;
  int update_born;
  int update_dead;

  cGenotype *genotype;

  bool operator<( const cTmpGenotype rhs ) const {
    return id_num < rhs.id_num; }
};	


bool cPopulation::LoadDumpFile(cString filename, int update)
{
  // set the update if requested
  if ( update >= 0 )
    stats.SetCurrentUpdate(update);

  // Clear out the population
  for (int i = 0; i < cell_array.GetSize(); i++) KillOrganism(cell_array[i]);

  cout << "Loading: " << filename << endl;

  cInitFile input_file(filename);
  if (!input_file.IsOpen()) {
    cerr << "Error: Cannot load file: \"" << filename << "\"." << endl;
    exit(1);
  }
  input_file.Load();
  input_file.Compress();
  input_file.Close();

  // First, we read in all the genotypes and store them in a list

  vector<cTmpGenotype> genotype_vect;

  while (input_file.GetNumLines() > 0) {
    cString cur_line = input_file.RemoveLine();

    // Setup the genotype for this line...
    cTmpGenotype tmp;
    tmp.id_num      = cur_line.PopWord().AsInt();
    tmp.parent_id   = cur_line.PopWord().AsInt();
    /*parent_dist =*/          cur_line.PopWord().AsInt();
    tmp.num_cpus    = cur_line.PopWord().AsInt();
    tmp.total_cpus  = cur_line.PopWord().AsInt();
    /*length      =*/          cur_line.PopWord().AsInt();
    tmp.merit 	    = cur_line.PopWord().AsDouble();
    /*gest_time   =*/ cur_line.PopWord().AsInt();
    /*fitness     =*/ cur_line.PopWord().AsDouble();
    tmp.update_born = cur_line.PopWord().AsInt();
    tmp.update_dead = cur_line.PopWord().AsInt();
    /*depth       =*/ cur_line.PopWord().AsInt();
    cString name = cStringUtil::Stringf("org-%d", tmp.id_num);
    cGenome genome( cur_line.PopWord() );

    // we don't allow birth or death times larger than the current update
    if ( stats.GetUpdate() > tmp.update_born )
      tmp.update_born = stats.GetUpdate();
    if ( stats.GetUpdate() > tmp.update_dead )
      tmp.update_dead = stats.GetUpdate();

    tmp.genotype =
      new cGenotype(tmp.update_born, tmp.id_num);
    tmp.genotype->SetGenome( genome );
    tmp.genotype->SetName( name );

    genotype_vect.push_back( tmp );
  }

  // now, we sort them in ascending order according to their id_num
  sort( genotype_vect.begin(), genotype_vect.end() );
  // set the parents correctly

  vector<cTmpGenotype>::const_iterator it = genotype_vect.begin();
  for ( ; it != genotype_vect.end(); it++ ){
    vector<cTmpGenotype>::const_iterator it2 = it;
    cGenotype *parent = 0;
    // search backwards till we find the parent
    if ( it2 != genotype_vect.begin() )
      do{
	it2--;
	if ( (*it).parent_id == (*it2).id_num ){
	  parent = (*it2).genotype;
	  break;
	}	
      }
      while ( it2 != genotype_vect.begin() );
    (*it).genotype->SetParent( parent );
  }

  int cur_update = stats.GetUpdate(); 
  int current_cell = 0;
  bool soup_full = false;
  it = genotype_vect.begin();
  for ( ; it != genotype_vect.end(); it++ ){
    genebank->AddGenotype( (*it).genotype );
    if ( (*it).num_cpus == 0 ){ // historic organism
      // remove immediately, so that it gets transferred into the
      // historic database. We change the update temporarily to the
      // true death time of this organism, so that all stats are correct.
      stats.SetCurrentUpdate( (*it).update_dead );
      genebank->RemoveGenotype( *(*it).genotype );
      stats.SetCurrentUpdate( cur_update );
    }
    else{ // otherwise, we insert as many organisms as we need
      for ( int i=0; i<(*it).num_cpus; i++ ){
	if ( current_cell >= cell_array.GetSize() ){
	  soup_full = true;
	  break;
	}	  
	InjectGenotype( current_cell, (*it).genotype );
	cPhenotype & phenotype = GetCell(current_cell).GetOrganism()->GetPhenotype();
	if ( (*it).merit > 0) phenotype.SetMerit( cMerit((*it).merit) );
	schedule->Adjust(current_cell, phenotype.GetMerit());

	int lineage_label = 0;
	LineageSetupOrganism(GetCell(current_cell).GetOrganism(),
			     0, lineage_label,
			     (*it).genotype->GetParentGenotype());
	current_cell += 1;
      }
    }
    cout << (*it).id_num << " "
	 << (*it).parent_id << " "
	 << (*it).genotype->GetParentID() << " "
	 << (*it).genotype->GetNumOffspringGenotypes() << " "
	 << (*it).num_cpus << " "
	 << (*it).genotype->GetNumOrganisms() << endl;
    if (soup_full){
      cout << "cPopulation::LoadDumpFile: You are trying to load more organisms than there is space!" << endl;
      cout << "cPopulation::LoadDumpFile: Remaining organisms are ignored." << endl;
      break;
    }
  }
  sync_events = true;

  return true;
}

//// Save And Load Populations ////
bool cPopulation::SavePopulation(ofstream & fp)
{
  if (fp.good() == false) return false;

  // Save the update
  fp << stats.GetUpdate() << endl;

  // looping through all cells saving state.
  for (int i = 0; i < cell_array.GetSize(); i++)  cell_array[i].SaveState(fp);

  return true;
}


bool cPopulation::LoadPopulation(ifstream & fp)
{
  if(fp.good() == false) return false;

  // Load Update...
  int cur_update;
  fp >> cur_update;
  stats.SetCurrentUpdate(cur_update);

  // Clear out the current population
  for (int i = 0; i < cell_array.GetSize(); i++) KillOrganism( cell_array[i] );

  // looping through all organims
  for (int i = 0; i < cell_array.GetSize(); i++) cell_array[i].LoadState(fp);

  sync_events = true;

  return true;
}


bool cPopulation::DumpMemorySummary(ofstream & fp)
{
  if (fp.good() == false) return false;

  // Dump the memory...

  for (int i = 0; i < cell_array.GetSize(); i++) {
    fp << i << " ";
    if (cell_array[i].IsOccupied() == false) {
      fp << "EMPTY" << endl;
    }
    else {
      cGenome & mem = cell_array[i].GetOrganism()->GetHardware().GetMemory();
      fp << mem.GetSize() << " "
	 << mem.AsString() << endl;
    }
  }
}


bool cPopulation::OK()
{
  // First check all sub-objects...
  if (!genebank->OK() || !schedule->OK()) return false;

  // Next check organisms...
  for (int i = 0; i < cell_array.GetSize(); i++) {
    if (cell_array[i].OK() == false) return false;
    assert(cell_array[i].GetID() == i);
  }

  // And stats...
  assert(world_x * world_y == cell_array.GetSize());

  return true;
}


/**
 * This function loads a genome from a given file, and initializes
 * a cpu with it.
 *
 * @param filename The name of the file to load.
 * @param in_cpu The grid-position into which the genome should be loaded.
 * @param merit An initial merit value.
 * @param lineage_label A value that allows to track the daughters of
 * this organism.
 **/

void cPopulation::Inject(const cGenome & genome, int cell_id, double merit,
			 int lineage_label, double neutral )
{
  // If an invalid cell was given, choose a new ID for it.
  if (cell_id < 0) {
    switch (cConfig::GetBirthMethod()) {
    case POSITION_CHILD_FULL_SOUP_ELDEST:
      cell_id = reaper_queue.PopRear()->GetID();
    default:
      cell_id = 0;
    }
  }

  InjectGenome( cell_id, genome );
  cPhenotype & phenotype = GetCell(cell_id).GetOrganism()->GetPhenotype();
  phenotype.SetNeutralMetric(neutral);

  if (merit > 0) phenotype.SetMerit( cMerit(merit) );
  schedule->Adjust(cell_id, phenotype.GetMerit());

  LineageSetupOrganism(GetCell(cell_id).GetOrganism(), 0, lineage_label);
}


cPopulationCell & cPopulation::GetCell(int in_num)
{
  return cell_array[in_num];
}


void cPopulation::UpdateResources(const tArray<double> & res_change)
{
  resource_count.Modify(res_change);
}

void cPopulation::UpdateResource(int id, double change)
{
  resource_count.Modify(id, change);
}

void cPopulation::UpdateCellResources(const tArray<double> & res_change, 
                                      const int cell_id)
{
  resource_count.ModifyCell(res_change, cell_id);
}

void cPopulation::SetResource(int id, double new_level)
{
  resource_count.Set(id, new_level);
}

void cPopulation::BuildTimeSlicer()
{
  switch (cConfig::GetSlicingMethod()) {
  case SLICE_CONSTANT:
    cout << "...Building Constant Time Slicer..." << endl;
    schedule = new cConstSchedule(cell_array.GetSize());
    break;
  case SLICE_PROB_MERIT:
    cout << "...Building Probablistic Time Slicer..." << endl;
    schedule = new cProbSchedule(cell_array.GetSize());
    break;
  case SLICE_INTEGRATED_MERIT:
    cout << "...Building Integrated Time Slicer..." << endl;
    schedule = new cIntegratedSchedule(cell_array.GetSize());
    break;
  default:
    cout << "...Requested Time Slicer not found, defaulting to Integrated..."
	 << endl;
    schedule = new cIntegratedSchedule(cell_array.GetSize());
    break;
  }

}


void cPopulation::PositionAge(cPopulationCell & parent_cell,
			      tList<cPopulationCell> & found_list,
			      bool parent_ok)
{
  // Start with the parent organism as the replacement, and see if we can find
  // anything equivilent or better.

  found_list.Push(&parent_cell);
  int max_age = parent_cell.GetOrganism()->GetPhenotype().GetAge();
  if (parent_ok == false) max_age = -1;

  // Now look at all of the neighbors.
  tListIterator<cPopulationCell> conn_it( parent_cell.ConnectionList() );

  cPopulationCell * test_cell;
  while ( (test_cell = conn_it.Next()) != NULL) {
    const int cur_age = test_cell->GetOrganism()->GetPhenotype().GetAge();
    if (cur_age > max_age) {
      max_age = cur_age;
      found_list.Clear();
      found_list.Push(test_cell);
    }
    else if (cur_age == max_age) {
      found_list.Push(test_cell);
    }
  }
}

void cPopulation::PositionMerit(cPopulationCell & parent_cell,
				tList<cPopulationCell> & found_list,
				bool parent_ok)
{
  // Start with the parent organism as the replacement, and see if we can find
  // anything equivilent or better.

  found_list.Push(&parent_cell);
  double max_ratio = parent_cell.GetOrganism()->CalcMeritRatio();
  if (parent_ok == false) max_ratio = -1;

  // Now look at all of the neighbors.
  tListIterator<cPopulationCell> conn_it( parent_cell.ConnectionList() );

  cPopulationCell * test_cell;
  while ( (test_cell = conn_it.Next()) != NULL) {
    const double cur_ratio = test_cell->GetOrganism()->CalcMeritRatio();
    if (cur_ratio > max_ratio) {
      max_ratio = cur_ratio;
      found_list.Clear();
      found_list.Push(test_cell);
    }
    else if (cur_ratio == max_ratio) {
      found_list.Push(test_cell);
    }
  }
}

void cPopulation::FindEmptyCell(tList<cPopulationCell> & cell_list,
				tList<cPopulationCell> & found_list)
{
  tListIterator<cPopulationCell> cell_it(cell_list);
  cPopulationCell * test_cell;

  while ( (test_cell = cell_it.Next()) != NULL) {
    // If this cell is empty, add it to the list...
    if (test_cell->IsOccupied() == false) found_list.Push(test_cell);
  }
}



void cPopulation::InjectGenotype(int cell_id, cGenotype *new_genotype)
{
  assert(cell_id >= 0 && cell_id < cell_array.GetSize());

  cOrganism * new_organism = new cOrganism(new_genotype->GetGenome(),
					   default_interface,
					   environment);

  // Set the genotype...
  new_organism->SetGenotype(new_genotype);

  // Setup the phenotype...
  cPhenotype & phenotype = new_organism->GetPhenotype();
  phenotype.SetupInject(new_genotype->GetLength());
  phenotype.SetMerit( cMerit(new_genotype->GetTestMerit()) );

  // @CAO are these really needed?
  phenotype.SetLinesCopied( new_genotype->GetTestCopiedSize() );
  phenotype.SetLinesExecuted( new_genotype->GetTestExecutedSize() );
  phenotype.SetGestationTime( new_genotype->GetTestGestationTime() );

  // Prep the cell..
  if (cConfig::GetBirthMethod() == POSITION_CHILD_FULL_SOUP_ELDEST &&
      cell_array[cell_id].IsOccupied() == true) {
    // Have to manually take this cell out of the reaper Queue.
    reaper_queue.Remove( &(cell_array[cell_id]) );
  }

  // Setup the child's mutation rates.  Since this organism is being injected
  // and has no parent, we should always take the rate from the environment.
  new_organism->MutationRates().Copy(cell_array[cell_id].MutationRates());


  // Activate the organism in the population...
  ActivateOrganism(new_organism, cell_array[cell_id]);
}


void cPopulation::InjectGenome(int cell_id, const cGenome & genome)
{
  // Setup the genotype...
  cGenotype * new_genotype = genebank->AddGenotype(genome);

  // The rest is done by InjectGenotype();
  InjectGenotype( cell_id, new_genotype );
}


void cPopulation::SerialTransfer(int transfer_size, bool ignore_deads)
{
  assert(transfer_size > 0);

  // If we are ignoring all dead organisms, remove them from the population.
  if (ignore_deads == true) {
    for (int i = 0; i < GetSize(); i++) {
      cPopulationCell & cell = cell_array[i];
      if (cell.IsOccupied() && cell.GetOrganism()->GetTestFitness() == 0.0) {
	KillOrganism(cell);
      }
    }
  }

  // If removing the dead was enough, stop here.
  if (num_organisms <= transfer_size) return;

  // Collect a vector of the occupied cells...
  vector<int> transfer_pool;
  transfer_pool.reserve(num_organisms);
  for (int i = 0; i < GetSize(); i++) {
    if (cell_array[i].IsOccupied()) transfer_pool.push_back(i);
  }

  // Remove the proper number of cells.
  const int removal_size = num_organisms - transfer_size;
  for (int i = 0; i < removal_size; i++) {
    int j = (int) g_random.GetUInt(transfer_pool.size());
    KillOrganism(cell_array[transfer_pool[j]]);
    transfer_pool[j] = transfer_pool.back();
    transfer_pool.pop_back();
  }
}
