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
#include "merit.hh"

class cGlobalStats {
private:
  int num_populations;
  int num_instructions;
  int size_merit_method;
  int task_merit_method;
  int birth_method;
  int death_method;
  int age_limit;
  int ave_time_slice;

  int current_update;
  int sub_update;
  int max_updates;

  cString inst_filename;
  cString task_filename;
  cString event_filename;

  ofstream fp_test;
public:
  cGlobalStats();
  ~cGlobalStats();

  inline void SetNumPopulation(int in_num_populations);
  inline void SetNumInstructions(int in_num_instructions);
  inline void SetSizeMeritMethod(int in_merit_method);
  inline void SetTaskMeritMethod(int in_merit_method);
  inline void SetBirthMethod(int in_birth_method);
  inline void SetDeathMethod(int in_death_method);
  inline void SetAgeLimit(int in_age_limit);
  inline void SetAveTimeslice(int in_slice) { ave_time_slice = in_slice; }
  
  inline void SetCurrentUpdate(int new_update);
  inline void IncSubUpdate() { sub_update++; }
  inline void SetMaxUpdates(int in_max_updates);

  inline void SetInstFilename(const cString & in_name)
    { inst_filename = in_name; }
  inline void SetTaskFilename(const cString & in_name)
    { task_filename = in_name; }
  inline void SetEventFilename(const cString & in_name)
    { event_filename = in_name; }

  inline int GetNumPopulations() { return num_populations; }
  inline int GetNumInstructions() { return num_instructions; }
  inline int GetSizeMeritMethod() { return size_merit_method; }
  inline int GetTaskMeritMethod() { return task_merit_method; }
  inline int GetBirthMethod() { return birth_method; }
  inline int GetDeathMethod() { return death_method; }
  inline int GetAgeLimit() { return age_limit; }
  inline int GetAveTimeslice() { return ave_time_slice; }
  inline int GetUpdate() { return current_update; }
  inline int GetSubUpdate() { return sub_update; }
  inline int GetMaxUpdates() { return max_updates; }

  inline cString & GetInstFilename()  { return inst_filename; }
  inline cString & GetTaskFilename()  { return task_filename; }
  inline cString & GetEventFilename() { return event_filename; }

  //inline FILE * GetTestFP() { return fp_test; }
};

class cLocalStats {
private:
  // Output Files:  data files are taken every N updates and report the
  // current state of the soup in a rigid format.  Status files are also
  // printed every N updates, but are much more flexible in their format,
  // and must typically be processed more before they can be used.  Log files
  // are taken every time a specified event occurs.

  ofstream fp_average_data;
  ofstream fp_dominant_data;
  ofstream fp_count_data;
  ofstream fp_totals_data;
  ofstream fp_tasks_data;
  ofstream fp_stats_data;

  ofstream fp_genotype_status;
  ofstream fp_diversity_status;

  ofstream fp_creature_log;
  ofstream fp_phylogeny_log;
  ofstream fp_breed_log;
  ofstream fp_genotype_log;
  ofstream fp_threshold_log;
  ofstream fp_species_log;

  int average_data_interval;
  int dominant_data_interval;
  int count_data_interval;
  int totals_data_interval;
  int tasks_data_interval;
  int stats_data_interval;

  int genotype_status_interval;
  int diversity_status_interval;

  int log_threshold_only;

  // The world structure...

  int population_mode;
  int copy_mut_rate;
  int point_mut_rate;
  int divide_mut_rate;
  int divide_ins_rate;
  int divide_del_rate;

  // The statistics

  int current_update;
  cMerit ave_merit;
  double ave_gestation;
  double ave_repro_rate;
  double entropy;
  double species_entropy;
  double energy;
  double dom_fidelity;
  double ave_fidelity;
  double ave_fitness;
  double ave_size;
  double ave_copy_size;
  double ave_exe_size;
  double ave_mem_size;

  double ave_creature_age;
  double ave_genotype_age;
  double ave_threshold_age;
  double ave_species_age;

  cMerit best_merit;
  double best_gestation;
  double best_fitness;
  int best_size;
  double best_copied_size;
  double best_exe_size;
  double max_fitness;
  double max_merit;
  int best_genotype_id;
  cString best_name;
  int best_births;
  int best_abundance;

  int num_cells;
  int num_deaths;
  int num_creatures;
  int num_genotypes;
  int num_threshold;
  int num_species;
  int num_thresh_species;
  int num_breed_true;
  int num_executed;
  int num_parasites;

  int num_genotypes_last;

  int tot_creatures;
  int tot_genotypes;
  int tot_threshold;
  int tot_species;
  int tot_thresh_species;
  int tot_executed;

  int * task_count;
  int num_tasks;
  int get_count;
  int put_count;
  int ggp_count;
public:
  cLocalStats(const cGenesis & genesis);
  ~cLocalStats();

  int OK() { return TRUE; }  // @CAO FIX NOW!!!!
  void ProcessUpdate(int new_update);

  inline int GetGenotypeStatusInterval()  { return genotype_status_interval; }
  inline int GetDiversityStatusInterval() { return diversity_status_interval; }

  inline ofstream & GetGenotypeStatusFP()  { return fp_genotype_status; }
  inline ofstream & GetDiversityStatusFP() { return fp_diversity_status; }

  void FlushFP(); // Flush all the files

  // World data

  inline void SetPopulationMode(int in_population_mode);

  // Generic data

  void SetBestGenotype(cGenotype * best_genotype);

  inline void SetNumCells(int in_num_cells) { num_cells = in_num_cells; }
  inline void SetNumGenotypes(int new_genotypes);
  inline void SetNumCreatures(int new_creatures);
  inline void SetNumThreshSpecies(int new_thresh_species);
  inline void SetBreedTrue(int in_num_breed_true);
  inline void SetNumParasites(int in_num_parasites);
  inline void SetCopyMutRate(int in_cmut) { copy_mut_rate = in_cmut; }

  inline void SetMaxFitness(double in_max_fitness);
  inline void SetMaxMerit(double in_max_merit);
  inline void SetAveMerit(cMerit new_ave_merit);
  inline void SetAveGestation(double new_ave_gestation);
  inline void SetAveReproRate(double new_ave_repro_rate);
  inline void SetEntropy(double in_entropy) { entropy = in_entropy; }
  inline void SetSpeciesEntropy(double in_ent) { species_entropy = in_ent; }
  inline void SetAveFitness(double new_ave_fitness);
  inline void SetAveSize(double in_ave_size);
  inline void SetAveCopySize(double in_ave_copy_size);
  inline void SetAveExeSize(double in_ave_exe_size);
  inline void SetAveMemSize(double in_ave_mem_size);

  inline void SetAveCreatureAge(double in_age)  { ave_creature_age = in_age; }
  inline void SetAveGenotypeAge(double in_age)  { ave_genotype_age = in_age; }
  inline void SetAveThresholdAge(double in_age) { ave_threshold_age = in_age; }
  inline void SetAveSpeciesAge(double in_age)   { ave_species_age = in_age; }

  inline void AddDeath() { num_deaths++; }

  inline void CalcEnergy();
  inline void CalcFidelity();

  inline void AddCreature(int id_num, int genotype_num);
  inline void NewCreature(int genotype_id, int parent_id, int parent_dist);
  inline void RemoveCreature(int genotype_num, int num_divides, int age);
  inline void AddGenotype(int id_num);
  inline void RemoveGenotype(int id_num, int parent_id, int parent_distance, 
			     int max_abundance, int parasite_abundance, 
			     int age, int length);
  inline void AddThreshold(int id_num, char * name, int species_num=-1);
  inline void RemoveThreshold(int id_num);
  inline void AddSpecies(int id_num);
  inline void RemoveSpecies(int id_num, int parent_id, int max_gen_abundance, 
			    int max_abundance, int age);

  inline void AddExecuted(int in_executed);

  inline void SetGets(int in_gets);
  inline void SetPuts(int in_puts);
  inline void SetGGPs(int in_ggps);
  inline void SetTasks(int task_num, int in_tasks);

  // Information retrieval section...

  inline int GetPopulationMode() { return population_mode; }
  inline int GetPointMutRate()   { return point_mut_rate; }
  inline int GetCopyMutRate()    { return copy_mut_rate; }
  inline int GetDivideMutRate()  { return divide_mut_rate; }
  inline int GetDivideInsRate()  { return divide_ins_rate; }
  inline int GetDivideDelRate()  { return divide_del_rate; }

  inline int GetNumCells()         { return num_cells; }
  inline int GetNumDeaths()        { return num_deaths; }
  inline int GetNumCreatures()     { return num_creatures; }
  inline int GetNumGenotypes()     { return num_genotypes; }
  inline int GetNumThreshold()     { return num_threshold; }
  inline int GetNumSpecies()       { return num_species; }
  inline int GetNumThreshSpecies() { return num_thresh_species; }
  inline int GetBreedTrue()        { return num_breed_true; }
  inline int GetNumParasites()     { return num_parasites; }

  inline int GetTotCreatures()     { return tot_creatures; }
  inline int GetTotGenotypes()     { return tot_genotypes; }
  inline int GetTotThreshold()     { return tot_threshold; }
  inline int GetTotSpecies()       { return tot_species; }
  inline int GetTotThreshSpecies() { return tot_thresh_species; }

  inline int GetGetCount() { return get_count; }
  inline int GetPutCount() { return put_count; }
  inline int GetGGPCount() { return ggp_count; }
  inline int GetTaskCount(int task_num) { return task_count[task_num]; }

  inline cMerit GetAveMerit()       { return ave_merit; }
  inline double GetAveGestation()   { return ave_gestation; }
  inline double GetAveReproRate()   { return ave_repro_rate; }
  inline double GetAveFitness()     { return ave_fitness; }
  inline double GetAveSize()        { return ave_size; }
  inline double GetAveCopySize()    { return ave_copy_size; }
  inline double GetAveExeSize()     { return ave_exe_size; }
  inline double GetAveMemSize()     { return ave_mem_size; }
  inline double GetEntropy()        { return entropy; }
  inline double GetSpeciesEntropy() { return species_entropy; }
  inline double GetEnergy()         { return energy; }

  inline double GetAveCreatureAge()  { return ave_creature_age; }
  inline double GetAveGenotypeAge()  { return ave_genotype_age; }
  inline double GetAveThresholdAge() { return ave_threshold_age; }
  inline double GetAveSpeciesAge()   { return ave_species_age; }

  inline double GetMaxFitness() { return max_fitness; }
  inline double GetMaxMerit()   { return max_merit; }

  inline double GetProbCopyTrue();
};

inline void cLocalStats::SetPopulationMode(int in_population_mode)
{
  population_mode = in_population_mode;
}

inline void cLocalStats::SetMaxFitness(double in_max_fitness)
{
  max_fitness = in_max_fitness;
}

inline void cLocalStats::SetMaxMerit(double in_max_merit)
{
  max_merit = in_max_merit;
}

inline void cLocalStats::SetAveMerit(cMerit new_ave_merit)
{
  ave_merit = new_ave_merit;
}

inline void cLocalStats::SetAveGestation(double new_ave_gestation)
{
  ave_gestation = new_ave_gestation;
}

inline void cLocalStats::SetAveReproRate(double new_ave_repro_rate)
{
  ave_repro_rate = new_ave_repro_rate;
}

inline void cLocalStats::SetAveFitness(double new_ave_fitness)
{
  ave_fitness = new_ave_fitness;
}

inline void cLocalStats::SetAveSize(double in_ave_size)
{
  ave_size = in_ave_size;
}

inline void cLocalStats::SetAveCopySize(double in_ave_copy_size)
{
  ave_copy_size = in_ave_copy_size;
}

inline void cLocalStats::SetAveExeSize(double in_ave_exe_size)
{
  ave_exe_size = in_ave_exe_size;
}

inline void cLocalStats::SetAveMemSize(double in_ave_mem_size)
{
  ave_mem_size = in_ave_mem_size;
}

inline void cLocalStats::CalcEnergy()
{
  if (ave_fitness == 0.0 || best_fitness == 0.0) {
    energy = 0.0;
  } else  {
    energy = log(best_fitness / ave_fitness);
  }
}

inline void cLocalStats::CalcFidelity()
{
  double base_fidelity = (1.0 - (double)divide_mut_rate / DIVIDE_MUT_BASE) *
    (1.0 - (double)divide_ins_rate / DIVIDE_INS_BASE) * 
    (1.0 - (double)divide_del_rate / DIVIDE_DEL_BASE);
  ave_fidelity =
    base_fidelity * pow(1.0 - (double)copy_mut_rate / COPY_MUT_BASE, ave_size);
  dom_fidelity =
    base_fidelity * pow(1.0 - (double)copy_mut_rate / COPY_MUT_BASE, best_size);
}

inline void cLocalStats::AddCreature(int id_num, int genotype_num ){
  tot_creatures++;
  if( fp_creature_log.good() )
    fp_creature_log<<current_update<<" "
		   <<id_num<<" "
		   <<genotype_num<<endl;
}

inline void cLocalStats::NewCreature(int genotype_num, int parent_id, 
				     int parent_dist){
  if( fp_phylogeny_log.good() )
    fp_phylogeny_log<<current_update<<" "
		   <<genotype_num<<" "
		   <<parent_id<<" "
		   <<parent_dist<<endl;
} 

inline void cLocalStats::RemoveCreature(int genotype_num, int num_divides,
					int age) {
  if( fp_breed_log.good() )
    fp_breed_log<<current_update<<" "
		<<genotype_num<<" "
		<<num_divides<<" "
		<<age<<endl;
}

inline void cLocalStats::AddGenotype(int id_num) {
  id_num = -1;  // @CAO do we still need id_num here?
  tot_genotypes++;
}

inline void cLocalStats::RemoveGenotype(int id_num, int parent_id,
   int parent_dist, int max_abundance, int parasite_abundance, int age,
   int length) {
  if( fp_genotype_log.good() && (!log_threshold_only || max_abundance > 2))
    fp_genotype_log<<current_update<<" "
		   <<id_num<<" "
		   <<parent_id<<" "
		   <<parent_dist<<" "
		   <<max_abundance<<" "
		   <<parasite_abundance<<" "
		   <<age<<" "
		   <<length<<endl;
}

inline void cLocalStats::AddThreshold(int id_num, char * name, int species_num)
{
  num_threshold++;
  tot_threshold++;
  if( fp_threshold_log.good() )
    fp_threshold_log<<current_update<<" "
		    <<id_num<<" "
		    <<species_num<<" "
		    <<name<<endl;
}

inline void cLocalStats::RemoveThreshold(int id_num) {
  id_num = -1;  // @CAO do we still need id_num here?
  num_threshold--;
}

inline void cLocalStats::AddSpecies(int id_num) {
  id_num = -1; // @CAO do we still need id_num here?
  tot_species++;
  num_species++;
}

inline void cLocalStats::RemoveSpecies(int id_num, int parent_id,
			   int max_gen_abundance, int max_abundance, int age) {
  num_species--;
  if( fp_species_log.good() )
    fp_species_log<<current_update<<" "
		  <<id_num<<" "
		  <<parent_id<<" "
		  <<max_gen_abundance<<" "
		  <<max_abundance<<" "
		  <<age<<endl;
}

inline void cLocalStats::AddExecuted(int in_executed)
{
  num_executed += in_executed;
}

inline void cLocalStats::SetGets(int in_gets)
{
  get_count = in_gets;
}

inline void cLocalStats::SetPuts(int in_puts)
{
  put_count = in_puts;
}

inline void cLocalStats::SetGGPs(int in_ggps)
{
  ggp_count = in_ggps;
}

inline void cLocalStats::SetTasks(int task_num, int in_tasks)
{
#ifdef DEBUG
  if (task_num > num_tasks) {
    g_debug.Error("Recording non-existant task!\n");
  }
#endif

  task_count[task_num] = in_tasks;
}

inline void cLocalStats::SetNumGenotypes(int new_genotypes)
{
  num_genotypes_last = num_genotypes;
  num_genotypes = new_genotypes;
}

inline void cLocalStats::SetNumCreatures(int new_creatures)
{
  num_creatures = new_creatures;
}

inline void cLocalStats::SetNumThreshSpecies(int new_thresh_species)
{
  num_thresh_species = new_thresh_species;
}

inline void cLocalStats::SetBreedTrue(int in_num_breed_true)
{
  num_breed_true = in_num_breed_true;
}

inline void cLocalStats::SetNumParasites(int in_num_parasites)
{
  num_parasites = in_num_parasites;
}

inline double cLocalStats::GetProbCopyTrue()
{
  double real_copy_mut_rate = 0.0001 * (double) copy_mut_rate;
  double copy_true_rate = 1 - real_copy_mut_rate;
  double copy_true = 1;
  for (int i = 0; i < ave_size; i++) {
    copy_true *= copy_true_rate;
  }
  return copy_true;
}

//////////////////
//  cGlobalStats
//////////////////

inline void cGlobalStats::SetNumPopulation(int in_num_populations)
{
  num_populations = in_num_populations;
}

inline void cGlobalStats::SetNumInstructions(int in_num_instructions)
{
  num_instructions = in_num_instructions;
}

inline void cGlobalStats::SetSizeMeritMethod(int in_merit_method)
{
  size_merit_method = in_merit_method;
}

inline void cGlobalStats::SetTaskMeritMethod(int in_merit_method)
{
  task_merit_method = in_merit_method;
}

inline void cGlobalStats::SetBirthMethod(int in_birth_method)
{
  birth_method = in_birth_method;
}

inline void cGlobalStats::SetDeathMethod(int in_death_method)
{
  death_method = in_death_method;
}

inline void cGlobalStats::SetAgeLimit(int in_age_limit)
{
  age_limit = in_age_limit;
}

inline void cGlobalStats::SetCurrentUpdate(int new_update)
{
  current_update = new_update;
  sub_update = 0;
}

inline void cGlobalStats::SetMaxUpdates(int in_max_updates)
{
  max_updates = in_max_updates;
}

extern cGlobalStats stats;

#endif
