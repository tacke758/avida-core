//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef STATS_HH
#define STATS_HH

#include <fstream.h>
#include <iostream.h>
#include "../tools/tools.hh"
#include "../tools/datafile.hh"
#include "merit.hh"

class cDataFile;
class cTaskLib; // @TCC remove when remove default task lib


#define SLICE_CONSTANT         0
#define SLICE_BLOCK_MERIT      1
#define SLICE_PROB_MERIT       2
#define SLICE_INTEGRATED_MERIT 3
#define SLICE_LOGRITHMIC_MERIT 4

#define POSITION_CHILD_RANDOM           0
#define POSITION_CHILD_AGE              1
#define POSITION_CHILD_MERIT            2
#define POSITION_CHILD_EMPTY            3
#define NUM_LOCAL_POSITION_CHILD        4

#define POSITION_CHILD_FULL_SOUP_RANDOM 4
#define POSITION_CHILD_FULL_SOUP_ELDEST 5

#define DEATH_METHOD_OFF       0
#define DEATH_METHOD_CONST     1
#define DEATH_METHOD_MULTIPLE  2

#define ALLOC_METHOD_DEFAULT   0
#define ALLOC_METHOD_NECRO     1

#define TASK_MERIT_OFF         0
#define TASK_MERIT_EXPONENTIAL 1
#define TASK_MERIT_LINEAR      2

#define SIZE_MERIT_OFF         0
#define SIZE_MERIT_COPIED      1
#define SIZE_MERIT_EXECUTED    2
#define SIZE_MERIT_FULL        3
#define SIZE_MERIT_LEAST       4
#define SIZE_MERIT_SQRT_LEAST  5

class cConfig {
private:
  // Config Filenames
  static cString inst_filename;
  static cString task_filename;
  static cString event_filename;
  static cString start_creature;
  static cString clone_filename;

  // Archetecture
  static int max_updates;
  static int max_generations;
  static int world_x;
  static int world_y;

  // Mutations
  static double point_mut_prob;
  static double copy_mut_prob;
  static double ins_mut_prob;
  static double del_mut_prob;
  static double divide_mut_prob;
  static double divide_ins_prob;
  static double divide_del_prob;

  // CPU Configutation
  static int num_instructions;
  static int max_cpu_threads;

  // Merit info
  static int size_merit_method;
  static int base_size_merit;

  // Task info
  static cTaskLib default_task_lib;
  static int num_tasks;

  // Task Merit Method
  static int task_merit_method;
  static int max_num_tasks_rewarded;

  // Methodology
  static int slicing_method;
  static int birth_method;
  static int death_method;
  static int alloc_method;
  static int age_limit;
  static double child_size_range;
  static double min_copied_lines;
  static double min_exe_lines;  
  static int ave_time_slice;

  // Geneology
  static int species_threshold;
  static int threshold;
  static int genotype_print;
  static int species_print;
  static int species_recording;
  static int genotype_print_dom;


  // Output files...
  static int average_data_interval;
  static int dominant_data_interval;
  static int count_data_interval;
  static int totals_data_interval;
  static int tasks_data_interval;
  static int stats_data_interval;
  static int time_data_interval;

  static int genotype_status_interval;
  static int diversity_status_interval;

  static int log_threshold_only;
  static int log_geneology_leaves;

  static int log_breed_count;
  static int log_creatures;
  static int log_phylogeny;
  static int log_geneology;
  static int log_genotypes;
  static int log_threshold;
  static int log_species;
  static int log_landscape;

  // Debugging
  static int debug_level;

  // Viewer
  static int view_mode;

  // Other functions...
  static void ProcessConfiguration(int argc, char* argv[], cGenesis & genesis);
public:
  static void Setup(int argc, char * argv[]);
  static void SetupMS();

  // ``Get''
  static inline cString & GetInstFilename()  { return inst_filename; }
  static inline cString & GetTaskFilename()  { return task_filename; }
  static inline cString & GetEventFilename() { return event_filename; }
  static inline cString & GetStartCreature() { return start_creature; }
  static inline cString & GetCloneFilename() { return clone_filename; }

  static inline int GetMaxUpdates()     { return max_updates; }
  static inline int GetMaxGenerations() { return max_generations; }
  static inline int GetWorldX()         { return world_x; }
  static inline int GetWorldY()         { return world_y; }

  static inline double GetPointMutProb()  { return point_mut_prob; }
  static inline double GetCopyMutProb()   { return copy_mut_prob; }
  static inline double GetInsMutProb()    { return ins_mut_prob; }
  static inline double GetDelMutProb()    { return del_mut_prob; }
  static inline double GetDivideMutProb() { return divide_mut_prob; }
  static inline double GetDivideInsProb() { return divide_ins_prob; }
  static inline double GetDivideDelProb() { return divide_del_prob; }

  static inline int GetNumInstructions() { return num_instructions; }
  static inline int GetMaxCPUThreads()  { return max_cpu_threads; }

  static inline int GetSizeMeritMethod() { return size_merit_method; }
  static inline int GetBaseSizeMerit()  { return base_size_merit; }
  static inline int GetTaskMeritMethod() { return task_merit_method; }
  static inline int GetMaxNumTasksRewarded() { return max_num_tasks_rewarded; }

  static const cTaskLib * GetDefaultTaskLib() { return &default_task_lib; }
  static inline int GetNumTasks() { return num_tasks; }

  static inline int GetSlicingMethod() { return slicing_method; }
  static inline int GetBirthMethod() { return birth_method; }
  static inline int GetDeathMethod() { return death_method; }
  static inline int GetAllocMethod() { return alloc_method; }
  static inline int GetAgeLimit() { return age_limit; }
  static inline double GetChildSizeRange() { return child_size_range; }
   

  static inline int GetAveTimeslice() { return ave_time_slice; }
  static inline int GetSpeciesThreshold() { return species_threshold; }
  static inline int GetThreshold() { return threshold; }
  static inline int GetGenotypePrint() { return genotype_print; }
  static inline int GetSpeciesPrint() { return species_print; }
  static inline int GetSpeciesRecording() { return species_recording; }
  static inline int GetGenotypePrintDom() { return genotype_print_dom; }


  static inline int GetAverageDataInterval()
    { return average_data_interval; }
  static inline int GetDominantDataInterval()
    { return dominant_data_interval; }
  static inline int GetCountDataInterval()
    { return count_data_interval; }
  static inline int GetTotalsDataInterval()
    { return totals_data_interval; }
  static inline int GetTasksDataInterval()
    { return tasks_data_interval; }
  static inline int GetStatsDataInterval()
    { return stats_data_interval; }
  static inline int GetTimeDataInterval()
    { return time_data_interval; }

  static inline int GetGenotypeStatusInterval()
    { return genotype_status_interval; }
  static inline int GetDiversityStatusInterval()
    { return diversity_status_interval; }

  static inline int GetLogThresholdOnly()  { return log_threshold_only; }
#ifdef GENEOLOGY
  static inline int GetLogGeneologyLeaves(){ return log_geneology_leaves; }
#endif

  static inline int GetLogBreedCount() { return log_breed_count; }
  static inline int GetLogCreatures() { return log_creatures; }
  static inline int GetLogPhylogeny() { return log_phylogeny; }
  static inline int GetLogGeneology() { return log_geneology; }
  static inline int GetLogGenotypes() { return log_genotypes; }
  static inline int GetLogThreshold() { return log_threshold; }
  static inline int GetLogSpecies()   { return log_species; }
  static inline int GetLogLandscape() { return log_landscape; }


  static inline int GetDebugLevel() { return debug_level; }
  static inline int GetViewMode() { return view_mode; }


  // ``Set''
  static inline void SetInstFilename(const cString & in_name)
    { inst_filename = in_name; }
  static inline void SetTaskFilename(const cString & in_name)
    { task_filename = in_name; }
  static inline void SetEventFilename(const cString & in_name)
    { event_filename = in_name; }

  static inline void SetMaxUpdates(int in_max_updates)
    { max_updates = in_max_updates; }
  static inline void SetMaxGenerations(int _in)
    { max_generations = _in; }

  static inline void SetCopyMutProb(double _in)  { copy_mut_prob = _in; }
  static inline void SetPointMutProb(double _in) { point_mut_prob = _in; }

  static inline void SetNumInstructions(int in_num_instructions)
    { num_instructions = in_num_instructions; }
  static inline void SetMaxCPUThreads(int in_threads)
    { max_cpu_threads = in_threads; }

  static inline void SetSizeMeritMethod(int in_merit_method)
    { size_merit_method = in_merit_method; }
  static inline void SetBaseSizeMerit(int in_base_size_merit)
    { base_size_merit = in_base_size_merit; }

  static inline void SetBirthMethod(int in_birth_method)
    { birth_method = in_birth_method; }
  static inline void SetDeathMethod(int in_death_method)
    { death_method = in_death_method; }
  static inline void SetAllocMethod(int in_alloc_method)
    { alloc_method = in_alloc_method; }
  static inline void SetAgeLimit(int in_age_limit)
    { age_limit = in_age_limit; }

  static inline void SetAveTimeslice(int in_slice)
    { ave_time_slice = in_slice; }

};



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
  static cRunningSum sum_repro_rate;


  //// By Creature Sums ////  (Cleared and resummed by population each update)

  static cRunningSum sum_merit;
  static cRunningSum sum_mem_size;
  static cRunningSum sum_creature_age;
  static cRunningSum sum_generation;
  static cRunningSum sum_num_threads;
  static cRunningSum sum_thread_dist;

  static cRunningSum sum_neutral_metric;
  static cRunningSum sum_lineage_label;


  //// By Genotype Sums ////  (Cleared and resummed by population each update)

  // Merit, Gestation, &  Fitness
  static cRunningSum sum_gestation;
  static cRunningSum sum_fitness;

  // Sizes
  static cRunningSum sum_size;
  static cRunningSum sum_copy_size;
  static cRunningSum sum_exe_size;

  // Age
  static cRunningSum sum_genotype_age;

  // breed/geneological stats
  static cRunningSum sum_abundance;
  static cRunningSum sum_genotype_depth;


  //// By Threshold Sums ////  (Cleared and resummed by population each update)

  static cRunningSum sum_threshold_age;


  //// By Species Sums ////  (Cleared and resummed by population each update)

  static cRunningSum sum_species_age;


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

  static int num_genotypes_last;

  static int tot_creatures;
  static int tot_genotypes;
  static int tot_threshold;
  static int tot_species;
  static int tot_thresh_species;
  static int tot_executed;

  static int * task_count;

public:
  static void Setup();
  static void Shutdown();

  static void ProcessUpdate(int new_update);

  // Specific event-triggered files.

  static void TestThreads(const cCodeArray & code);


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

  // LOCAL STATS

  static int OK() { return TRUE; }  // @CAO FIX NOW!!!!

  static inline ofstream & GetGenotypeStatusFP()
    { return fp_genotype_status; }
  static inline ofstream & GetDiversityStatusFP()
    { return fp_diversity_status; }

  static void FlushFP(); // Flush all the files

  // World data

  // Generic data

  static void SetBestGenotype(cGenotype * best_genotype);

  static inline void SetNumGenotypes(int new_genotypes);
  static inline void SetNumCreatures(int new_creatures);
  static inline void SetNumThreshSpecies(int new_thresh_species);
  static inline void SetBreedTrueCreatures(int in_num_breed_true_creatures);
  static inline void SetNumParasites(int in_num_parasites);

  static inline void SetMaxFitness(double in_max_fitness)
    { max_fitness = in_max_fitness; }
  static inline void SetMaxMerit(double in_max_merit)
    { max_merit = in_max_merit; }
  static inline void SetEntropy(double in_entropy)
    { entropy = in_entropy; }
  static inline void SetSpeciesEntropy(double in_ent)
    { species_entropy = in_ent; }

  static inline cRunningSum & SumReproRate()     { return sum_repro_rate; }

  static inline cRunningSum & SumMerit()         { return sum_merit; }
  static inline cRunningSum & SumCreatureAge()   { return sum_creature_age; }
  static inline cRunningSum & SumGeneration()    { return sum_generation; }
  static inline cRunningSum & SumNumThreads()    { return sum_num_threads; }
  static inline cRunningSum & SumThreadDist()    { return sum_thread_dist; }
  static inline cRunningSum & SumMemSize()       { return sum_mem_size; }

  static inline cRunningSum & SumNeutralMetric() { return sum_neutral_metric; }
  static inline cRunningSum & SumLineageLabel()  { return sum_lineage_label; }

  static inline cRunningSum & SumGestation()     { return sum_gestation; }
  static inline cRunningSum & SumFitness()       { return sum_fitness; }

  static inline cRunningSum & SumGenotypeAge()   { return sum_genotype_age; }

  static inline cRunningSum & SumSize()          { return sum_size; }
  static inline cRunningSum & SumCopySize()      { return sum_copy_size; }
  static inline cRunningSum & SumExeSize()       { return sum_exe_size; }

  static inline cRunningSum & SumAbundance()     { return sum_abundance; }
  static inline cRunningSum & SumGenotypeDepth() { return sum_genotype_depth; }

  static inline cRunningSum & SumThresholdAge()  { return sum_threshold_age; }
  static inline cRunningSum & SumSpeciesAge()    { return sum_species_age; }

  static inline void AddBirth() { num_births++; }
  static inline void AddDeath() { num_deaths++; }
  static inline void AddBreedIn() { num_births++; }
  static inline void AddBreedTrue() { num_breed_true++; }

  static inline void CalcEnergy();
  static inline void CalcFidelity();

  static inline void AddCreature(int id_num, int genotype_num);
  static inline void NewCreature(int genotype_id, int parent_id,
				 int parent_dist);
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

  static inline void SetGets(int in_gets);
  static inline void SetPuts(int in_puts);
  static inline void SetGGPs(int in_ggps);
  static inline void SetTasks(int task_num, int in_tasks);
  static inline void AddTasks(int task_num, int in_tasks);
  static        void ZeroTasks();

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

  static inline int GetTotCreatures()     { return tot_creatures; }
  static inline int GetTotGenotypes()     { return tot_genotypes; }
  static inline int GetTotThreshold()     { return tot_threshold; }
  static inline int GetTotSpecies()       { return tot_species; }
  static inline int GetTotThreshSpecies() { return tot_thresh_species; }

  static inline int GetTaskCount(int task_num) { return task_count[task_num]; }

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


  // Public calls to output data files (for events)
  static void PrintAverageData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputAverageData)).Output(); }
  static void PrintErrorData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputErrorData)).Output(); }
  static void PrintVarianceData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputVarianceData)).Output(); }
  static void PrintDominantData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputDominantData)).Output(); }
  static void PrintStatsData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputStatsData)).Output(); }
  static void PrintCountData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputCountData)).Output(); }
  static void PrintTotalsData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputTotalsData)).Output(); }
  static void PrintTasksData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputTasksData)).Output(); }
  static void PrintTimeData(const cString & filename){
    data_file_manager.Get(filename,&(cStats::OutputTimeData)).Output(); }


  // Actual output methods used by cDataFile & cDataFileManager
private:
  static void OutputAverageData(ofstream & fp);
  static void OutputErrorData(ofstream & fp);
  static void OutputVarianceData(ofstream & fp);
  static void OutputDominantData(ofstream & fp);
  static void OutputStatsData(ofstream & fp);
  static void OutputCountData(ofstream & fp);
  static void OutputTotalsData(ofstream & fp);
  static void OutputTasksData(ofstream & fp);
  static void OutputTimeData(ofstream & fp);
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
				     int parent_dist){
  if( fp_phylogeny_log.good() )
    fp_phylogeny_log<<cStats::GetUpdate()<<" "
		   <<genotype_num<<" "
		   <<parent_id<<" "
		   <<parent_dist<<endl;
}

inline void cStats::RemoveCreature(int genotype_num, int num_divides,
					int age) {
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
    fp_genotype_log<<cStats::GetUpdate()<<" "
		   <<id_num<<" "
		   <<parent_id<<" "
		   <<parent_dist<<" "
		   <<depth<<" "
		   <<max_abundance<<" "
		   <<parasite_abundance<<" "
		   <<age<<" "
		   <<length<<endl;
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
  task_count[task_num] += in_tasks;
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

#endif
