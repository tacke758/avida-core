//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef INJECT_GENEBANK_HH
#define INJECT_GENEBANK_HH

#include "../tools/string.hh"
#include "../defs.hh"

// porting to gcc 3.1 -- k
//class std::ofstream;
//class std::ifstream;

class cInjectGenebank;
class cInjectGenotype;
class cGenome;
class cSpecies;
class cStats;

#define INJECTGENOTYPE_HASH_SIZE 307    // @CAO Is this an optimal number?
#define SPECIES_HASH_SIZE 101
#define INJECTGENOTYPE_THREADS 2

#define SPECIES_RECORD_OFF     0
#define SPECIES_RECORD_FULL    1
#define SPECIES_RECORD_LIMITED 2

class cInjectGenotypeElement {
private:
  cInjectGenotype * inject_genotype;
  cInjectGenotypeElement * next;
  cInjectGenotypeElement * prev;
public:
  inline cInjectGenotypeElement(cInjectGenotype * in_gen=NULL) : inject_genotype(in_gen) {
    next = NULL;  prev = NULL;
  }
  inline ~cInjectGenotypeElement() { ; }

  inline cInjectGenotype * GetInjectGenotype() const { return inject_genotype; }
  inline cInjectGenotypeElement * GetNext() const { return next; }
  inline cInjectGenotypeElement * GetPrev() const { return prev; }

  inline void SetNext(cInjectGenotypeElement * in_next) { next = in_next; }
  inline void SetPrev(cInjectGenotypeElement * in_prev) { prev = in_prev; }
};

class cInjectGenotypeQueue {
private:
  int size;
  cInjectGenotypeElement root;

  void Remove(cInjectGenotypeElement * in_element);
public:
  cInjectGenotypeQueue();
  ~cInjectGenotypeQueue();

  bool OK();

  void Insert(cInjectGenotype & in_inject_genotype);
  void Remove(cInjectGenotype & in_inject_genotype);
  cInjectGenotype * Find(const cGenome & in_genome) const;
};

class cInjectGenotypeControl {
private:
  int size;
  cInjectGenotype * best;
  cInjectGenotype * coalescent;
  cInjectGenotype * threads[INJECTGENOTYPE_THREADS];
  cInjectGenebank & genebank;

  cInjectGenotype * historic_list;
  int historic_count;

  void Insert(cInjectGenotype & in_inject_genotype, cInjectGenotype * prev_inject_genotype);
  bool CheckPos(cInjectGenotype & in_inject_genotype);
public:
  cInjectGenotypeControl(cInjectGenebank & in_gb);
  ~cInjectGenotypeControl();

  bool OK();
  void Remove(cInjectGenotype & in_inject_genotype);
  void Insert(cInjectGenotype & new_inject_genotype);
  bool Adjust(cInjectGenotype & in_inject_genotype);

  void RemoveHistoric(cInjectGenotype & in_inject_genotype);
  void InsertHistoric(cInjectGenotype & in_inject_genotype);
  int GetHistoricCount() { return historic_count; }

  int UpdateCoalescent();

  inline int GetSize() const { return size; }
  inline cInjectGenotype * GetBest() const { return best; }
  inline cInjectGenotype * GetCoalescent() const { return coalescent; }

  cInjectGenotype * Find(const cGenome & in_genome) const;
  int FindPos(cInjectGenotype & in_inject_genotype, int max_depth = -1);

  inline cInjectGenotype * Get(int thread) const { return threads[thread]; }
  inline cInjectGenotype * Reset(int thread)
    { return threads[thread] = best; }
  inline cInjectGenotype * ResetHistoric(int thread)
    { return threads[thread] = historic_list; }
  cInjectGenotype * Next(int thread);
  cInjectGenotype * Prev(int thread);
};

class cInjectGenebank {
private:
  unsigned int inject_genotype_count[MAX_CREATURE_SIZE];
  cInjectGenotypeQueue active_inject_genotypes[INJECTGENOTYPE_HASH_SIZE];
  cInjectGenotypeControl * inject_genotype_control;
  cStats & stats;

private:
  cString GetLabel(int in_size, int in_num);

public:
  cInjectGenebank(cStats & stats);
  ~cInjectGenebank();

  void UpdateReset();

  /** 
   * This function can be used to add a injectgenotype that was created
   * outside the genebank. In this case, the parameter in_list_num
   * should not be given. Normally, injectgenotypes are added through the 
   * function AddInjectGenotype(const cGenome & in_genome, 
   * cInjectGenotype * parent_injectgenotype = NULL), which then calls this one.
   **/
  void AddInjectGenotype(cInjectGenotype *in_inject_genotype, int in_list_num = -1 );
  cInjectGenotype * AddInjectGenotype(const cGenome & in_genome,
			  cInjectGenotype * parent_inject_genotype = NULL);
  cInjectGenotype * FindInjectGenotype(const cGenome & in_genome) const;
  void RemoveInjectGenotype(cInjectGenotype & in_inject_genotype);
  void ThresholdInjectGenotype(cInjectGenotype & in_inject_genotype);
  bool AdjustInjectGenotype(cInjectGenotype & in_inject_genotype);

  bool SaveClone(std::ofstream & fp);
  bool LoadClone(std::ifstream & fp);
  bool DumpTextSummary(std::ofstream & fp);
  //bool DumpDetailedSummary(std::ofstream & fp);
  bool DumpDetailedSummary(const cString & file, int update);
  bool DumpHistoricSummary(std::ofstream & fp);
  //void DumpDetailedEntry(cInjectGenotype * inject_genotype, std::ofstream & fp);
  void DumpDetailedEntry(cInjectGenotype * inject_genotype, const cString & file, int update);
  bool OK();

  inline int GetSize() const { return inject_genotype_control->GetSize(); }
  inline cInjectGenotype * GetBestInjectGenotype() const
    { return inject_genotype_control->GetBest(); }
  inline cInjectGenotype * GetCoalescentInjectGenotype() const
    { return inject_genotype_control->GetCoalescent(); }
  
  inline cInjectGenotype * GetInjectGenotype(int thread) const
    { return inject_genotype_control->Get(thread); }
  inline cInjectGenotype * NextInjectGenotype(int thread) {
    cInjectGenotype * next = inject_genotype_control->Next(thread);
    return (next == inject_genotype_control->GetBest()) ? (cInjectGenotype*)NULL : next;
  }
  inline cInjectGenotype * ResetThread(int thread)
    { return inject_genotype_control->Reset(thread); }

  int CountNumCreatures();
  inline int FindPos(cInjectGenotype & in_inject_genotype, int max_depth = -1)
    { return inject_genotype_control->FindPos(in_inject_genotype, max_depth); }
   unsigned int FindCRC(const cGenome & in_genome) const;
};

#endif
