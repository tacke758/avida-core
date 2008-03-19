//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef GENOTYPE_HH
#define GENOTYPE_HH

#include "code_array.hh"
#include "../tools/tools.hh"
#include "stats.hh"

#define GENOTYPE_FLAG_THRESHOLD 1

class cMerit;

class cGenotype {
private:
  // Physical info

  cCodeArray code;
  cFlags flags;
  int id_num;
  char symbol;

  // Statistical info

  int num_CPUs;
  int last_num_CPUs;
  int total_CPUs;
  int total_parasites;

  // Birth/Beed info
  int births;      // # of creatures born    (this genotype -> other genotype)
  int breed_true;  // # of exact copy births (this genotype -> this genotype)
  int breed_in;    // # of births into type (other genotype -> this genotype)
  int last_births;      // Last reported births
  int last_breed_true;  // Last reported breed_true
  int last_breed_in;    // Last reported breed_in

  // Geneological info
  int update_born;
  int parent_id;
  int parent_distance;
  int gene_depth;           // depth in the geneological tree
  cString name;
  cSpecies * species;
  cSpecies * parent_species;

  // Data Structure stuff...
  cGenotype * next;
  cGenotype * prev;


  ////// Statistical info //////

  // Collected on Divides
  cDoubleSum sum_copied_size;
  cDoubleSum sum_exe_size;

  cDoubleSum sum_gestation_time;
  cDoubleSum sum_repro_rate;  // should make gestation obsolete (not new)

  cDoubleSum sum_merit;
  cDoubleSum sum_fitness;

  // Temporary (Approx stats used before any divides done)
  // Set in "SetParent"
  cDoubleSum tmp_sum_copied_size;
  cDoubleSum tmp_sum_exe_size;

  cDoubleSum tmp_sum_gestation_time;
  cDoubleSum tmp_sum_repro_rate;

  cDoubleSum tmp_sum_merit;
  cDoubleSum tmp_sum_fitness;


public:
  cGenotype(int in_update_born = 0);
  ~cGenotype();

  int SaveClone(ofstream & fp);
  int LoadClone(ifstream & fp);
  int OK();
  void Mutate();
  void UpdateReset();

  void SetCode(const cCodeArray & in_code);
  void SetSpecies(cSpecies * in_species);
  char GetSpeciesSymbol() const;

  void SetParent(cGenotype & parent);
  inline void SetName(cString in_name)     { name = in_name; }
  inline void SetNext(cGenotype * in_next) { next = in_next; }
  inline void SetPrev(cGenotype * in_prev) { prev = in_prev; }
  inline void SetThreshold();
  inline void SetSymbol(char in_symbol) { symbol = in_symbol; }

  // Setting New Stats
  void AddCopiedSize      (int in)   { sum_copied_size.Add(in); }
  void AddExeSize         (int in)   { sum_exe_size.Add(in); }
  void AddGestationTime   (int in)   { sum_gestation_time.Add(in);
                                       sum_repro_rate.Add(1/(double)in); }
  void AddMerit      (const cMerit & in);
  void RemoveMerit   (const cMerit & in);
  void AddFitness    (double in){ sum_fitness.Add(in); }
  void RemoveFitness (double in){ sum_fitness.Subtract(in); }


  // Setting New Stats (Should be replaced with Add Functions!!! -- TEMP)
  inline void SetCopiedSize      (int in){ AddCopiedSize(in); }
  inline void SetExecutedSize    (int in){ AddExeSize(in); }
  inline void SetGestationTime   (int in){ AddGestationTime(in); }
  inline void SetMerit(const cMerit & in){ AddMerit(in); }
  inline void SetFitness      (double in){ AddFitness(in); }

  //// Properties Native to Genotype ////
  inline cCodeArray &       GetCode()         { return code; }
  inline const cCodeArray & GetCode()   const { return code; }
  inline int                GetLength() const { return code.GetSize(); }

  inline int  GetBirths()        const { return births; }
  inline int  GetBreedTrue()     const { return breed_true; }
  inline int  GetBreedIn()       const { return breed_in; }
  inline int  GetThisBirths()    const { return births-last_births; }
  inline int  GetThisBreedTrue() const { return breed_true-last_breed_true; }
  inline int  GetThisBreedIn()   const { return breed_in-last_breed_in; }
  inline int  GetThisDeaths()    const { return last_num_CPUs - num_CPUs + GetThisBreedTrue() + GetThisBreedIn(); }
  inline int  GetLastNumCPUs()   const { return last_num_CPUs; }
  inline int  GetLastBirths()    const { return last_births; }
  inline int  GetLastBreedTrue() const { return last_breed_true; }
  inline int  GetLastBreedIn()   const { return last_breed_in; }

  inline void IncBreedIn()       { ++breed_in; }
  inline void SetLastNumCPUs()   { last_num_CPUs = num_CPUs; }
  inline void SetLastBirths()    { last_births = births; }
  inline void SetLastBreedTrue() { last_breed_true = breed_true; }
  inline void SetLastBreedIn()   { last_breed_in = breed_in; }

  inline void SetBreedStats(cGenotype & daughter); // called by ActivateChild

  //// Properties Averaged Over Creatues ////
  double GetCopiedSize()    const { return (sum_copied_size.Count()>0) ?
	   sum_copied_size.Average() : tmp_sum_copied_size.Average(); }
  double GetExecutedSize()  const { return (sum_exe_size.Count()>0) ?
	   sum_exe_size.Average() : tmp_sum_exe_size.Average(); }
  double GetGestationTime() const { return (sum_gestation_time.Count()>0) ?
	   sum_gestation_time.Average() : tmp_sum_gestation_time.Average(); }
  double GetReproRate()     const { return (sum_repro_rate.Count()>0) ?
	   sum_repro_rate.Average() : tmp_sum_repro_rate.Average(); }
  double GetMerit()         const { return (sum_merit.Count()>0) ?
	   sum_merit.Average() : tmp_sum_merit.Average(); }
  double GetFitness()       const { return (sum_fitness.Count()>0) ?
	   sum_fitness.Average() : tmp_sum_fitness.Average(); }

  // Shouldn't Be needed.... SO TEMPORARY
  double GetApproxCopiedSize()    const { return GetCopiedSize(); }
  double GetApproxExecutedSize()  const { return GetExecutedSize(); }
  double GetApproxGestationTime() const { return GetGestationTime(); }
  double GetApproxReproRate()     const { return GetReproRate(); }
  double GetApproxMerit()         const { return GetMerit(); }
  double GetApproxFitness()       const { return GetFitness(); }


  inline int GetUpdateBorn()     { return update_born; }
  inline int GetAge();
  inline int GetParentID()       { return parent_id; }
  inline int GetParentDistance() { return parent_distance; }
  inline int GetDepth() { return gene_depth; }
  inline cString & GetName()     { return name; }
  inline cSpecies * GetSpecies() { return species; }
  inline cSpecies * GetParentSpecies() { return parent_species; }
  inline cGenotype * GetNext()   { return next; }
  inline cGenotype * GetPrev()   { return prev; }
  inline int GetThreshold() const ;
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
    return GetCode().FindGeneticDistance(in_genotype->GetCode()); }
  inline int FindGeneticDistance(cCodeArray & in_code_array){
    return GetCode().FindGeneticDistance(in_code_array); }

};

// The genotype pointer template...



// All the inline stuff...

  ////////////////
 //  cGenotype //
////////////////

inline void cGenotype::SetThreshold()
{
  flags.SetFlag(GENOTYPE_FLAG_THRESHOLD);
  if (symbol == '.') symbol = '+';
}

inline int cGenotype::GetAge()
{
  return cStats::GetUpdate() - update_born;
}

inline int cGenotype::GetThreshold() const
{
  return flags.GetFlag(GENOTYPE_FLAG_THRESHOLD);
}

inline int cGenotype::AddCPU()
{
  total_CPUs++;
  return ++num_CPUs;
}

inline int cGenotype::RemoveCPU()
{
  return --num_CPUs;
}

// If a new creature is born into a CPU with the same genotype as it had.
inline void cGenotype::SwapCPU()
{
  total_CPUs++;
}

inline int cGenotype::AddParasite()
{
  return ++total_parasites;
}


inline void cGenotype::SetBreedStats(cGenotype & daughter){
  births++;
  if (daughter.id_num == id_num) {
    breed_true++;
  } else {
    daughter.IncBreedIn();
  }
}

#endif
