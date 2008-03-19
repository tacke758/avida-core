//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef POPULATION_HH
#define POPULATION_HH

#define HEAD_RANGE 4096

#include "../tools/int.hh"
#include "../tools/file.hh"
#include "event.hh"
#include "../cpu/cpu.hh"
#include "landscape.hh"

class cSchedule;
class cEnvironment;
class cGeneology;

class cPopulation {
private:
  // Components...
  cView * viewer;
  cSchedule * schedule;
  cInstLib * inst_lib;
  cBaseCPU * cpu_array;
  cEventList * event_list;
  cEnvironment * default_environment;

  // Data Tracking...
  cGenebank * genebank;
  cGeneology * geneology;
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
  void CalcConsensus(int lines_saved);

  // If a filename is passed in, write out the fitness results of each mutation
  cCodeArray CalcLandscape(int dist, const cGenotype & genotype,
          cString * fitnessChartFileName=0);
  cCodeArray SampleLandscape(int sample_size, const cCodeArray & code);
  cCodeArray RandomLandscape(int dist, int sample_size, int min_found,
     int max_sample_size, const cCodeArray & code, int print_if_found=FALSE);
  void AnalyzeLandscape(const cCodeArray & code, int sample_size=1000);
  void PairTestLandscape(const cCodeArray & code, int sample_size=1000);
  void PairTestFullLandscape(const cCodeArray & code);
  void Apocalypse(double kill_prob);
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

public:
  cPopulation();
  ~cPopulation();

  void ProcessEvents(); // Process all active events from the event list.
  void InitSoup();      // Place the initial creatures in the soup...

  // Stat functions...

  void MapGeneticDistance();

  // Update all of the creatures currently in the soup by whatever means the
  // soup is currently configured for.
  void DoUpdate();

  // This function is called when a mother successfully executed a divide
  // command.  The population is given back all the information it needs on
  // the child, as well as a pointer to the mother's cpu.
  void ActivateChild(sReproData & child_info, cBaseCPU * in_cpu=NULL);

  // This function directs which position function should be used.  It
  // could have also been done with a function pointer, but the dividing
  // of a CPU takes enough time that this will be a negligible addition,
  // and it gives a centralized function to work with.
  cBaseCPU * PositionChild(cBaseCPU * parent_cpu);

  // This function is to be run whenever the merit of a creature has changed
  // so as to do whatever adjustments may be needed to the time-slicing method
  // used.
  void AdjustTimeSlice(int);

  // The following function is called whenever a tagged CPU executes an
  // instruction so that the population object can in turn inform any other
  // object (such as the viewer) which need to know.  NOTE: The bodies of
  // these methods are located in avida.hh.
  void NotifyUpdate();

  void NotifyError(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyComment(const cString & in_string);
  void Pause();
  void DoBreakpoint();

  // For block time slice, this function gets the merit of the incoming
  // creature, and returns the number of instructions to be processed.
  int ScaleMerit(const cMerit & in_merit);
  void CalcUpdateStats();

  // Other useful functions...

  void HillClimb(const cCodeArray & code);

  int SaveClone(ofstream & fp);
  int LoadClone(ifstream & fp);
  int OK();

  void Clear();
  void Inject(const char * filename, int in_cpu = 0, int merit = -1, int lineage_label = 0);
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

  // Viewer info...

  inline void SetViewer(cView & in_viewer) { viewer = &in_viewer; }

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
};

#endif
