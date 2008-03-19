//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_HH
#define CONFIG_HH

#include <fstream.h>
#include <iostream.h>
#include "../defs.hh"
#include "../tools/tools.hh"
#include "tasks.hh"


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
#define ALLOC_METHOD_RANDOM    2

#define DIVIDE_METHOD_OFFSPRING 0
#define DIVIDE_METHOD_SPLIT     1

#define GENERATION_COUNT_METHOD_OFFSPRING 0
#define GENERATION_COUNT_METHOD_BOTH      1

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
  static cString resource_filename;
  static cString event_filename;
  static cString start_creature;
  static cString clone_filename;
  static cString load_pop_filename;

  // Archetecture
  static int max_updates;
  static int max_generations;
  static int end_condition_mode;
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
  static double crossover_prob;
  static double aligned_cross_prob;
  static double exe_err_prob;

  // CPU Configutation
  static int num_instructions;
  static int max_cpu_threads;

  // Merit info
  static int size_merit_method;
  static int base_size_merit;
  static int ave_time_slice;

  // Task info
  static cTaskLib default_task_lib;
  static int num_tasks;

  // Task Merit Method
  static int task_merit_method;
  static int max_num_tasks_rewarded;

  // Resources
  static int resources;
  static int merit_time;

  // Methodology
  static int slicing_method;
  static int birth_method;
  static int death_method;
  static int alloc_method;
  static int divide_method;
  static int generation_count_method;
  static int age_limit;
  static double child_size_range;
  static double min_copied_lines;
  static double min_exe_lines;

  // Geneology & Recording
  static int species_threshold;
  static int threshold;
  static int genotype_print;
  static int species_print;
  static int species_recording;
  static int genotype_print_dom;
  static int test_cpu_time_mod;

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
  static int log_mutations;

  // Debugging
  static int debug_level;

  // Viewer
  static int view_mode;
  static int viewer_type;

  // Other functions...
  static void ProcessConfiguration(int argc, char* argv[], cGenesis & genesis);
public:
  static void Setup(int argc, char * argv[]);
  static void SetupMS();

  static inline void LoadTaskSet(){
    default_task_lib.Load(cConfig::GetTaskFilename());
    //@TCC Move
    if( cConfig::GetResources() == 1 ){
      default_task_lib.LoadResources(cConfig::GetResourceFilename());
    }
    num_tasks = default_task_lib.GetNumTasks();
  }
  static inline void PerturbTaskSet(double max_factor) {
    default_task_lib.Perturb(max_factor);
  }
  static inline void ChangeTaskSet(double prob_change, double min_bonus,
				   double max_bonus) {
    default_task_lib.Change(prob_change, min_bonus, max_bonus);
  }

  // ``Get''
  static inline cString & GetInstFilename()  { return inst_filename; }
  static inline cString & GetTaskFilename()  { return task_filename; }
  static inline cString & GetResourceFilename()  { return resource_filename; }
  static inline cString & GetEventFilename() { return event_filename; }
  static inline cString & GetStartCreature() { return start_creature; }
  static inline cString & GetCloneFilename() { return clone_filename; }
  static inline cString & GetLoadPopFilename() { return load_pop_filename; }

  static inline int GetMaxUpdates()     { return max_updates; }
  static inline int GetMaxGenerations() { return max_generations; }
  static inline int GetEndConditionMode() { return end_condition_mode; }
  static inline int GetWorldX()         { return world_x; }
  static inline int GetWorldY()         { return world_y; }

  static inline double GetPointMutProb()  { return point_mut_prob; }
  static inline double GetCopyMutProb()   { return copy_mut_prob; }
  static inline double GetInsMutProb()    { return ins_mut_prob; }
  static inline double GetDelMutProb()    { return del_mut_prob; }
  static inline double GetDivideMutProb() { return divide_mut_prob; }
  static inline double GetDivideInsProb() { return divide_ins_prob; }
  static inline double GetDivideDelProb() { return divide_del_prob; }
  static inline double GetCrossoverProb() { return crossover_prob; }
  static inline double GetAlignedCrossProb() { return aligned_cross_prob; }
  static inline double GetExeErrProb()    { return exe_err_prob; }

  static inline int GetNumInstructions() { return num_instructions; }
  static inline int GetMaxCPUThreads()  { return max_cpu_threads; }

  static inline int GetSizeMeritMethod() { return size_merit_method; }
  static inline int GetBaseSizeMerit()  { return base_size_merit; }
  static inline int GetTaskMeritMethod() { return task_merit_method; }
  static inline int GetMaxNumTasksRewarded() { return max_num_tasks_rewarded; }

  static inline int GetResources() { return resources; }
  static inline int GetMeritTime() { return merit_time; }

  static cTaskLib * GetDefaultTaskLib() { return &default_task_lib; }
  static inline int GetNumTasks() { return num_tasks; }

  static inline int GetSlicingMethod() { return slicing_method; }
  static inline int GetBirthMethod() { return birth_method; }
  static inline int GetDeathMethod() { return death_method; }
  static inline int GetAllocMethod() { return alloc_method; }
  static inline int GetDivideMethod() { return divide_method; }
  static inline int GetGenerationCountMethod() { return generation_count_method; }
  static inline int GetAgeLimit() { return age_limit; }
  static inline double GetChildSizeRange() { return child_size_range; }
  static inline double GetMinCopiedLines() { return min_copied_lines; }
  static inline double GetMinExeLines() { return min_exe_lines; }

  static inline int GetAveTimeslice() { return ave_time_slice; }
  static inline int GetSpeciesThreshold() { return species_threshold; }
  static inline int GetThreshold() { return threshold; }
  static inline int GetGenotypePrint() { return genotype_print; }
  static inline int GetSpeciesPrint() { return species_print; }
  static inline int GetSpeciesRecording() { return species_recording; }
  static inline int GetGenotypePrintDom() { return genotype_print_dom; }
  static inline int GetTestCPUTimeMod() { return test_cpu_time_mod; }


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
  static inline int GetLogMutations() { return log_mutations; }


  static inline int GetDebugLevel() { return debug_level; }

  static inline int GetViewMode() { return view_mode; }
  static inline int GetViewerType() { return viewer_type; }


  // ``Set''
  static inline void SetInstFilename(const cString & in_name)
    { inst_filename = in_name; }
  static inline void SetTaskFilename(const cString & in_name)
    { task_filename = in_name; }
  static inline void SetResourceFilename(const cString & in_name)
    { resource_filename = in_name; }
  static inline void SetEventFilename(const cString & in_name)
    { event_filename = in_name; }

  static inline void SetMaxUpdates(int in_max_updates)
    { max_updates = in_max_updates; }
  static inline void SetMaxGenerations(int _in)
    { max_generations = _in; }
  static inline void SetEndConditionMode(int _in)
    { end_condition_mode = _in; }

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
  static inline void SetDivideMethod(int in_divide_method)
    { divide_method = in_divide_method; }
  static inline void SetGenerationCoundMethod(int in_generation_count_method)
    { generation_count_method = in_generation_count_method; }
  static inline void SetAgeLimit(int in_age_limit)
    { age_limit = in_age_limit; }

  static inline void SetAveTimeslice(int in_slice)
    { ave_time_slice = in_slice; }

};


#endif // #ifndef CONFIG_HH
