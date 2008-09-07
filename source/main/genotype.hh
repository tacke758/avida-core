//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef GENOTYPE_HH
#define GENOTYPE_HH

#include "code_array.hh"
#include "../tools/tools.hh"
#include "merit.hh"
#include "stats.hh"

#define GENOTYPE_FLAG_THRESHOLD 1

class cGenotype {
private:
  // Physical info

  cCodeArray code;
  cFlags flags;
  int id_num;
  char symbol;

  // Statistical info

  int num_CPUs;
  int total_CPUs;
  int total_parasites;
  int creatures_born;     // Number of creatures born this update.

  int tot_copied_size;
  int num_copied_size;

  int tot_exe_size;
  int num_exe_size;

  // Execution stats

  int gestation_time;
  int num_gestations;

  cLongMerit merit;
  int num_merits;

  double total_fitness;
  int num_fitness;

  // Geneological info

  int update_born;
  int parent_distance;
  int parent_id;
  cString name;
  cSpecies * species;

  // Data Structure stuff...

  cGenotype * next;
  cGenotype * prev;

public:
  cGenotype(int in_update_born = 0);
  ~cGenotype();

  int OK();
  void Mutate();
  void UpdateReset();

  void SetCode(cCodeArray * in_code);
  void SetSpecies(cSpecies * in_species);
  char GetSpeciesSymbol();

  inline void SetCopiedSize(int in_copied_size);
  inline void SetExecutedSize(int in_exe_size);
  inline void SetGestationTime(int in_gestation_time);
  inline void SetMerit(const cMerit & in_merit);
  inline void SetFitness(double in_fitness);

  void SetParent(cGenotype * parent);
  inline void SetName(cString in_name)     { name = in_name; }
  inline void SetNext(cGenotype * in_next) { next = in_next; }
  inline void SetPrev(cGenotype * in_prev) { prev = in_prev; }
  inline void SetThreshold();
  inline void SetSymbol(char in_symbol) { symbol = in_symbol; }

  inline cCodeArray * GetCode()  { return &code; }
  inline int GetLength()         { return code.GetSize(); }
  inline int GetCreaturesBorn()  { return creatures_born; }

  inline int GetCopiedSize();
  inline int GetExecutedSize();
  inline int GetGestationTime();
  inline cMerit GetMerit();
  inline double GetFitness();

  inline int GetApproxCopiedSize();
  inline int GetApproxExecutedSize();
  inline int GetApproxGestationTime();
  inline cMerit GetApproxMerit();
  inline double GetApproxFitness();

  inline int GetUpdateBorn()     { return update_born; }
  inline int GetAge();
  inline int GetParentID()       { return parent_id; }
  inline int GetParentDistance() { return parent_distance; }
  inline cString & GetName()     { return name; }
  inline cSpecies * GetSpecies() { return species; }
  inline cGenotype * GetNext()   { return next; }
  inline cGenotype * GetPrev()   { return prev; }
  inline int GetThreshold();
  inline int GetID()             { return id_num; }
  inline char GetSymbol()        { return symbol; }

  inline int AddCPU();
  inline int RemoveCPU();
  inline int AddParasite();
  inline void SwapCPU();
  inline int GetNumCPUs()        { return num_CPUs; }
  inline int GetTotalCPUs()      { return total_CPUs; }
  inline int GetTotalParasites() { return total_parasites; }

  inline int FindGeneticDistance(cGenotype * in_genotype){
    return GetCode()->FindGeneticDistance(in_genotype->GetCode()); }
};

// All the inline stuff...

  ////////////////
 //  cGenotype //
////////////////

inline void cGenotype::SetCopiedSize(int in_copied_size)
{
  if (!num_copied_size) tot_copied_size = 0;
  tot_copied_size += in_copied_size;
  num_copied_size++;
}

inline void cGenotype::SetExecutedSize(int in_exe_size)
{
  if (!num_exe_size) tot_exe_size = 0;
  tot_exe_size += in_exe_size;
  num_exe_size++;
}

inline void cGenotype::SetGestationTime(int in_gestation_time)
{
  if (!num_gestations) gestation_time = 0;
  gestation_time += in_gestation_time;
  num_gestations++;
}

inline void cGenotype::SetMerit(const cMerit & in_merit)
{
  if (!num_merits) merit.Clear();
  merit += in_merit;
  num_merits++;
}

inline void cGenotype::SetFitness(double in_fitness)
{
  if (!num_fitness) total_fitness = 0.0;
  total_fitness += in_fitness;
  num_fitness++;
}

inline void cGenotype::SetThreshold()
{
  flags.SetFlag(GENOTYPE_FLAG_THRESHOLD);
  if (symbol == '.') symbol = '+';
}

inline int cGenotype::GetCopiedSize() 
{ 
  return (num_copied_size) ? tot_copied_size  / num_copied_size : 0;
}

inline int cGenotype::GetExecutedSize()
{
  return (num_exe_size) ? tot_exe_size / num_exe_size : 0;
}

inline int cGenotype::GetGestationTime()
{
  return (num_gestations) ? gestation_time / num_gestations : 0;
}

inline cMerit cGenotype::GetMerit()
{
  if (num_merits) return  merit.GetAverage(num_merits);
  else return 1;
}

inline double cGenotype::GetFitness()
{
  return (num_fitness) ? total_fitness / (double) num_fitness : 0.0;
}

inline int cGenotype::GetApproxCopiedSize() 
{ 
  return (num_copied_size) ?
    tot_copied_size / num_copied_size : tot_copied_size;
}

inline int cGenotype::GetApproxExecutedSize()
{
  return (num_exe_size) ? tot_exe_size / num_exe_size : tot_exe_size;
}

inline int cGenotype::GetApproxGestationTime()
{
  return (num_gestations) ? gestation_time / num_gestations : gestation_time;
}

inline cMerit cGenotype::GetApproxMerit()
{
  return (num_merits) ? merit.GetAverage(num_merits) : merit.GetAverage(1);
}

inline double cGenotype::GetApproxFitness()
{
  return (num_fitness) ? total_fitness / (double) num_fitness : total_fitness;
}

inline int cGenotype::GetAge()
{
  return stats.GetUpdate() - update_born;
}

inline int cGenotype::GetThreshold()
{
  return flags.GetFlag(GENOTYPE_FLAG_THRESHOLD);
}

inline int cGenotype::AddCPU()
{
  total_CPUs++;
  creatures_born++;
  return ++num_CPUs;
}

inline int cGenotype::RemoveCPU()
{
  return --num_CPUs;
}

inline int cGenotype::AddParasite()
{
  return ++total_parasites;
}

// If a new creature is born into a CPU with the same genotype as it had.
inline void cGenotype::SwapCPU()
{
  creatures_born++;
}

#endif
