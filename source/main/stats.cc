//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "config.hh"
#include "stats.hh"
#include "genotype.hh"
#include "inst_lib.hh"
#include "../cpu/environment.hh"

////////////
//  cStats
////////////

void cStats::Setup()
{
  int i;

  g_memory.Add(C_STATISTICS);

  current_update = -1;
  ave_generation = 0;
  avida_time = 0;

  // Open Status files.  @TCC -- Move these to Events!!!!

  if (cConfig::GetGenotypeStatusInterval())
    fp_genotype_status.open("genotype.status");
  else fp_genotype_status.close();

  if (cConfig::GetDiversityStatusInterval())
    fp_diversity_status.open("diversity.status");
  else fp_diversity_status.close();


  // Open log files.

  if (cConfig::GetLogBreedCount())
    fp_breed_log.open("breed_count.log");
  else fp_breed_log.close();

  if (cConfig::GetLogCreatures())
    fp_creature_log.open("creature.log");
  else fp_creature_log.close();

  if (cConfig::GetLogPhylogeny())
    fp_phylogeny_log.open("phylogeny.log");
  else fp_phylogeny_log.close();

#ifdef GENEOLOGY
  if (cConfig::GetLogGeneology())
    fp_geneology_log.open("geneology.log");
  else fp_geneology_log.close();
#endif

  if (cConfig::GetLogGenotypes())
    fp_genotype_log.open("genotype.log");
  else fp_genotype_log.close();

  if (cConfig::GetLogThreshold())
    fp_threshold_log.open("threshold.log");
  else fp_threshold_log.close();

  if (cConfig::GetLogSpecies())
    fp_species_log.open("species.log");
  else fp_species_log.close();

  if (cConfig::GetLogLandscape())
    fp_landscape_log.open("landscape.log");
  else fp_landscape_log.close();

  // Update information

  best_merit     = 0;
  best_fitness   = 0;
  best_gestation = 0.0;
  max_fitness    = 0.0;

  entropy        = 0.0;
  energy         = 0.0;

  num_deaths     = 0;
  num_births     = 0;
  num_executed   = 0;
  num_creatures  = 0;
  num_genotypes  = 0;
  num_threshold  = 0;
  num_species    = 0;
  num_thresh_species = 0;
  num_breed_true = 0;
  num_breed_true_creatures = 0;
  num_no_birth_creatures = 0;
  num_parasites = 0;

  num_inst_executed   = 0;

  num_genotypes_last = 1;

#ifdef TEST_SEARCH_SIZE
  num_search_approx_size = 0;
#endif

  tot_creatures = 0;
  tot_genotypes = 0;
  tot_threshold = 0;
  tot_species   = 0;
  tot_thresh_species = 0;
  tot_executed  = 0;

  task_count = g_memory.GetInts(cConfig::GetNumTasks());
  task_exe_count = g_memory.GetInts(cConfig::GetNumTasks());
  for (i = 0; i < cConfig::GetNumTasks(); i++) {
    task_count[i] = 0;
    task_exe_count[i] = 0;
  }
}


void cStats::ZeroTasks(){
  for( int i=0; i<cConfig::GetNumTasks(); ++i ){
    task_count[i] = 0;
  }
}


void cStats::Shutdown()
{
  g_memory.Remove(C_STATISTICS);
  g_memory.Free(task_count, cConfig::GetNumTasks());

  cout<<"Closing stats object..."<<endl;

  // Close all the statistics files.

  data_file_manager.FlushAll();

  fp_genotype_status.close();
  fp_diversity_status.close();

  fp_breed_log.close();
  fp_creature_log.close();
  fp_genotype_log.close();
  fp_threshold_log.close();
  fp_species_log.close();
  fp_landscape_log.close();
}

void cStats::SetBestGenotype(cGenotype * best_genotype)
{
  dom_genotype = best_genotype;
  best_merit = best_genotype->GetApproxMerit();
  best_gestation = best_genotype->GetApproxGestationTime();
  best_repro_rate = best_genotype->GetReproRate();
  best_fitness = best_genotype->GetApproxFitness();
  best_copied_size = best_genotype->GetApproxCopiedSize();
  best_exe_size = best_genotype->GetApproxExecutedSize();

  best_size = best_genotype->GetLength();
  best_genotype_id = best_genotype->GetID();
  best_name = best_genotype->GetName();
  best_births = best_genotype->GetThisBirths();
  best_breed_true = best_genotype->GetThisBreedTrue();
  best_breed_in = best_genotype->GetThisBreedIn();
  best_abundance = best_genotype->GetNumCPUs();
  best_gene_depth = best_genotype->GetDepth();
}

void cStats::ProcessUpdate(int new_update)
{
  static int last_update = 0;
  (void)new_update;

  // Increment the "avida_time"
  if( sum_merit.Count() > 0  &&  sum_merit.Average() > 0 ){
    double delta = ((double)(current_update-last_update)) / sum_merit.Average();
    avida_time += delta;
    
    // calculate the true replication rate in this update
    rave_true_replication_rate.Add( num_births/
	  (delta * cConfig::GetAveTimeslice() * num_creatures) );
  }
  last_update = current_update;

  // Average generation
  ave_generation = sum_generation.Average();

  int i;

  
  
  // @TCC -- Output of data files handled by automatically adding to event list

  // Zero-out any variables which need to be cleared at end of update.

  num_births = 0;
  num_deaths = 0;
  num_breed_true = 0;

  tot_executed += num_executed;
  num_executed = 0;

  for (i = 0; i < cConfig::GetNumTasks(); i++) {
    task_count[i] = 0;
    task_exe_count[i] = 0;
  }

#ifdef TEST_SEARCH_SIZE
  num_search_approx_size = 0;
#endif

  best_merit = 0;
  best_gestation = 0.0;
  best_fitness = 0.0;
  max_fitness = 0.0;

  // Clear Sums that are collected durring the update
  sum_repro_rate.Clear();
}


void cStats::TestThreads(const cCodeArray & code)
{
  static ofstream fp("threads.dat");

  cCPUTestInfo test_info(TEST_CPU_GENERATIONS, TRUE);
  cTestCPU::TestCode(test_info, code);

  cDivideRecord * divide1 = test_info.GetDivideRecord(1);
  cDivideRecord * divide2 = test_info.GetDivideRecord(2);

  fp << cStats::GetUpdate()            << " "   // 1
     << code.GetSize()                 << " "   // 2
     << sum_num_threads.Average()      << " "   // 3
     << sum_thread_dist.Average()      << " ";  // 4

  if (divide1) {
    fp << divide1->GetMerit()          << " "   // 5
       << divide1->GetGestationTime()  << " "   // 6
       << divide1->GetFitness()        << " "   // 7
       << divide1->GetThreadFrac()     << " "   // 8
       << divide1->GetThreadTimeDiff() << " "   // 9
       << divide1->GetThreadCodeDiff() << " ";  // 10
  } else {
    fp << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0;
  }

  if (divide2) {
    fp << divide2->GetMerit()          << " "   // 11
       << divide2->GetGestationTime()  << " "   // 12
       << divide2->GetFitness()        << " "   // 13
       << divide2->GetThreadFrac()     << " "   // 14
       << divide2->GetThreadTimeDiff() << " "   // 15
       << divide2->GetThreadCodeDiff() << " ";  // 16
  } else {
    fp << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0;
  }

  fp << endl;
}


void cStats::PrintThreads(const cCodeArray & code)
{
  cCPUTestInfo test_info(TEST_CPU_GENERATIONS, TRUE, TRUE);
  cTestCPU::TestCode(test_info, code);
}


void cStats::FlushFP(){
  data_file_manager.FlushAll();

  fp_genotype_status.flush();
  fp_diversity_status.flush();

  fp_creature_log.flush();
  fp_breed_log.flush();
  fp_genotype_log.flush();
  fp_threshold_log.flush();
  fp_species_log.flush();
  fp_landscape_log.flush();
}

////////////////////////////////////////////////
//  And now, a whole mess of static variables!
////////////////////////////////////////////////

// -- cStats --
int cStats::current_update;
double cStats::ave_generation;
int cStats::sub_update;
double cStats::avida_time;

cDataFileManager cStats::data_file_manager;

ofstream cStats::fp_genotype_status;
ofstream cStats::fp_diversity_status;
ofstream cStats::fp_creature_log;
ofstream cStats::fp_phylogeny_log;
ofstream cStats::fp_geneology_log;
ofstream cStats::fp_breed_log;
ofstream cStats::fp_genotype_log;
ofstream cStats::fp_threshold_log;
ofstream cStats::fp_species_log;
ofstream cStats::fp_landscape_log;
cDoubleSum cStats::sum_repro_rate;
cDoubleSum cStats::sum_merit;
cDoubleSum cStats::sum_mem_size;
cDoubleSum cStats::sum_creature_age;
cDoubleSum cStats::sum_generation;
cDoubleSum cStats::sum_neutral_metric;
cDoubleSum cStats::sum_lineage_label;
tVector<cDoubleSum> cStats::sum_lineage_merit_c;
tVector<cDoubleSum> cStats::sum_lineage_fitness_c;
cDoubleSum cStats::sum_num_threads;
cDoubleSum cStats::sum_thread_dist;
cDoubleSum cStats::sum_gestation;
cDoubleSum cStats::sum_fitness;
cRunningAverage cStats::rave_true_replication_rate( 500 );
cDoubleSum cStats::sum_size;
cDoubleSum cStats::sum_copy_size;
cDoubleSum cStats::sum_exe_size;
cDoubleSum cStats::sum_genotype_age;
cDoubleSum cStats::sum_abundance;
cDoubleSum cStats::sum_genotype_depth;
cDoubleSum cStats::sum_threshold_age;
cDoubleSum cStats::sum_species_age;
cIntSum cStats::isum_parent_dist;
cIntSum cStats::isum_parent_size;
cIntSum cStats::isum_child_size;
cIntSum cStats::isum_point_mut;
cIntSum cStats::isum_copy_mut;
cIntSum cStats::isum_insert_mut;
cIntSum cStats::isum_point_mut_line;
cIntSum cStats::isum_copy_mut_line;
cIntSum cStats::isum_delete_mut;
cIntSum cStats::isum_divide_mut;
cIntSum cStats::isum_divide_insert_mut;
cIntSum cStats::isum_divide_delete_mut;
cIntSum cStats::isum_copied_size;
cIntSum cStats::isum_executed_size;
cIntSum cStats::isum_copies_exec;
cDoubleSum cStats::dsum_copy_mut_by_copies_exec;
cDoubleSum cStats::dsum_copied_size_by_copies_exec;
cDoubleSum cStats::dsum_copy_mut_lines_by_copied_size;
cDoubleSum cStats::dsum_copy_mut_lines_by_copy_mut;
cIntSum cStats::isum_num_inst_exec;
tArray<cIntSum> cStats::isum_array_inst_exec_count;
cIntSum cStats::isum_dom_num_inst_exec;
tArray<cIntSum> cStats::isum_dom_array_inst_exec_count;
double cStats::entropy;
double cStats::species_entropy;
double cStats::energy;
double cStats::dom_fidelity;
double cStats::ave_fidelity;
cGenotype * cStats::dom_genotype;
double cStats::best_merit;
double cStats::best_gestation;
double cStats::best_repro_rate;
double cStats::best_fitness;
int cStats::best_size;
double cStats::best_copied_size;
double cStats::best_exe_size;
double cStats::max_fitness;
double cStats::max_merit;
int cStats::best_genotype_id;
cString cStats::best_name;
int cStats::best_births;
int cStats::best_breed_true;
int cStats::best_breed_in;
int cStats::best_abundance;
int cStats::best_gene_depth;
int cStats::num_births;
int cStats::num_deaths;
int cStats::num_breed_in;
int cStats::num_breed_true;
int cStats::num_breed_true_creatures;
int cStats::num_creatures;
int cStats::num_genotypes;
int cStats::num_threshold;
int cStats::num_species;
int cStats::num_thresh_species;
int cStats::num_executed;
int cStats::num_parasites;
int cStats::num_no_birth_creatures;
int cStats::num_genotypes_last;
int cStats::tot_creatures;
int cStats::tot_genotypes;
int cStats::tot_threshold;
int cStats::tot_species;
int cStats::tot_thresh_species;
int cStats::tot_executed;
int * cStats::task_count;
int * cStats::task_exe_count;
int cStats::num_inst_executed;

#ifdef TEST_SEARCH_SIZE
int cStats::num_search_approx_size;
#endif



void cStats::ReportNewCreature( sCPUStats & cpu_stats ){
  cpu_stats.NewCreatureClear();
}


void cStats::ReportDivide( sCPUStats & p_stats, sCPUStats & d_stats ){
  // Called by cPopulation::ActiveChild

  //// Instruction Execution Counts ////
#ifdef INSTRUCTION_COUNT

  //// Record for Dominant if it is it.... ////
  if( best_genotype_id == p_stats.genotype_id ){
    // Resize the isum_dom_array_inst_exec_count if needed
    if( isum_dom_array_inst_exec_count.GetSize() <
	p_stats.inst_stats.exec_count.GetSize() ){
      g_debug.Comment("Resizing cStats::isum_dom_array_inst_exec_count.  Inst-lib size change?");
      isum_dom_array_inst_exec_count.ResizeClear(p_stats.inst_stats.exec_count.GetSize());
      for( int i=0; i<isum_array_inst_exec_count.GetSize(); ++i ){
	isum_dom_array_inst_exec_count[i].Clear();
      }
    }
    // Add the variables... Not Clearing (we do that later)
    isum_dom_num_inst_exec.Add(p_stats.inst_stats.num_executed);
    for( int i=0; i<p_stats.inst_stats.exec_count.GetSize(); ++i ){
      isum_dom_array_inst_exec_count[i].Add(p_stats.inst_stats.exec_count[i]);
    }
  }

  // Resize the isum_array_inst_exec_count if needed
  if( isum_array_inst_exec_count.GetSize() <
      p_stats.inst_stats.exec_count.GetSize() ){
    g_debug.Comment("Resizing cStats::isum_array_inst_exec_count.  Inst-lib size change?");
    isum_array_inst_exec_count.ResizeClear(p_stats.inst_stats.exec_count.GetSize());
    for( int i=0; i<isum_array_inst_exec_count.GetSize(); ++i ){
      isum_array_inst_exec_count[i].Clear();
    }
  }
  // Add the variables... Clearing as we go (more efficient)
  isum_num_inst_exec.Add(p_stats.inst_stats.num_executed);
  p_stats.inst_stats.num_executed = 0;
  for( int i=0; i<p_stats.inst_stats.exec_count.GetSize(); ++i ){
    isum_array_inst_exec_count[i].Add(p_stats.inst_stats.exec_count[i]);
    p_stats.inst_stats.exec_count[i] = 0;
  }

#endif

  //// Mutation Counts ////
  isum_parent_size.Add( p_stats.size );
  isum_child_size.Add( p_stats.child_size );
  isum_executed_size.Add( p_stats.copied_size );
  isum_copied_size.Add( p_stats.executed_size );
  isum_parent_dist.Add( p_stats.parent_dist );
  isum_point_mut.Add( p_stats.mut_stats.point_mut_count );
  isum_copy_mut.Add( p_stats.mut_stats.copy_mut_count );
  isum_insert_mut.Add( p_stats.mut_stats.insert_mut_count );
  isum_delete_mut.Add( p_stats.mut_stats.delete_mut_count );
  isum_divide_mut.Add( p_stats.mut_stats.divide_mut_count );
  isum_divide_insert_mut.Add( p_stats.mut_stats.divide_insert_mut_count );
  isum_divide_delete_mut.Add( p_stats.mut_stats.divide_delete_mut_count );
  isum_point_mut_line.Add( p_stats.mut_stats.point_mut_line_count );
  isum_copy_mut_line.Add( p_stats.mut_stats.copy_mut_line_count );
  isum_copies_exec.Add( p_stats.mut_stats.copies_exec );
  if( p_stats.mut_stats.copies_exec ){
    dsum_copy_mut_by_copies_exec.Add(p_stats.mut_stats.copy_mut_count /
				     (double)p_stats.mut_stats.copies_exec);
    dsum_copied_size_by_copies_exec.Add(p_stats.copied_size /
					(double)p_stats.mut_stats.copies_exec);
  }
  if( p_stats.copied_size ){
    dsum_copy_mut_lines_by_copied_size.Add(p_stats.mut_stats.
		   copy_mut_line_count / (double)p_stats.copied_size);
  }
  if( p_stats.mut_stats.copy_mut_line_count ){
    dsum_copy_mut_lines_by_copy_mut.Add(p_stats.mut_stats.copy_mut_line_count
				/ (double)p_stats.mut_stats.copy_mut_count);
  }

    //// Clearing ////
  p_stats.DivideClear();
  // cBaseCPU::ChangeGenotype calls ReportNewCreature
}


void cStats::ReportDeath( sCPUStats & cpu_stats ){
  cpu_stats.DeathClear();
}



//// Lineage based stats ////
void cStats::ClearSumLineageStats(){
  for( int i=0; i<sum_lineage_merit_c.Size(); ++i ){
    sum_lineage_merit_c[i].Clear();
    sum_lineage_fitness_c[i].Clear();
  }
}


cDoubleSum & cStats::AccessLineageStat(tVector<cDoubleSum>& stat_c, int i){
  while( i>= stat_c.Size() ){
    cDoubleSum tmp;
    stat_c.Add(tmp);
  }
  return stat_c[i];
}






void cStats::PrintLineageData(const cString & filename){
  tArray<ofstream *> fp(sum_lineage_merit_c.Size());
  cString fname;
  for( int i=0; i<sum_lineage_merit_c.Size(); ++i ){
    fname.Set("%s.%d", filename(),i);
    fp[i] = &GetDataFileOFStream(fname);
    assert(fp[i]->good());
  }


  for( int i=0; i<sum_lineage_merit_c.Size(); ++i ){
    (*fp[i])<< GetUpdate()                           << " "; // 1
    (*fp[i])<< sum_lineage_merit_c[i].Count()        << " "; // 2
    (*fp[i])<< sum_lineage_merit_c[i].Average()      << " "; // 3
    (*fp[i])<< sum_lineage_fitness_c[i].Average()    << " "; // 4
    (*fp[i])<<endl;
  }	

}


void cStats::PrintAverageData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()                          << " "  // 1
    << sum_merit.Average()                  << " "  // 2
    << sum_gestation.Average()              << " "  // 3
    << sum_fitness.Average()                << " "  // 4
    << sum_repro_rate.Average()             << " "  // 5
    << sum_size.Average()                   << " "  // 6
    << sum_copy_size.Average()              << " "  // 7
    << sum_exe_size.Average()               << " "  // 8
    << sum_abundance.Average()              << " "  // 9
    << (double)num_births/num_creatures     << " "  // 10
    << (double)num_breed_true/num_creatures << " "  // 11
    << sum_genotype_depth.Average()         << " "  // 12
    << sum_num_threads.Average()            << " "  // 13
    << sum_thread_dist.Average()            << " "  // 14
    << sum_generation.Average()             << " "  // 15
    << sum_neutral_metric.Average()         << " "  // 16
    << sum_lineage_label.Average()          << " "  // 17
    << rave_true_replication_rate.Average() << endl;// 18
}


void cStats::PrintErrorData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()                          << " "  // 1
    << sum_merit.StdError()                 << " "  // 2
    << sum_gestation.StdError()             << " "  // 3
    << sum_fitness.StdError()               << " "  // 4
    << sum_repro_rate.StdError()            << " "  // 5
    << sum_size.StdError()                  << " "  // 6
    << sum_copy_size.StdError()             << " "  // 7
    << sum_exe_size.StdError()              << " "  // 8
    << sum_abundance.StdError()             << " "  // 9
    << -1                                   << " "  // 10
    << -1                                   << " "  // 11
    << sum_genotype_depth.StdError()        << " "  // 12
    << sum_num_threads.StdError()           << " "  // 13
    << sum_thread_dist.StdError()           << " "  // 14
    << sum_generation.StdError()            << " "  // 15
    << sum_neutral_metric.StdError()        << " "  // 16
    << sum_lineage_label.StdError()         << endl;// 17
}


void cStats::PrintVarianceData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<<GetUpdate()                           << " "  // 1
    << sum_merit.Variance()                 << " "  // 2
    << sum_gestation.Variance()             << " "  // 3
    << sum_fitness.Variance()               << " "  // 4
    << sum_repro_rate.Variance()            << " "  // 5
    << sum_size.Variance()                  << " "  // 6
    << sum_copy_size.Variance()             << " "  // 7
    << sum_exe_size.Variance()              << " "  // 8
    << sum_abundance.Variance()             << " "  // 9
    << -1                                   << " "  // 10
    << -1                                   << " "  // 11
    << sum_genotype_depth.Variance()        << " "  // 12
    << sum_num_threads.Variance()           << " "  // 13
    << sum_thread_dist.Variance()           << " "  // 14
    << sum_generation.Variance()            << " "  // 15
    << sum_neutral_metric.Variance()        << " "  // 16
    << sum_lineage_label.Variance()         << " "  // 17
    << rave_true_replication_rate.Variance()<< endl;// 18
}


void cStats::PrintDominantData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<<GetUpdate()            <<" "   // 1
    <<best_merit             <<" "   // 2
    <<best_gestation         <<" "   // 3
    <<best_fitness           <<" "   // 4
    <<best_repro_rate        <<" "   // 5
    <<best_size              <<" "   // 6
    <<best_copied_size       <<" "   // 7
    <<best_exe_size          <<" "   // 8
    <<best_abundance         <<" "   // 9
    <<best_births            <<" "   // 10
    <<best_breed_true        <<" "   // 11
    <<best_gene_depth        <<" "   // 12
    // These would be nice
    //<<best_num_threads       <<" "   // 13
    //<<best_thread_dist       <<" "   // 14
    //<<best_generation        <<" "   // 15
    //<<best_neutral_metric    <<" "   // 16
    //<<best_lineage_label     <<" "   // 17
    <<best_breed_in          <<" "   // 13
    <<max_fitness            <<" "   // 14
    <<best_genotype_id       <<" "   // 15
    <<best_name              <<endl; // 16
}


void cStats::PrintStatsData(const cString & filename){
  int genotype_change = num_genotypes - num_genotypes_last;
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<<GetUpdate()<<" "            // 1
    <<energy<<" "                 // 2
    <<(1.0-ave_fidelity)<<" "     // 3
    <<1.0-dom_fidelity<<" ";      // 4
  if( ave_fidelity > 0 )
    fp<<-Log(ave_fidelity)<<" ";  // 5
  else
    fp<<0<<" ";
  if( ave_fidelity > 0 )
    fp<<-Log(dom_fidelity)<<" ";  // 6
  else
    fp<<0<<" ";
  fp<<genotype_change<<" "        // 7
    <<entropy<<" "                // 8
    <<species_entropy<<endl;      // 9
}


void cStats::PrintCountData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()              << " "  // 1
    << num_executed             << " "  // 2
    << num_creatures            << " "  // 3
    << num_genotypes            << " "  // 4
    << num_threshold            << " "  // 5
    << num_species              << " "  // 6
    << num_thresh_species       << " "  // 7
    << num_births               << " "  // 8
    << num_deaths               << " "  // 9
    << num_breed_true           << " "  // 10
    << num_breed_true_creatures << " "  // 11
    << num_parasites            << " "  // 12
    << num_no_birth_creatures   << " "  // 13
#ifdef TEST_SEARCH_SIZE
    << num_search_approx_size   << " "  // 14
#endif
    << endl;
}


void cStats::PrintTotalsData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<<GetUpdate()<<" "    // 1
    <<(tot_executed+num_executed)<<" "   // 2
    << num_inst_executed             << " "  // 2
    <<tot_creatures<<" "  // 3
    <<tot_genotypes<<" "  // 4
    <<tot_threshold<<" "  // 5
    <<tot_species<<endl;  // 6
}


void cStats::PrintTasksData(const cString & filename){
  int i;
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<<GetUpdate();
  for( i=0; i<cConfig::GetNumTasks(); i++ ){
    fp<<" "<<task_count[i];
  }
  fp<<endl;
}


void cStats::PrintTasksExeData(const cString & filename){
  int i;
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<<GetUpdate();
  for( i=0; i<cConfig::GetNumTasks(); i++ ){
    fp<<" "<<task_exe_count[i];
    task_exe_count[i] = 0;
  }
  fp<<endl;
}


void cStats::PrintTimeData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()        <<" "   // 1
    << avida_time         <<" "   // 2
    << GetAveGeneration() <<" "   // 3
    << num_inst_executed <<endl;  // 4
}


void cStats::PrintMutationData(const cString & filename){
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()                              <<" "   // 1
    << isum_parent_dist.Ave()                   <<" "   // 2
    << isum_parent_size.Ave()                   <<" "   // 3
    << isum_child_size.Ave()                    <<" "   // 4
    << isum_copied_size.Ave()                   <<" "   // 5
    << isum_executed_size.Ave()                 <<" "   // 6
    << isum_copies_exec.Ave()                   <<" "   // 7
    << isum_point_mut.Ave()                     <<" "   // 8
    << isum_copy_mut.Ave()                      <<" "   // 9
    << isum_insert_mut.Ave()                    <<" "   // 10
    << isum_delete_mut.Ave()                    <<" "   // 11
    << isum_point_mut_line.Ave()                <<" "   // 12
    << isum_copy_mut_line.Ave()                 <<" "   // 13
    << isum_divide_mut.Ave()                    <<" "   // 14
    << isum_divide_insert_mut.Ave()             <<" "   // 15
    << isum_divide_delete_mut.Ave()             <<" "   // 16
    << dsum_copy_mut_by_copies_exec.Ave()       <<" "   // 17
    << dsum_copied_size_by_copies_exec.Ave()    <<" "   // 18
    << dsum_copy_mut_lines_by_copied_size.Ave() <<" "   // 19
    << dsum_copy_mut_lines_by_copy_mut.Ave()    <<endl; // 20
  isum_parent_dist.Clear();
  isum_parent_size.Clear();
  isum_child_size.Clear();
  isum_point_mut.Clear();
  isum_copy_mut.Clear();
  isum_insert_mut.Clear();
  isum_point_mut_line.Clear();
  isum_copy_mut_line.Clear();
  isum_delete_mut.Clear();
  isum_divide_mut.Clear();
  isum_divide_insert_mut.Clear();
  isum_divide_delete_mut.Clear();
  isum_copied_size.Clear();
  isum_executed_size.Clear();
  isum_copies_exec.Clear();
  dsum_copy_mut_by_copies_exec.Clear();
  dsum_copied_size_by_copies_exec.Clear();
  dsum_copy_mut_lines_by_copied_size.Clear();
  dsum_copy_mut_lines_by_copy_mut.Clear();
}


void cStats::PrintInstructionData(const cString & filename){
#ifdef INSTRUCTION_COUNT
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()                   <<" "  // 1
    << isum_num_inst_exec.Ave()  <<" "; // 2
  isum_num_inst_exec.Clear();
  for( int i=0; i<isum_array_inst_exec_count.GetSize(); ++i ){
    fp<< isum_array_inst_exec_count[i].Ave() <<" ";
    isum_array_inst_exec_count[i].Clear();
  }
  fp<<endl;
#else // INSTRUCTION_COUNT undefined
  // @TCC make NotifyWarning somehow
  cerr<<"Warning: Instruction Counts not compiled in"<<endl;
#endif // ifdef INSTRUCTION_COUNT
}


void cStats::PrintDomInstructionData(const cString & filename){
#ifdef INSTRUCTION_COUNT
  ofstream & fp = GetDataFileOFStream(filename);
  assert(fp.good());
  fp<< GetUpdate()                   <<" "  // 1
    << isum_dom_num_inst_exec.Ave()  <<" "; // 2
  isum_dom_num_inst_exec.Clear();
  for( int i=0; i<isum_dom_array_inst_exec_count.GetSize(); ++i ){
    fp<< isum_dom_array_inst_exec_count[i].Ave() <<" ";
    isum_dom_array_inst_exec_count[i].Clear();
  }
  fp<<endl;
#else // INSTRUCTION_COUNT undefined
  // @TCC make NotifyWarning somehow
  cerr<<"Warning: Instruction Counts not compiled in"<<endl;
#endif // ifdef INSTRUCTION_COUNT
}




