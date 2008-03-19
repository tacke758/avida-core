//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef STATS_HH
#define STATS_HH

#include <fstream.h>
#include <iostream.h>
#include "../defs.hh"
#include "../tools/tools.hh"
#include "../tools/tVector.hh"
#include "../tools/datafile.hh"
#include "config.hh"
#include "tasks.hh"
#include "../cpu/cpu_stats.hh"

class sInstructionStats;  // A class in CPU used by stats...
class cDataFile;


class cStats {
friend class cDataFile;
private:

  // Time scales...
  static int current_update;
  static double ave_generation;
  static int sub_update;
  static double avida_time;

  // Output Files:  data files are taken every N updates and report the
  // current state of the soup in a rigid format.  Status files are also
  // printed every N updates, but are much more flexible in their format,
  // and must typically be processed more before they can be used.  Log files
  // are taken every time a specified event occurs.

  static cDataFileManager data_file_manager;

  static ofstream fp_genotype_status;
  static ofstream fp_diversity_status;

  static ofstream fp_creature_log;
  static ofstream fp_phylogeny_log;
  static ofstream fp_geneology_log;
  static ofstream fp_breed_log;
  static ofstream fp_genotype_log;
  static ofstream fp_threshold_log;
  static ofstream fp_species_log;
  static ofstream fp_landscape_log;


  //// By Time Sums ////  (Recoreded to stats upon appropriate events)
  static cDoubleSum sum_repro_rate;


  //// By Creature Sums ////  (Cleared and resummed by population each update)
  static cDoubleSum sum_merit;
  static cDoubleSum sum_mem_size;
  static cDoubleSum sum_creature_age;
  static cDoubleSum sum_generation;
  static cDoubleSum sum_num_threads;
  static cDoubleSum sum_thread_dist;

  static cDoubleSum sum_neutral_metric;
  static cDoubleSum sum_lineage_label;

  //// By Lineage Stats (collected over by creature) ////
  static tVector<cDoubleSum> sum_lineage_merit_c;
  static tVector<cDoubleSum> sum_lineage_fitness_c;

  //// By Genotype Sums ////  (Cleared and resummed by population each update)

  // Merit, Gestation, &  Fitness
  static cDoubleSum sum_gestation;
  static cDoubleSum sum_fitness;
  
  // calculates a running average over the actual replication rate
  // given by the number of births per update
  static cRunningAverage rave_true_replication_rate;

  // Sizes
  static cDoubleSum sum_size;
  static cDoubleSum sum_copy_size;
  static cDoubleSum sum_exe_size;

  // Age
  static cDoubleSum sum_genotype_age;

  // breed/geneological stats
  static cDoubleSum sum_abundance;
  static cDoubleSum sum_genotype_depth;


  //// By Threshold Sums ////  (Cleared and resummed by population each update)

  static cDoubleSum sum_threshold_age;


  //// By Species Sums ////  (Cleared and resummed by population each update)

  static cDoubleSum sum_species_age;


  //// Sums cleard on output only ////
  static cIntSum isum_parent_dist;
  static cIntSum isum_parent_size;
  static cIntSum isum_child_size;
  static cIntSum isum_point_mut;
  static cIntSum isum_copy_mut;
  static cIntSum isum_insert_mut;
  static cIntSum isum_point_mut_line;
  static cIntSum isum_copy_mut_line;
  static cIntSum isum_delete_mut;
  static cIntSum isum_divide_mut;
  static cIntSum isum_divide_insert_mut;
  static cIntSum isum_divide_delete_mut;
  static cIntSum isum_copied_size;
  static cIntSum isum_executed_size;
  static cIntSum isum_copies_exec;
  static cDoubleSum dsum_copy_mut_by_copies_exec;
  static cDoubleSum dsum_copied_size_by_copies_exec;
  static cDoubleSum dsum_copy_mut_lines_by_copied_size;
  static cDoubleSum dsum_copy_mut_lines_by_copy_mut;
  // Instruction Counts (ave over time between dumps)
  static cIntSum isum_num_inst_exec;
  static tArray<cIntSum> isum_array_inst_exec_count;
  static cIntSum isum_dom_num_inst_exec;
  static tArray<cIntSum> isum_dom_array_inst_exec_count;



  // Calculated stats
  static double entropy;
  static double species_entropy;
  static double energy;
  static double dom_fidelity;
  static double ave_fidelity;


  // Dominant Genotype
  static cGenotype * dom_genotype;
  static double best_merit;
  static double best_gestation;
  static double best_repro_rate;
  static double best_fitness;
  static int best_size;
  static double best_copied_size;
  static double best_exe_size;
  static double max_fitness;
  static double max_merit;
  static int best_genotype_id;
  static cString best_name;
  static int best_births;
  static int best_breed_true;
  static int best_breed_in;
  static int best_abundance;
  static int best_gene_depth;

  static int num_births;
  static int num_deaths;
  static int num_breed_in;
  static int num_breed_true;
  static int num_breed_true_creatures;
  static int num_creatures;
  static int num_genotypes;
  static int num_threshold;
  static int num_species;
  static int num_thresh_species;
  static int num_executed;
  static int num_parasites;
  static int num_no_birth_creatures;

  static int num_inst_executed;

  static int num_genotypes_last;

#ifdef TEST_SEARCH_SIZE
  static int num_search_approx_size;
#endif

  static int tot_creatures;
  static int tot_genotypes;
  static int tot_threshold;
  static int tot_species;
  static int tot_thresh_species;
  static int tot_executed;

  static int * task_count;
  static int * task_exe_count;

public:
  static void Setup();
  static void Shutdown();

  static void ProcessUpdate(int new_update);

  // Specific event-triggered files.

  static void TestThreads(const cCodeArray & code);
  static void PrintThreads(const cCodeArray & code);


  // GLOBAL STATS

  static inline void SetCurrentUpdate(int new_update) {
    current_update = new_update;
    sub_update = 0;
  }

  static inline void IncCurrentUpdate() {
    current_update++;
    sub_update = 0;
  }

  static inline void IncSubUpdate() { sub_update++; }

  static inline int GetUpdate() { return current_update; }
  static inline int GetSubUpdate() { return sub_update; }
  static inline double GetAveGeneration() { return ave_generation; }

  static inline void ClearAveGeneration() {  // Needed for inject all
    sum_generation.Clear(); ave_generation=0; }

  // LOCAL STATS

  static int OK() { return TRUE; }  // @CAO FIX NOW!!!!

  static inline ofstream & GetGenotypeStatusFP()
    { return fp_genotype_status; }
  static inline ofstream & GetDiversityStatusFP()
    { return fp_diversity_status; }

  static void FlushFP(); // Flush all the files

  // World data

  //// Reporting... Clears input variables! ////
  static void ReportNewCreature( sCPUStats & cpu_stats );
  static void ReportDivide( sCPUStats & parent, sCPUStats & daughter );
  static void ReportDeath( sCPUStats & cpu_stats );
  // End Reporting Functions



  // Generic data

  static void SetBestGenotype(cGenotype * best_genotype);

  static inline void SetNumGenotypes(int new_genotypes);
  static inline void SetNumCreatures(int new_creatures);
  static inline void SetNumThreshSpecies(int new_thresh_species);
  static inline void SetBreedTrueCreatures(int in_num_breed_true_creatures);
  static inline void SetNumParasites(int in_num_parasites);
  static inline void SetNumNoBirthCreatures(int in_num_no_birth_creatures);

  static inline void SetMaxFitness(double in_max_fitness)
    { max_fitness = in_max_fitness; }
  static inline void SetMaxMerit(double in_max_merit)
    { max_merit = in_max_merit; }
  static inline void SetEntropy(double in_entropy)
    { entropy = in_entropy; }
  static inline void SetSpeciesEntropy(double in_ent)
    { species_entropy = in_ent; }

  static inline cDoubleSum & SumReproRate()     { return sum_repro_rate; }

  static inline cDoubleSum & SumMerit()         { return sum_merit; }
  static inline cDoubleSum & SumCreatureAge()   { return sum_creature_age; }
  static inline cDoubleSum & SumGeneration()    { return sum_generation; }
  static inline cDoubleSum & SumNumThreads()    { return sum_num_threads; }
  static inline cDoubleSum & SumThreadDist()    { return sum_thread_dist; }
  static inline cDoubleSum & SumMemSize()       { return sum_mem_size; }

  static inline cDoubleSum & SumNeutralMetric() { return sum_neutral_metric; }
  static inline cDoubleSum & SumLineageLabel()  { return sum_lineage_label; }


  //// Lineage Based Stats
  static void ClearSumLineageStats();
private:
  static cDoubleSum & AccessLineageStat(tVector<cDoubleSum>& stat_c, int i);
public:
  static cDoubleSum & SumLineageMerit(int i){
    return AccessLineageStat(sum_lineage_merit_c, i); }
  static cDoubleSum & SumLineageFitness(int i){
    return AccessLineageStat(sum_lineage_fitness_c, i); }


  static inline cDoubleSum & SumGestation()     { return sum_gestation; }
  static inline cDoubleSum & SumFitness()       { return sum_fitness; }

  static inline cDoubleSum & SumGenotypeAge()   { return sum_genotype_age; }

  static inline cDoubleSum & SumSize()          { return sum_size; }
  static inline cDoubleSum & SumCopySize()      { return sum_copy_size; }
  static inline cDoubleSum & SumExeSize()       { return sum_exe_size; }

#ifdef TEST_SEARCH_SIZE
  static inline int  GetSearchApproxSize(){ return num_search_approx_size; }
  static inline void AddSearchApproxSize(){ ++num_search_approx_size; }
#endif

  static inline cDoubleSum & SumAbundance()     { return sum_abundance; }
  static inline cDoubleSum & SumGenotypeDepth() { return sum_genotype_depth; }

  static inline cDoubleSum & SumThresholdAge()  { return sum_threshold_age; }
  static inline cDoubleSum & SumSpeciesAge()    { return sum_species_age; }

  static inline void AddBirth() { num_births++; }
  static inline void AddDeath() { num_deaths++; }
  static inline void AddBreedIn() { num_births++; }
  static inline void AddBreedTrue() { num_breed_true++; }

  static inline void CalcEnergy();
  static inline void CalcFidelity();

  static inline void AddCreature(int id_num, int genotype_num);
  static inline void NewCreature(int genotype_num, int parent_id,
				 int parent_dist,
				 int parent_size,
				 int child_size,
				 int point_mut_count = -1,
				 int copy_mut_count = -1,
				 int insert_mut_count = -1,
				 int point_mut_line_count = -1,
				 int copy_mut_line_count = -1,
				 int delete_mut_count = -1,
				 int divide_mut_count = -1,
				 int divide_insert_mut_count = -1,
				 int divide_delete_mut_count = -1,
				 int copied_size = -1,
				 int executed_size = -1,
				 int copies_exec = -1);
  static inline void AddGenotype(int id_num);
  static inline void RemoveCreature(int genotype_num, int num_divides,int age);
  static inline void RemoveGenotype(int id_num, int parent_id,
			    int parent_distance, int depth, int max_abundance,
			    int parasite_abundance, int age, int length);
  static inline void AddThreshold(int id_num, const char * name,
				  int species_num=-1);
  static inline void RemoveThreshold(int id_num);
  static inline void AddSpecies(int id_num);
  static inline void RemoveSpecies(int id_num, int parent_id,
			    int max_gen_abundance, int max_abundance, int age);

  static inline void AddExecuted(int in_executed)
    { num_executed += in_executed; }

  static inline void IncNumInstExecuted(){ ++num_inst_executed; }

  static inline void SetGets(int in_gets);
  static inline void SetPuts(int in_puts);
  static inline void SetGGPs(int in_ggps);
  static inline void SetTasks(int task_num, int in_tasks);
  static inline void AddTasks(int task_num, int in_tasks);
  static        void ZeroTasks();

  static inline void IncTaskExeCount(int task_num){
    assert( task_num >= 0  &&  task_num < cConfig::GetNumTasks() );
    ++task_exe_count[task_num]; }

#ifdef GENEOLOGY
  static inline ostream & GetGeneologyLog(){ return fp_geneology_log; }
#endif

  static inline ostream & GetLandscapeLog() { return fp_landscape_log; }

  // Information retrieval section...

  static inline int GetNumBirths()        { return num_births; }
  static inline int GetNumDeaths()        { return num_deaths; }
  static inline int GetBreedIn()          { return num_breed_in; }
  static inline int GetBreedTrue()        { return num_breed_true; }
  static inline int GetBreedTrueCreatures()
    { return num_breed_true_creatures; }
  static inline int GetNumCreatures()     { return num_creatures; }
  static inline int GetNumGenotypes()     { return num_genotypes; }
  static inline int GetNumThreshold()     { return num_threshold; }
  static inline int GetNumSpecies()       { return num_species; }
  static inline int GetNumThreshSpecies() { return num_thresh_species; }
  static inline int GetNumParasites()     { return num_parasites; }
  static inline int GetNumNoBirthCreatures(){ return num_no_birth_creatures; }

  static inline int GetTotCreatures()     { return tot_creatures; }
  static inline int GetTotGenotypes()     { return tot_genotypes; }
  static inline int GetTotThreshold()     { return tot_threshold; }
  static inline int GetTotSpecies()       { return tot_species; }
  static inline int GetTotThreshSpecies() { return tot_thresh_species; }

  static inline int GetTaskCount(int task_num) { return task_count[task_num]; }
  static inline int GetTaskExeCount(int task_num) { return task_exe_count[task_num]; }

  static inline double GetAveReproRate()   { return sum_repro_rate.Average(); }

  static inline double GetAveMerit()
    { return sum_merit.Average(); }
  static inline double GetAveCreatureAge()
    { return sum_creature_age.Average(); }
  static inline double GetAveNumThreads() { return sum_num_threads.Average(); }
  static inline double GetAveThreadDist() { return sum_thread_dist.Average(); }
  static inline double GetAveMemSize()    { return sum_mem_size.Average(); }

  static inline double GetAveNeutralMetric() {
    return sum_neutral_metric.Average(); }

  static inline double GetAveLineageLabel() {
    return sum_lineage_label.Average(); }

  static inline double GetAveGestation()  { return sum_gestation.Average(); }
  static inline double GetAveFitness()    { return sum_fitness.Average(); }

  static inline double GetAveGenotypeAge(){ return sum_genotype_age.Average();}

  static inline double GetAveSize()        { return sum_size.Average(); }
  static inline double GetAveCopySize()    { return sum_copy_size.Average(); }
  static inline double GetAveExeSize()     { return sum_exe_size.Average(); }

  static inline double GetEntropy()        { return entropy; }
  static inline double GetSpeciesEntropy() { return species_entropy; }
  static inline double GetEnergy()         { return energy; }

  static inline double GetAveThresholdAge()
    { return sum_threshold_age.Average(); }
  static inline double GetAveSpeciesAge() { return sum_species_age.Average(); }

  static inline double GetMaxFitness() { return max_fitness; }
  static inline double GetMaxMerit()   { return max_merit; }

  // Access to data_file_manager (so cPopulation can output files)
  static inline ofstream & GetDataFileOFStream(const cString & fname){
    return data_file_manager.GetOFStream(fname); }

  // Public calls to output data files (for events)
  static void PrintLineageData(const cString & filename);
  static void PrintAverageData(const cString & filename);
  static void PrintErrorData(const cString & filename);
  static void PrintVarianceData(const cString & filename);
  static void PrintDominantData(const cString & filename);
  static void PrintStatsData(const cString & filename);
  static void PrintCountData(const cString & filename);
  static void PrintTotalsData(const cString & filename);
  static void PrintTasksData(const cString & filename);
  static void PrintTasksExeData(const cString & filename);
  static void PrintTimeData(const cString & filename);
  static void PrintMutationData(const cString & filename);
  static void PrintInstructionData(const cString & filename);
  static void PrintDomInstructionData(const cString & filename);
};


inline void cStats::CalcEnergy()
{
  if (sum_fitness.Average() == 0.0 || best_fitness == 0.0) {
    energy = 0.0;
  } else  {
    energy = Log(best_fitness / sum_fitness.Average());
  }
}

inline void cStats::CalcFidelity()
{
  // There is a (small) probability that when a random instruction is picked
  // after a mutation occurs, that it will be the original instruction again;
  // This needs to be adjusted for!

  double adj = ((double) (cConfig::GetNumInstructions() - 1)) /
    (double) cConfig::GetNumInstructions();

  double base_fidelity = (1.0 - adj * cConfig::GetDivideMutProb()) *
    (1.0 - cConfig::GetDivideInsProb()) * (1.0 - cConfig::GetDivideDelProb());

  double true_cm_rate = adj * cConfig::GetCopyMutProb();
  ave_fidelity = base_fidelity * pow(1.0 - true_cm_rate, sum_size.Average());
  dom_fidelity = base_fidelity * pow(1.0 - true_cm_rate, best_size);
}

inline void cStats::AddCreature(int id_num, int genotype_num ){
  tot_creatures++;
  if( fp_creature_log.good() )
    fp_creature_log<<cStats::GetUpdate()<<" "
		   <<id_num<<" "
		   <<genotype_num<<endl;
}

inline void cStats::NewCreature(int genotype_num, int parent_id,
				int parent_dist,
				int parent_size,
				int child_size,
				int point_mut_count,
				int copy_mut_count,
				int insert_mut_count,
				int point_mut_line_count,
				int copy_mut_line_count,
				int delete_mut_count,
				int divide_mut_count,
				int divide_insert_mut_count,
				int divide_delete_mut_count,
				int copied_size,
				int executed_size,
				int copies_exec ){
  if( fp_phylogeny_log.good() )
    fp_phylogeny_log<<cStats::GetUpdate()<<" "
		    <<genotype_num<<" "
		    <<parent_id<<" "
		    <<parent_dist<<endl;
  if( cConfig::GetLogMutations() ){
    ofstream & fp = GetDataFileOFStream("mutations.log");
    fp<< GetUpdate()               <<" " // 1
      << parent_dist               <<" " // 2
      << parent_size               <<" " // 3
      << child_size                <<" " // 4
      << copied_size               <<" " // 5
      << executed_size             <<" " // 6
      << copies_exec               <<" " // 7
      << point_mut_count           <<" " // 8
      << copy_mut_count            <<" " // 9
      << insert_mut_count          <<" " // 10
      << point_mut_line_count      <<" " // 11
      << copy_mut_line_count       <<" " // 12
      << delete_mut_count          <<" " // 13
      << divide_mut_count          <<" " // 14
      << divide_insert_mut_count   <<" " // 15
      << divide_delete_mut_count <<endl; // 16
  }
}





inline void cStats::RemoveCreature(int genotype_num, int num_divides, int age){
  if( fp_breed_log.good() )
    fp_breed_log<<cStats::GetUpdate()<<" "
		<<genotype_num<<" "
		<<num_divides<<" "
		<<age<<endl;
}

inline void cStats::AddGenotype(int id_num) {
  id_num = -1;  // @CAO do we still need id_num here?
  tot_genotypes++;
}

inline void cStats::RemoveGenotype(int id_num, int parent_id,
   int parent_dist, int depth, int max_abundance, int parasite_abundance,
   int age, int length) {
  if (fp_genotype_log.good() &&
      (!cConfig::GetLogThresholdOnly() || max_abundance > 2))
    fp_genotype_log << id_num                        << " "    //  1
		    << cStats::GetUpdate() - age + 1 << " "    //  2

		    << parent_id                     << " "    //  3
		    << parent_dist                   << " "    //  4
		    << depth                         << " "    //  5

		    << max_abundance                 << " "    //  6
		    << age                           << " "    //  7
		    << length                        << endl;  //  8

  (void) parasite_abundance; // Not used now, but maybe in future.
}

inline void cStats::AddThreshold(int id_num, const char * name, int species_num)
{
  num_threshold++;
  tot_threshold++;
  if( fp_threshold_log.good() )
    fp_threshold_log<<cStats::GetUpdate()<<" "
		    <<id_num<<" "
		    <<species_num<<" "
		    <<name<<endl;
}

inline void cStats::RemoveThreshold(int id_num) {
  id_num = -1;  // @CAO do we still need id_num here?
  num_threshold--;
}

inline void cStats::AddSpecies(int id_num) {
  id_num = -1; // @CAO do we still need id_num here?
  tot_species++;
  num_species++;
}

inline void cStats::RemoveSpecies(int id_num, int parent_id,
			   int max_gen_abundance, int max_abundance, int age) {
  num_species--;
  if( fp_species_log.good() )
    fp_species_log<<cStats::GetUpdate()<<" "
		  <<id_num<<" "
		  <<parent_id<<" "
		  <<max_gen_abundance<<" "
		  <<max_abundance<<" "
		  <<age<<endl;
}


inline void cStats::SetTasks(int task_num, int in_tasks)
{
#ifdef DEBUG
  if (task_num > cConfig::GetNumTasks()) {
    g_debug.Error("Recording non-existant task!\n");
  }
#endif

  task_count[task_num] = in_tasks;
}

inline void cStats::AddTasks(int task_num, int in_tasks){
  assert( task_num >= 0  &&  task_num < cConfig::GetNumTasks() );
  task_count[task_num] ++;
  // task_exe_count[task_num] += in_tasks; // This is done in tasks
}

inline void cStats::SetNumGenotypes(int new_genotypes)
{
  num_genotypes_last = num_genotypes;
  num_genotypes = new_genotypes;
}

inline void cStats::SetNumCreatures(int new_creatures)
{
  num_creatures = new_creatures;
}

inline void cStats::SetNumThreshSpecies(int new_thresh_species)
{
  num_thresh_species = new_thresh_species;
}

inline void cStats::SetBreedTrueCreatures(int in_num_breed_true_creatures)
{
  num_breed_true_creatures = in_num_breed_true_creatures;
}

inline void cStats::SetNumParasites(int in_num_parasites)
{
  num_parasites = in_num_parasites;
}

inline void cStats::SetNumNoBirthCreatures(int in_num_no_birth_creatures){
  num_no_birth_creatures = in_num_no_birth_creatures;
}

#endif
