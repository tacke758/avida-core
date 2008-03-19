//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_HH
#define CONFIG_HH

//#include <fstream.h>
//#include <iostream.h>
// porting to gcc 3.1 -- k
#include <fstream>
#include <iostream>

#include "../tools/file.hh"
#include "../defs.hh"

#define SLICE_CONSTANT         0
#define SLICE_PROB_MERIT       1
#define SLICE_INTEGRATED_MERIT 2

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

#define GENERATION_INC_OFFSPRING 0
#define GENERATION_INC_BOTH      1

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
/*
switched from private to protected so that I can try to subclass
cConfig.
-- kaben.
*/
//private:
protected:
/**/

  // Major Configurations
  static bool analyze_mode;     // Should avida do only analysis work?
  static bool primitive_mode;   // Should avida run in primitive mode?

  // Config Filenames
  static cString default_dir;
  static cString genesis_filename;
  static cString inst_filename;
  static cString event_filename;
  static cString analyze_filename;
  static cString env_filename;

  static cString start_creature;
  static cString clone_filename;
  static cString load_pop_filename;

  // Archetecture
  static int max_updates;
  static int max_generations;
  static int end_condition_mode;
  static int world_x;
  static int world_y;
  static int rand_seed;

  // Mutations
  static double point_mut_prob;
  static double copy_mut_prob;
  static double ins_mut_prob;
  static double del_mut_prob;
  static double div_mut_prob;
  static double divide_mut_prob;
  static double divide_ins_prob;
  static double divide_del_prob;
  static double parent_mut_prob;
  static double crossover_prob;
  static double aligned_cross_prob;
  static double exe_err_prob;

  // CPU Configutation
  static int num_instructions;
  static int max_cpu_threads;
  static int thread_slicing_method;

  // Merit info
  static int size_merit_method;
  static int base_size_merit;
  static int ave_time_slice;

  // Task info
  static int num_tasks;
  static int num_reactions;
  static int num_resources;

  // Task Merit Method
  static int task_merit_method;
  static int max_num_tasks_rewarded;

  static int merit_time;

  // Methodology
  static int slicing_method;
  static int birth_method;
  static int death_method;
  static int alloc_method;
  static int divide_method;
  static int required_task;
  static int lineage_creation_method;
  static int generation_inc_method;
  static int age_limit;
  static double age_deviation;
  static double child_size_range;
  static double min_copied_lines;
  static double min_exe_lines;
  static int require_allocate;
  static int max_label_exe_size;

  // For specialized experiments -- mutation reversion.
  static bool test_on_divide;
  static double revert_fatal;
  static double revert_neg;
  static double revert_neut;
  static double revert_pos;
  static double sterilize_fatal;
  static double sterilize_neg;
  static double sterilize_neut;
  static double sterilize_pos;
  static int fail_implicit;

  // Geneology & Recording
  static int species_threshold;
  static int threshold;
  static int genotype_print;
  static int species_print;
  static int species_recording;
  static int genotype_print_dom;
  static int test_cpu_time_mod;
  static int track_main_lineage;

  // Log files...
  static bool log_threshold_only;

  static bool log_breed_count;
  static bool log_creatures;
  static bool log_phylogeny;
  static bool log_genotypes;
  static bool log_threshold;
  static bool log_species;
  static bool log_landscape;
  static bool log_mutations;
  static bool log_lineages;

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

  // ``Get''
  static bool GetAnalyzeMode() { return analyze_mode; }
  static bool GetPrimitiveMode() { return primitive_mode; }

  /*
  addition to permit access to cConfig::default_dir for use in locating
  preset_organisms subdirectory (accessed in godbox widget).
  -- kaben.
  */
  static cString & GetDefaultDir() { return default_dir; }
  /* -- kaben. */
  
  static cString & GetGenesisFilename() { return genesis_filename; }
  static cString & GetInstFilename()  { return inst_filename; }
  static cString & GetEventFilename() { return event_filename; }
  static cString & GetAnalyzeFilename() { return analyze_filename; }
  static cString & GetEnvironmentFilename() { return env_filename; }

  static cString & GetStartCreature() { return start_creature; }
  static cString & GetCloneFilename() { return clone_filename; }
  static cString & GetLoadPopFilename() { return load_pop_filename; }

  static int GetMaxUpdates()     { return max_updates; }
  static int GetMaxGenerations() { return max_generations; }
  static int GetEndConditionMode() { return end_condition_mode; }
  static int GetWorldX()         { return world_x; }
  static int GetWorldY()         { return world_y; }
  static int GetRandSeed()       { return rand_seed; }

  static double GetPointMutProb()  { return point_mut_prob; }
  static double GetCopyMutProb()   { return copy_mut_prob; }
  static double GetInsMutProb()    { return ins_mut_prob; }
  static double GetDelMutProb()    { return del_mut_prob; }
  static double GetDivMutProb()    { return div_mut_prob; }
  static double GetDivideMutProb() { return divide_mut_prob; }
  static double GetDivideInsProb() { return divide_ins_prob; }
  static double GetDivideDelProb() { return divide_del_prob; }
  static double GetParentMutProb() { return parent_mut_prob; }
  static double GetCrossoverProb() { return crossover_prob; }
  static double GetAlignedCrossProb() { return aligned_cross_prob; }
  static double GetExeErrProb()    { return exe_err_prob; }

  static int GetNumInstructions() { return num_instructions; }
  static int GetMaxCPUThreads()  { return max_cpu_threads; }
  static int GetThreadSlicingMethod() { return thread_slicing_method; }

  static int GetSizeMeritMethod() { return size_merit_method; }
  static int GetBaseSizeMerit()  { return base_size_merit; }
  static int GetTaskMeritMethod() { return task_merit_method; }
  static int GetMaxNumTasksRewarded() { return max_num_tasks_rewarded; }
  static int GetMaxLabelExeSize() { return max_label_exe_size; }

  static int GetMeritTime() { return merit_time; }

  static int GetNumTasks() { return num_tasks; }
  static int GetNumReactions() { return num_reactions; }
  static int GetNumResources() { return num_resources; }

  static int GetSlicingMethod() { return slicing_method; }
  static int GetBirthMethod() { return birth_method; }
  static int GetDeathMethod() { return death_method; }
  static int GetAllocMethod() { return alloc_method; }
  static int GetDivideMethod() { return divide_method; }
  static int GetRequiredTask() { return required_task; }
  static int GetLineageCreationMethod() { return lineage_creation_method; }
  static int GetGenerationIncMethod() { return generation_inc_method; }
  static int GetAgeLimit() { return age_limit; }
  static double GetAgeDeviation() { return age_deviation; }
  static double GetChildSizeRange() { return child_size_range; }
  static double GetMinCopiedLines() { return min_copied_lines; }
  static double GetMinExeLines() { return min_exe_lines; }
  static int GetRequireAllocate() { return require_allocate; }

  static bool GetTestOnDivide() { return test_on_divide; }
  static double GetRevertFatal() { return revert_fatal; }
  static double GetRevertNeg() { return revert_neg; }
  static double GetRevertNeut() { return revert_neut; }
  static double GetRevertPos() { return revert_pos; }
  static double GetSterilizeFatal() { return sterilize_fatal; }
  static double GetSterilizeNeg() { return sterilize_neg; }
  static double GetSterilizeNeut() { return sterilize_neut; }
  static double GetSterilizePos() { return sterilize_pos; }
  static int GetFailImplicit() { return fail_implicit; }


  static int GetAveTimeslice() { return ave_time_slice; }
  static int GetSpeciesThreshold() { return species_threshold; }
  static int GetThreshold() { return threshold; }
  static int GetGenotypePrint() { return genotype_print; }
  static int GetSpeciesPrint() { return species_print; }
  static int GetSpeciesRecording() { return species_recording; }
  static int GetGenotypePrintDom() { return genotype_print_dom; }
  static int GetTestCPUTimeMod() { return test_cpu_time_mod; }
  static int GetTrackMainLineage() { return track_main_lineage; }


  static bool GetLogThresholdOnly()  { return log_threshold_only; }

  static bool GetLogBreedCount() { return log_breed_count; }
  static bool GetLogCreatures() { return log_creatures; }
  static bool GetLogPhylogeny() { return log_phylogeny; }
  static bool GetLogGenotypes() { return log_genotypes; }
  static bool GetLogThreshold() { return log_threshold; }
  static bool GetLogSpecies()   { return log_species; }
  static bool GetLogLandscape() { return log_landscape; }
  static bool GetLogMutations() { return log_mutations; }
  static bool GetLogLineages()  { return log_lineages; }

  static int GetDebugLevel() { return debug_level; }

  static int GetViewMode() { return view_mode; }
  static int GetViewerType() { return viewer_type; }


  // ``Set''
  static void SetInstFilename(const cString & in_name)
    { inst_filename = in_name; }
  static void SetEventFilename(const cString & in_name)
    { event_filename = in_name; }
  static void SetAnalyzeFilename(const cString & in_name)
    { analyze_filename = in_name; }
  static void SetEnvironmentFilename(const cString & in_name)
    { env_filename = in_name; }

  static void SetMaxUpdates(int in_max_updates)
    { max_updates = in_max_updates; }
  static void SetMaxGenerations(int _in)
    { max_generations = _in; }
  static void SetEndConditionMode(int _in)
    { end_condition_mode = _in; }

  static void SetCopyMutProb(double _in)  { copy_mut_prob = _in; }
  static void SetDivMutProb(double _in)  { div_mut_prob = _in; }
  static void SetPointMutProb(double _in) { point_mut_prob = _in; }

  /*
  addition to permit tweaking of ins_mut_prob, del_mut_prob from godbox
  widget.
  -- kaben.
  */
  static void SetInsMutProb(double _in)  { ins_mut_prob = _in; }
  static void SetDelMutProb(double _in) { del_mut_prob = _in; }
  /* -- kaben. */

  static void SetNumInstructions(int in_num_instructions)
    { num_instructions = in_num_instructions; }
  static void SetMaxCPUThreads(int in_threads)
    { max_cpu_threads = in_threads; }

  static void SetSizeMeritMethod(int in_merit_method)
    { size_merit_method = in_merit_method; }
  static void SetBaseSizeMerit(int in_base_size_merit)
    { base_size_merit = in_base_size_merit; }

  static void SetBirthMethod(int in_birth_method)
    { birth_method = in_birth_method; }
  static void SetDeathMethod(int in_death_method)
    { death_method = in_death_method; }
  static void SetAllocMethod(int in_alloc_method)
    { alloc_method = in_alloc_method; }
  static void SetDivideMethod(int in_divide_method)
    { divide_method = in_divide_method; }
  static void SetLineageCreationMethod( int in_lineage_creation_method )
    { lineage_creation_method = in_lineage_creation_method; }
  static void SetGenerationIncMethod(int in_generation_inc_method)
    { generation_inc_method = in_generation_inc_method; }
  static void SetAgeLimit(int in_age_limit) { age_limit = in_age_limit; }
  static void SetAgeDeviation(double in_dev) { age_deviation = in_dev; }

  static void SetAveTimeslice(int in_slice) { ave_time_slice = in_slice; }

  static void SetNumTasks(int in_num) { num_tasks = in_num; }
  static void SetNumReactions(int in_num) { num_reactions = in_num; }
  static void SetNumResources(int in_num) { num_resources = in_num; }
};


#endif // #ifndef CONFIG_HH
