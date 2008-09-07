//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef POPULATION_HH
#define POPULATION_HH

#define SLICE_CONSTANT         0
#define SLICE_BLOCK_MERIT      1
#define SLICE_PROB_MERIT       2
#define SLICE_INTEGRATED_MERIT 3
#define SLICE_LOGRITHMIC_MERIT 4

#define POSITION_CHILD_RANDOM   0
#define POSITION_CHILD_AGE      1
#define POSITION_CHILD_MERIT    2
#define POSITION_CHILD_EMPTY    3

#define DEATH_METHOD_OFF      0
#define DEATH_METHOD_CONST    1
#define DEATH_METHOD_MULTIPLE 2

#include "../tools/int.hh"
#include "../tools/file.hh"
#include "../cpu/cpu.hh"

class cSchedule;
class cEnvironment;

class cPopulation {
protected:
  cGenesis * genesis;
  cGenebank * genebank;
  cInstLib * inst_lib;
  cEventList * event_list;
  cView * viewer;
  cLocalStats local_stats;
  cSchedule * schedule;

  // General use stuff...
  int num_cells;       // Total cells in population.
  int num_creatures;   // Total living cells in population.
  cEnvironment * default_environment;

  // Time sliceing...
  int slicing_method;
  int birth_method;
  int death_method;
  int age_limit;
  int ave_time_slice;
  cMerit max_merit;
public:
  cPopulation(const cGenesis & in_genesis);
  virtual ~cPopulation();

  void InitSoup();   // Place the initial creatures in the soup...

  // Stat functions...

  void MapGeneticDistance();

  // Update all of the creatures currently in the soup by whatever means the
  // soup is currently configured for.
  virtual void DoUpdate();

  // This function is called when a mother successfully executed a divide
  // command.  The population is given back all the information it needs on
  // the child, as well as a pointer to the mother's cpu.
  virtual void ActivateChild(sReproData * child_info, cBaseCPU * in_cpu=NULL);

  // This function is to be run whenever the merit of a creature has changed
  // so as to do whatever adjustments may be needed to the time-slicing method
  // used.
  virtual void AdjustTimeSlice(int in_cpu);

  // The following function is called whenever a tagged CPU executes an
  // instruction so that the population object can in turn inform any other
  // object (such as the viewer) which need to know.  NOTE: The bodies of
  // these methods are located in avida.hh.
  void NotifyUpdate();
  void NotifyError(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyComment(const cString & in_string);
  void Pause();

  // For block time slice, this function gets the merit of the incoming
  // creature, and returns the number of instructions to be processed.
  int ScaleMerit(const cMerit & in_merit);
  virtual void CalcUpdateStats();

  // Handle incoming events, typically from the event_list file.
  virtual void ProcessEvent(const cEvent & event);

  // Other useful functions...

  virtual int OK();
  virtual void Clear();
  virtual void Inject(char * filename, int in_cpu = 0);
  cGenotype * GetGenotype(int thread);
  virtual cBaseCPU * GetRandomCPU();  

  inline void AddCreature(int id_num, int genotype_id) {
    local_stats.AddCreature(id_num, genotype_id);
    if (genotype_id >= 0) num_creatures++;
  }
  inline void RemoveCreature(int id_num, int divides, int age) {
    local_stats.RemoveCreature(id_num, divides, age);
    num_creatures--;
  }

  inline int GetSize() { return num_cells; }
  inline cGenesis & Genesis() { return *genesis; }
  inline cGenebank * GetGenebank() { return genebank; }
  inline int GetSlicingMethod() { return slicing_method; }
  inline int GetAveTimeslice() { return ave_time_slice; }
  inline cInstLib * GetInstLib() { return inst_lib; }
  inline cSchedule * GetSchedule() { return schedule; }
  inline cEnvironment * GetDefaultEnvironment()
    { return default_environment; }

  // Statistics...

  inline cLocalStats * GetLocalStats() { return &local_stats; }
  virtual unsigned int GetTotalMemory();

  inline int GetNumCreatures() { return num_creatures; }
  virtual int GetNumGenotypes();
  virtual int GetNumSpecies();

  // Viewer info...

  inline void SetViewer(cView * in_viewer) { viewer = in_viewer; }
};

class cGAPopulation : public cPopulation {
private:
  cBaseCPU cpu;                    // Single cpu cycled through.
public:
  cGAPopulation(const cGenesis & in_genesis);
  ~cGAPopulation();

  void DoUpdate();

  // Called from cpu...

  void ActivateChild(sReproData * child_info, cBaseCPU * in_cpu=NULL);

  // Other useful functions...

  int OK();
  void Inject(char * filename, int in_cpu = 0);

  // Statistics...

  unsigned int GetTotalMemory();
};

class cAutoAdaptivePopulation : public cPopulation {
protected:
  cBaseCPU * cpu_array;   // Each creature must have their own CPU...

  void BuildTimeSlicer(); // Build the schedule object
public:
  cAutoAdaptivePopulation(const cGenesis & in_genesis);
  virtual ~cAutoAdaptivePopulation();

  inline cBaseCPU * GetCPU(int in_num) { return &(cpu_array[in_num]); }
  inline cBaseCPU * GetCPUArray() { return cpu_array; }
  inline void SingleProcess(int cpu_num) { cpu_array[cpu_num].SingleProcess();}

  void DoUpdate();
  void AdjustTimeSlice(int in_cpu);
  void Clear();
  void Inject(char * filename, int in_cpu = 0);
  void CalcUpdateStats();
  void ProcessEvent(const cEvent & event);
  cBaseCPU * GetRandomCPU();
  unsigned int GetTotalMemory();

  virtual void ActivateChild(sReproData * child_info, cBaseCPU * in_cpu=NULL);
  virtual int OK();
};

class cTierraPopulation : public cAutoAdaptivePopulation {
private:
  cList * reaper_queue;       // Order for deaths to occur.
public:
  cTierraPopulation(const cGenesis & in_genesis);
  ~cTierraPopulation();

  void ActivateChild(sReproData * child_info, cBaseCPU * in_cpu=NULL);
  int OK();
};

class cAvidaPopulation : public cAutoAdaptivePopulation {
private:
  int world_x;
  int world_y;

  // Private functions.
  void PositionAge(cBaseCPU * parent_cpu, cList * found_list);
  void PositionMerit(cBaseCPU * parent_cpu, cList * found_list);

  void FindEmptyCell(cList * cell_list, cList * found_list);
public:
  cAvidaPopulation(const cGenesis & in_genesis);
  ~cAvidaPopulation();

  // Called from cpu...

  cBaseCPU * PositionChild(cBaseCPU * parent_cpu);
  void ActivateChild(sReproData * child_info, cBaseCPU * in_cpu=NULL);
  
  void SetBirthMethod(int in_birth_method) { birth_method = in_birth_method; }

  // Called from viewer...

  char * GetBasicGrid();
  char * GetSpeciesGrid();
  char * GetInjectGrid();
  char * GetResourceGrid();
  char * GetAgeGrid();
  char * GetBreedGrid();
  char * GetParasiteGrid();
  char * GetPointMutGrid();

  // Other useful functions...

  int OK();
  inline int GetWorldX() { return world_x; }
  inline int GetWorldY() { return world_y; }
};

#endif
