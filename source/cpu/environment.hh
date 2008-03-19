//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef ENVIRONMENT_HH
#define ENVIRONMENT_HH

#include "../defs.hh"
#include "cpu_defs.hh"
#include "cpu.hh"
#include "../main/population.hh"
#include "../main/genebank.hh"
#include "test_cpu.hh"

/**
 * The sEnvironment structure contains all the enviornment info about the
 * lattice cell in which the creature resides.
 **/

class cEnvironment {
protected:
  int resource[NUM_RESOURCES];
  int generation;
  int creature_count;   // Total creatures to have inhabited this CPU.

  double point_mut_prob;
  double copy_mut_prob;
  double ins_mut_prob;
  double del_mut_prob;
  double divide_mut_prob;
  double divide_ins_prob;
  double divide_del_prob;
  double crossover_prob;
  double aligned_cross_prob;
  double exe_err_prob;

  cBaseCPU * cpu;
public:
  cEnvironment(cBaseCPU * in_cpu);
  virtual ~cEnvironment();

  virtual int InPopulation() = 0;
  virtual void AdjustTimeSlice() { ; }
  virtual void ActivateChild(sReproData & repro_data) = 0;
  virtual void RotateL() { ; }
  virtual void RotateR() { ; }
  virtual void Rotate(cBaseCPU * cpu_dir) { (void) cpu_dir; }
  virtual int TestResources() = 0;
  virtual int UseResources() = 0;
  virtual cCPUHead GetHeadPosition(cBaseCPU * cur_cpu, int offset);
  virtual void AddDivideRecord(cDivideRecord & in_record)
    { (void) in_record; }
  virtual cDivideRecord * GetDivideRecord() { return NULL; }
  virtual void CPUReset() { generation=0; } // Clear Resources!!! @TCC

  virtual void Notify() { ; }
  virtual void Pause() { ; }
  virtual void NotifyError(const cString & message) { (void) message; }
  virtual void NotifyWarning(const cString & message) { (void) message; }
  virtual void NotifyComment(const cString & message) { (void) message; }
  virtual void DoBreakpoint() { ; }

  virtual cGenotype * AddGenotype(const cCodeArray & in_code)
    { (void) in_code;  return NULL; }
  virtual int AdjustGenotype(cGenotype & in_genotype)
    { (void) in_genotype; return -1; }
  
  virtual void KillCreature( cBaseCPU * cpu ) = 0;

  virtual cList * GetConnections() { return NULL; }
  virtual void SetConnections(cList * in_list) = 0;
  virtual cBaseCPU * GetFacing() { return NULL; }
  virtual cBaseCPU * GetNeighbor() { return NULL; }
  virtual int GetID() { return -1; }
  virtual void SetID(cPopulation * in_population, int in_id_num) = 0;

  virtual int GetNeighborhoodSize() = 0;

  virtual void Stats_AddCreature(int genotype_id) = 0;
  virtual void Stats_RemoveCreature(int divides, int age) = 0;
  virtual void Stats_AddExecuted(int in_exec) = 0;
  virtual void Stats_AddDeath() = 0;

  virtual void GStats_SetGestationTime(int in_gest_time) = 0;
  virtual void GStats_SetFitness(double in_fitness) = 0;
  virtual void GStats_SetMerit(const cMerit & in_merit) = 0;
  virtual void GStats_SetExecutedSize(int in_exe_size) = 0;
  virtual void GStats_SetCopiedSize(int in_copied_size) = 0;

  virtual int ScaleMerit(const cMerit & in_merit) = 0;

  virtual int OK();

  inline int TestCopyMut()   { return g_random.GetDouble() < copy_mut_prob; }
  // INS and DEL mutations are handled in cBaseCPU::Divide_Mutations
  inline int TestPointMut()  { return g_random.GetDouble() < point_mut_prob; }
  inline int TestDivideMut() { return g_random.GetDouble() < divide_mut_prob; }
  inline int TestDivideIns() { return g_random.GetDouble() < divide_ins_prob; }
  inline int TestDivideDel() { return g_random.GetDouble() < divide_del_prob; }
  inline int TestCrossover() { return g_random.P(crossover_prob); }
  inline int TestAlignedCross() { return g_random.P(aligned_cross_prob); }
  inline int TestExeErr()    { return g_random.GetDouble()<exe_err_prob; }

  inline int GetResource(int res_id) { return resource[res_id]; }
  inline int GetGeneration()    { return generation; }
  inline int GetCreatureCount() { return creature_count; }
  inline double GetCopyMutProb()   { return copy_mut_prob; }
  inline double GetInsMutProb()    { return ins_mut_prob; }
  inline double GetDelMutProb()    { return del_mut_prob; }
  inline double GetPointMutProb()  { return point_mut_prob; }
  inline double GetDivideMutProb() { return divide_mut_prob; }
  inline double GetDivideInsProb() { return divide_ins_prob; }
  inline double GetDivideDelProb() { return divide_del_prob; }
  inline double GetCrossoverProb() { return crossover_prob; }
  inline double GetAlignedCrossProb() { return aligned_cross_prob; }
  inline double GetExeErrProb()    { return exe_err_prob; }

  inline void SetGeneration(int in_gen) { generation = in_gen; }
  inline void SetCopyMutProb(double in_prob)   { copy_mut_prob   = in_prob; }
  inline void SetInsMutProb(double in_prob)    { ins_mut_prob   = in_prob; }
  inline void SetDelMutProb(double in_prob)    { del_mut_prob   = in_prob; }
  inline void SetPointMutProb(double in_prob)  { point_mut_prob  = in_prob; }
  inline void SetDivideMutProb(double in_prob) { divide_mut_prob = in_prob; }
  inline void SetDivideInsProb(double in_prob) { divide_ins_prob = in_prob; }
  inline void SetDivideDelProb(double in_prob) { divide_del_prob = in_prob; }
  inline void SetCrossoverProb(double in_prob) { crossover_prob = in_prob; }
  inline void SetAlignedCrossProb(double in)   { aligned_cross_prob = in; }
  inline void SetExeErrProb(double in_prob)    { exe_err_prob = in_prob; }
};


/**
 * The main environment redirects information into the rest of the soup.
 **/

class cMainEnvironment :  public cEnvironment {
private:
  cPopulation * population;
  cList * connection_list;
  int id_num;
public:
  cMainEnvironment(cBaseCPU * in_cpu);
  ~cMainEnvironment();

  int InPopulation() { return TRUE; }
  void AdjustTimeSlice() { population->AdjustTimeSlice(id_num); }
  void ActivateChild(sReproData & repro_data)
    { population->ActivateChild(repro_data); }
  void RotateL() { connection_list->CircPrev(); }
  void RotateR() { connection_list->CircNext(); }
  void Rotate(cBaseCPU * cpu_dir);

  int TestResources() { return TRUE; }
  int UseResources() { return TRUE; }
  cCPUHead GetHeadPosition(cBaseCPU * cur_cpu, int offset);

  void Notify() { population->NotifyUpdate(); }
  void Pause() { population->Pause(); }
  void NotifyError(const cString & message)
    { population->NotifyError(message); }
  void NotifyWarning(const cString & message)
    { population->NotifyWarning(message); }
  void NotifyComment(const cString & message)
    { population->NotifyComment(message); }
  void DoBreakpoint() { population->DoBreakpoint(); }

  cGenotype * AddGenotype(const cCodeArray & in_code)
    { return population->GetGenebank().AddGenotype(in_code); }
  int AdjustGenotype(cGenotype & in_genotype)
    { return population->GetGenebank().AdjustGenotype(in_genotype); }

  void KillCreature( cBaseCPU * cpu ){ 
    population->KillCreature( cpu ); }
  
  cList * GetConnections() { return connection_list; }
  void SetConnections(cList * in_list) { connection_list = in_list; }
  cBaseCPU * GetFacing() { return (cBaseCPU *) connection_list->GetCurrent(); }
  int GetID() { return id_num; }
  cBaseCPU * GetNeighbor();

  int GetNeighborhoodSize() { return connection_list->GetSize(); }

  void SetID(cPopulation * in_population, int in_id_num);

  void Stats_AddCreature(int genotype_id) {
    population->AddCreature(id_num, genotype_id);
  }

  void Stats_RemoveCreature(int divides, int age) {
    population->RemoveCreature(cpu->GetActiveGenotype()->GetID(),divides,age);
  }

  void Stats_AddExecuted(int in_exec) { cStats::AddExecuted(in_exec); }
  void Stats_AddDeath() { cStats::AddDeath(); }

  void GStats_SetGestationTime(int in_gest_time)
    { cpu->GetActiveGenotype()->SetGestationTime(in_gest_time); }
  void GStats_SetFitness(double in_fitness)
    { cpu->GetActiveGenotype()->SetFitness(in_fitness); }
  void GStats_SetMerit(const cMerit & in_merit)
    { cpu->GetActiveGenotype()->SetMerit(in_merit); }
  void GStats_SetExecutedSize(int in_exe_size)
    { cpu->GetActiveGenotype()->SetExecutedSize(in_exe_size); }
  void GStats_SetCopiedSize(int in_copied_size)
    { cpu->GetActiveGenotype()->SetCopiedSize(in_copied_size); }

  int ScaleMerit(const cMerit & in_merit)
    { return population->ScaleMerit(in_merit); }

  int OK();
};


/**
 * The test environment prevents data from going into the soup proper.
 **/

class cTestEnvironment :  public cEnvironment {
private:
public:
  cTestEnvironment(cBaseCPU * in_cpu);
  ~cTestEnvironment();

  int InPopulation() { return FALSE; }
  void ActivateChild(sReproData & repro_data);
  int TestResources() { return TRUE; }
  int UseResources() { return TRUE; }
  void CPUReset() {
  }

  void SetConnections(cList * in_list) { (void) in_list; }
  int GetNeighborhoodSize() { return 0; }
  int ScaleMerit(const cMerit & in_merit) { (void) in_merit; return 0; }

  void SetID(cPopulation * in_population, int in_id_num)
    { (void) in_population; (void) in_id_num; }

  int OK();

  void KillCreature( cBaseCPU *cpu ){
     cpu->Kill();
     cpu->ChangeGenotype(NULL);
  }
  
  void Stats_AddCreature(int genotype_id) { (void) genotype_id; }
  void Stats_RemoveCreature(int divides, int age) {(void) divides; (void) age;}
  void Stats_AddExecuted(int in_exec) { (void) in_exec; }
  void Stats_AddDeath() { ; }

  void GStats_SetGestationTime(int in_gest_time) { (void) in_gest_time; }
  void GStats_SetFitness(double in_fitness)      { (void) in_fitness; }
  void GStats_SetMerit(const cMerit & in_merit)  { (void) in_merit; }
  void GStats_SetExecutedSize(int in_exe_size)   { (void) in_exe_size; }
  void GStats_SetCopiedSize(int in_copied_size)  { (void) in_copied_size; }
};

#endif








