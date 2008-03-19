//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef POPULATION_HH
#define POPULATION_HH

#define HEAD_RANGE 4096

#include "../defs.hh"
#include "../tools/int.hh"
#include "../tools/file.hh"
#include "../cpu/cpu.hh"
#include "landscape.hh"

class cSchedule;
class cEnvironment;
class cGeneology;
class cLineageControl;

class cPopulation {
private:
  // Components...
  cSchedule * schedule;
  cInstLib * inst_lib;
  cBaseCPU * cpu_array;
  cEnvironment * default_environment;

  // Data Tracking...
  cGenebank * genebank;
  cGeneology * geneology;
  cLineageControl * lineage_control;
  cList * reaper_queue;   // Death order in certain global allocation methods.

  // Other data...
  int world_x;
  int world_y;
  int num_cells;       // Total cells in population.
  int num_creatures;   // Total living cells in population.
  cMerit max_merit;

private:
  void BuildTimeSlicer(); // Build the schedule object
  void PositionAge(cBaseCPU * parent_cpu, cList & found_list);
  void PositionMerit(cBaseCPU * parent_cpu, cList & found_list);
  void FindEmptyCell(cList * cell_list, cList & found_list);


public:  // Called from cEventList
  void SetCopyMutProb(double new_cmut, int cell = -1); // cell=-1 => all
  void SetPointMutProb(double new_pmut, int cell = -1); // cell=-1 => all
  void CalcConsensus(int lines_saved);
  cCodeArray CalcLandscape(int dist, const cCodeArray & code);
  cCodeArray PredictWLandscape(const cCodeArray & code, cString & filename);
  cCodeArray PredictNuLandscape(const cCodeArray & code, cString & filename);
  cCodeArray SampleLandscape(int sample_size, const cCodeArray & code);
  cCodeArray RandomLandscape(int dist, int sample_size, int min_found,
			     int max_sample_size, const cCodeArray & code, int print_if_found=FALSE);
  void AnalyzeLandscape(const cCodeArray & code, int sample_size=1000,
			int min_found=0, int max_sample_size = 0);
  void PairTestLandscape(const cCodeArray & code, int sample_size=1000);
  void PairTestFullLandscape(const cCodeArray & code);
  void AnalyzeTasksites(const cCodeArray & code);
  void TestCode(const cCodeArray & code);
  void HillClimb(const cCodeArray & code);
  void HillClimb_Neut(const cCodeArray & code);
  void HillClimb_Rand(const cCodeArray & code);

  /**
   * This function goes through all the creatures in the soup and kills
   * them randomly, according to the probability kill_prob.
   **/
  void Apocalypse(double kill_prob);

  /**
   * This function does mainly the same thing as Apocalypse, but it takes
   * a kill rate that is measured in the same units as fitness. Creatures
   * are killed according to this rate. The function makes sure that the
   * kill rate gets appropriately rescaled with the current average merrit
   * and the average time slice, so that the rate is not influenced by the
   * current population structure. It is assumed to be called on every update.
   *
   * If you want to kill creatures at a constant rate, this is the function
   * you want to use.
   **/
  void RateKill( double kill_rate );

  /**
   * This function goes through all creatures in the soup, and saves the
   * basic landscape data (neutrality, fitness, and so on) into a stream.
   *
   * @param fp The stream into which the data should be saved.
   *
   * @param sample_prob The probability with which a particular creature should
   * be analyzed (a value of 1 analyzes all creatures, a value of 0.1 analyzes
   * 10%, and so on).
   *
   * @param landscape A bool that indicates whether the creatures should be
   * landscaped (calc. neutrality and so on) or not.
   *
   * @param save_genotype A bool that indicates whether the creatures should
   * be saved or not.
   **/
  void AnalyzePopulation(ofstream & fp, double sample_prob = 1, bool landscape = false, bool save_genotype = false);

  /**
   * This function prints out fitness data. The main point is that it
   * calculates the average fitness from info from the testCPU + the actual
   * merit of the organisms, and assigns zero fitness to those organisms
   * that will never reproduce.
   *
   * The function also determines the maximum fitness genotype, and can
   * produce fitness histograms.
   *
   * @param datafp A stream into which the fitness data should be written.
   * @param histofp A stream into which the fitness histogram should be
   * written.
   * @param histo_testCPU_fp A stream into which the fitness histogram as
   * determined exclusively from the test-CPU should be written.
   * @param save_max_f_genotype A bool that determines whether the genotype
   * with the maximum fitness should be saved into the genebank.
   * @param print_fitness_histo A bool that determines whether fitness
   * histograms should be written.
   * @param hist_fmax The maximum fitness value to be taken into account
   * for the fitness histograms.
   * @param hist_fstep The width of the individual bins in the fitness
   * histograms.
   **/
  void PrintDetailedFitnessData(ofstream & datafp, ofstream & histofp, ofstream & histo_testCPU_fp, bool save_max_f_genotype, bool print_fitness_histo, double hist_fmax, double hist_fstep);

  /**
   * This function goes through all genotypes currently present in the soup,
   * and writes into an output file the average Hamming distance between the
   * creatures in the population and a given reference genome.
   *
   * @param fp The stream into which the data should be saved.
   * @param reference_code The reference genome.
   * @param save_creatures A bool that indicates whether creatures should be
   * saved into the genebank or not.
   **/
  void PrintGeneticDistanceData(ofstream & fp, const char *creature_name );

  /**
   * This function goes through all genotypes currently present in the soup,
   * and writes into an output file the names of the genotypes, the fitness
   * as determined in the test cpu, and the genetic distance to a reference
   * genome.
   *
   * @param fp The stream into which the data should be saved.
   * @param reference_code The reference genome.
   * @param save_creatures A bool that indicates whether creatures should be
   * saved into the genebank or not.
   **/
  void GeneticDistancePopDump(ofstream & fp, const char *creature_name, bool save_creatures = false );


  /**
   * This function goes through all creatures in the soup, and writes out
   * how many tasks the different creatures have done up to now. It counts
   * every task only once, i.e., if a creature does 'put' three times, that
   * will increase its count only by one.
   *
   * @param fp The file into which the result should be written.
   **/
  void TaskSnapshot( ofstream &fp );

  
  /**
   * This function prints all the tasks that viable creatures have performed
   * so far (compare with the event 'print_task_data', which prints all tasks.
   **/
  void PrintViableTasksData( ofstream &fp);
  
  void TestInsSizeChangeRobustness(const cString & filename,
				   const cCodeArray & in_code, int num_trials);


  void PrintTreeDepths();
public:
  cPopulation();
  ~cPopulation();

  /**
   * Place the initial creatures in the soup...
   **/
  void InitSoup();

  /**
   * Stat functions...
   */
  void MapGeneticDistance();

  /**
   * Update all of the creatures currently in the soup by whatever means the
   * soup is currently configured for.
   **/
  void DoUpdate();

  /**
   * This function is called when a mother successfully executed a divide
   * command.  The population is given back all the information it needs on
   * the child, as well as a pointer to the mother's cpu.
   **/
  void ActivateChild(sReproData & child_info, cBaseCPU * in_cpu=NULL);

  /**
   * This function does all the necessary stuff to kill a creature (apart
   * from readjusting the scheduler).
   **/
  void KillCreature( cBaseCPU * cpu, bool clear_genotype = true,
		     bool adjust_scheduler = true );

  /**
   * This function is responsible for adding a creature to a given lineage,
   * and setting the lineage label and the lineage pointer in the
   * creature correctly. Attention: The function ActivateChild() has its
   * own version of this.
   **/
  void LineageAddCreature( cBaseCPU *cpu, cLineage *lineage, int lineage_label );

  /**
   * This function directs which position function should be used.  It
   * could have also been done with a function pointer, but the dividing
   * of a CPU takes enough time that this will be a negligible addition,
   * and it gives a centralized function to work with.
   **/
  cBaseCPU * PositionChild(cBaseCPU * parent_cpu);

  /**
   * This function is to be run whenever the merit of a creature has changed
   * so as to do whatever adjustments may be needed to the time-slicing
   * method used.
   **/
  void AdjustTimeSlice(int);

  /**
   * The following function is called whenever a tagged CPU executes an
   * instruction so that the population object can in turn inform any other
   * object (such as the viewer) which need to know.  NOTE: The bodies of
   * these methods are located in avida.hh.
   **/
  void NotifyUpdate();

  void NotifyError(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyComment(const cString & in_string);
  void Pause();
  void DoBreakpoint();

  /**
   * For block time slice, this function gets the merit of the incoming
   * creature, and returns the number of instructions to be processed.
   **/
  int ScaleMerit(const cMerit & in_merit);

  void CalcUpdateStats();

  // Other useful functions...

  bool SaveClone(ofstream & fp);
  bool LoadClone(ifstream & fp);
  bool DumpTextSummary(ofstream & fp);
  bool DumpDetailedSummary(ofstream & fp);
  bool DumpHistoricSummary(ofstream & fp);

  int SavePopulation(ofstream & fp);
  int LoadPopulation(ifstream & fp);

  int SaveOrganism(cString & filename, int cpu_num);
  int LoadOrganism(cString & filename, int cpu_num, int lineage_label);

  int OK();

  void Clear();

  /**
   * This function loads a genome from a given file, and initializes
   * a cpu with it.
   *
   * @param filename The name of the file to load.
   *
   * @param in_cpu The grid-position into which the code should be loaded.
   *
   * @param merit An initial merit value.
   *
   * @param lineage_label A value that allows to track the daughters of
   * this creature.
   **/
  void Inject(const char * filename, int in_cpu = 0, double merit = -1, int lineage_label = 0, double neutral_metric = 0 );

  void InjectAll(const char * filename);
  void InjectRandom(int mem_size);
  cGenotype * GetGenotype(int thread);
  cBaseCPU * GetRandomCPU();
  cCPUHead GetHeadPosition(cBaseCPU * in_cpu, int offset);


  inline void AddCreature(int id_num, int genotype_id) {
    cStats::AddCreature(id_num, genotype_id);
    if (genotype_id >= 0) num_creatures++;
  }
  inline void RemoveCreature(int id_num, int divides, int age) {
    cStats::RemoveCreature(id_num, divides, age);
    num_creatures--;
  }

  inline int GetSize() { return num_cells; }
  inline int GetWorldX() { return world_x; }
  inline int GetWorldY() { return world_y; }

  inline cBaseCPU & GetCPU(int in_num) { return cpu_array[in_num]; }
  inline cBaseCPU * GetCPUArray() { return cpu_array; }

  inline cGenebank & GetGenebank() { return *genebank; }
  inline cLineageControl * GetLineageControl() { return lineage_control; }
  inline cGeneology * GetGeneology() { return geneology; }
  inline cInstLib * GetInstLib() { return inst_lib; }
  inline cSchedule * GetSchedule() { return schedule; }
  inline cEnvironment * GetDefaultEnvironment()
  { return default_environment; }

  // Statistics...

  unsigned int GetTotalMemory();

  inline int GetNumCreatures() { return num_creatures; }
  int GetNumGenotypes();
  int GetNumSpecies();

  // Called from viewer...

  char * GetBasicGrid();
  char * GetSpeciesGrid();
  char * GetModifiedGrid();
  char * GetResourceGrid();
  char * GetAgeGrid();
  char * GetBreedGrid();
  char * GetParasiteGrid();
  char * GetPointMutGrid();
  char * GetThreadGrid();
  char * GetLineageGrid();


  // Event Triggers & Outputs
  void PrintDepthHistogram(const cString & filename);
  void PrintGenotypeAbundanceHistogram(const cString & filename);
  void PrintSpeciesAbundanceHistogram(const cString & filename);

};

#endif

