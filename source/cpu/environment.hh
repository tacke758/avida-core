//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef ENVIRONMENT_HH
#define ENVIRONMENT_HH

#include "../defs.hh"
#include "cpu_defs.hh"
#include "cpu.hh"
#include "../main/population.hh"
#include "../main/genebank.hh"

// The sEnvironment structure contains all the enviornment info about the
// lattice cell in which the creature resides.

class cEnvironment {
protected:
  int resource[NUM_RESOURCES];
  int generation;

  int copy_mut_rate;
  int point_mut_rate;
  int divide_mut_rate;
  int divide_ins_rate;
  int divide_del_rate;

  cBaseCPU * cpu;
public:
  cEnvironment(cBaseCPU * in_cpu);
  virtual ~cEnvironment();  

  virtual void AdjustTimeSlice() { ; }
  virtual void ActivateChild(sReproData * repro_data) = 0;
  virtual void RotateL() { ; }
  virtual void RotateR() { ; }
  virtual void Rotate(cBaseCPU * cpu_dir) { (void) cpu_dir; }
  virtual int TestResources() = 0;
  virtual int UseResources() = 0;

  virtual void Notify() { ; }
  virtual void NotifyError(const cString & message) { (void) message; }
  virtual void NotifyWarning(const cString & message) { (void) message; }
  virtual void NotifyComment(const cString & message) { (void) message; }

  virtual cGenotype * AddGenotype(cCodeArray * in_code)
    { (void) in_code;  return NULL; }
  virtual int AdjustGenotype(cGenotype * in_genotype)
    { (void) in_genotype; return -1; }

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

  inline int TestCopyMut()
   { return (g_random.GetUInt(COPY_MUT_BASE) < (UINT) copy_mut_rate); }
  inline int TestPointMut()
   { return (g_random.GetUInt(POINT_MUT_BASE) <(UINT) point_mut_rate); }
  inline int TestDivideMut()
   { return (g_random.GetUInt(DIVIDE_MUT_BASE) < (UINT) divide_mut_rate); }
  inline int TestDivideIns()
   { return (g_random.GetUInt(DIVIDE_INS_BASE) < (UINT) divide_ins_rate); }
  inline int TestDivideDel()
   { return (g_random.GetUInt(DIVIDE_DEL_BASE) < (UINT) divide_del_rate); }

  inline int GetResource(int res_id) { return resource[res_id]; }
  inline int GetCopyMutRate()   { return copy_mut_rate; }
  inline int GetPointMutRate()  { return point_mut_rate; }
  inline int GetDivideMutRate() { return divide_mut_rate; }
  inline int GetDivideInsRate() { return divide_ins_rate; }
  inline int GetDivideDelRate() { return divide_del_rate; }

  inline void SetCopyMutRate(int in_rate)   { copy_mut_rate   = in_rate; }
  inline void SetPointMutRate(int in_rate)  { point_mut_rate  = in_rate; }
  inline void SetDivideMutRate(int in_rate) { divide_mut_rate = in_rate; }
  inline void SetDivideInsRate(int in_rate) { divide_ins_rate = in_rate; }
  inline void SetDivideDelRate(int in_rate) { divide_del_rate = in_rate; }
};


// The main environment redirects information into the rest of the soup.

class cMainEnvironment : public cEnvironment {
private:
  cPopulation * population;
  cList * connection_list;
  int id_num;
public:
  cMainEnvironment(cBaseCPU * in_cpu);
  ~cMainEnvironment();

  void AdjustTimeSlice() { population->AdjustTimeSlice(id_num); }
  void ActivateChild(sReproData * repro_data)
    { population->ActivateChild(repro_data); }
  void RotateL() { connection_list->CircPrev(); }
  void RotateR() { connection_list->CircNext(); }
  void Rotate(cBaseCPU * cpu_dir);

  int TestResources() { return TRUE; }
  int UseResources() { return TRUE; }
  
  void Notify() { population->NotifyUpdate(); }
  void NotifyError(const cString & message)
    { population->NotifyError(message); }
  void NotifyWarning(const cString & message)
    { population->NotifyWarning(message); }
  void NotifyComment(const cString & message)
    { population->NotifyComment(message); }

  cGenotype * AddGenotype(cCodeArray * in_code) 
    { return population->GetGenebank()->AddGenotype(in_code); }
  int AdjustGenotype(cGenotype * in_genotype)
    { return population->GetGenebank()->AdjustGenotype(in_genotype); }

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

  void Stats_AddExecuted(int in_exec)
    { population->GetLocalStats()->AddExecuted(in_exec); }
  void Stats_AddDeath() { population->GetLocalStats()->AddDeath(); }

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


// The test environment prevents data from going into the soup proper.

class cTestEnvironment : public cEnvironment {
private:
public:
  cTestEnvironment(cBaseCPU * in_cpu);
  ~cTestEnvironment();

  void ActivateChild(sReproData * repro_data);
  int TestResources() { return TRUE; }
  int UseResources() { return TRUE; }

  void SetConnections(cList * in_list) { (void) in_list; }
  int GetNeighborhoodSize() { return 0; }
  int ScaleMerit(const cMerit & in_merit) { (void) in_merit; return 0; }

  void SetID(cPopulation * in_population, int in_id_num)
    { (void) in_population; (void) in_id_num; }

  int OK();

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
