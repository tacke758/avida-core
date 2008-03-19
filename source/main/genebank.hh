//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef GENEBANK_HH
#define GENEBANK_HH

#include "../tools/string.hh"
#include "../defs.hh"

// porting to gcc 3.1 -- k
//class std::ofstream;
//class std::ifstream;

class cGenebank;
class cGenotype;
class cGenome;
class cSpecies;
class cStats;

#define GENOTYPE_HASH_SIZE 307    // @CAO Is this an optimal number?
#define SPECIES_HASH_SIZE 101
#define GENOTYPE_THREADS 2

#define SPECIES_RECORD_OFF     0
#define SPECIES_RECORD_FULL    1
#define SPECIES_RECORD_LIMITED 2

class cGenotypeElement {
private:
  cGenotype * genotype;
  cGenotypeElement * next;
  cGenotypeElement * prev;
public:
  inline cGenotypeElement(cGenotype * in_gen=NULL) : genotype(in_gen) {
    next = NULL;  prev = NULL;
  }
  inline ~cGenotypeElement() { ; }

  inline cGenotype * GetGenotype() const { return genotype; }
  inline cGenotypeElement * GetNext() const { return next; }
  inline cGenotypeElement * GetPrev() const { return prev; }

  inline void SetNext(cGenotypeElement * in_next) { next = in_next; }
  inline void SetPrev(cGenotypeElement * in_prev) { prev = in_prev; }
};

class cGenotypeQueue {
private:
  int size;
  cGenotypeElement root;

  void Remove(cGenotypeElement * in_element);
public:
  cGenotypeQueue();
  ~cGenotypeQueue();

  bool OK();

  void Insert(cGenotype & in_genotype);
  void Remove(cGenotype & in_genotype);
  cGenotype * Find(const cGenome & in_genome) const;
};

class cGenotypeControl {
private:
  int size;
  cGenotype * best;
  cGenotype * coalescent;
  cGenotype * threads[GENOTYPE_THREADS];
  cGenebank & genebank;

  cGenotype * historic_list;
  int historic_count;

  void Insert(cGenotype & in_genotype, cGenotype * prev_genotype);
  bool CheckPos(cGenotype & in_genotype);
public:
  cGenotypeControl(cGenebank & in_gb);
  ~cGenotypeControl();

  bool OK();
  void Remove(cGenotype & in_genotype);
  void Insert(cGenotype & new_genotype);
  bool Adjust(cGenotype & in_genotype);

  void RemoveHistoric(cGenotype & in_genotype);
  void InsertHistoric(cGenotype & in_genotype);
  int GetHistoricCount() { return historic_count; }

  int UpdateCoalescent();

  inline int GetSize() const { return size; }
  inline cGenotype * GetBest() const { return best; }
  inline cGenotype * GetCoalescent() const { return coalescent; }

  cGenotype * Find(const cGenome & in_genome) const;
  int FindPos(cGenotype & in_genotype, int max_depth = -1);

  inline cGenotype * Get(int thread) const { return threads[thread]; }
  inline cGenotype * Reset(int thread)
    { return threads[thread] = best; }
  inline cGenotype * ResetHistoric(int thread)
    { return threads[thread] = historic_list; }
  cGenotype * Next(int thread);
  cGenotype * Prev(int thread);
};

class cSpeciesQueue {
private:
  int size;
  cSpecies * first;
public:
  cSpeciesQueue();
  ~cSpeciesQueue();

  void InsertRear(cSpecies & new_species);
  void Remove(cSpecies & in_species);
  void Adjust(cSpecies & in_species);
  void Purge();
  bool OK(int queue_type);

  inline int GetSize() const { return size; }
  inline cSpecies * GetFirst() const { return first; }
};

class cSpeciesControl {
private:
  cSpeciesQueue active_queue;
  cSpeciesQueue inactive_queue;
  cSpeciesQueue garbage_queue;
  cGenebank & genebank;
public:
  cSpeciesControl(cGenebank & in_gb);
  ~cSpeciesControl();

  void Remove(cSpecies & in_species);
  void Adjust(cSpecies & in_species);
  void SetInactive(cSpecies & in_species);
  void SetActive(cSpecies & in_species);
  void SetGarbage(cSpecies & in_species);
  void Purge(cStats & stats);

  bool OK();

  int FindPos(cSpecies & in_species, int max_depth = -1);
  cSpecies * Find(cGenotype & in_genotype, int record_level);

  inline cSpecies * GetFirst() const { return active_queue.GetFirst(); }
  inline cSpecies * GetFirstInactive() const
    { return inactive_queue.GetFirst(); }
  inline cSpecies * GetFirstGarbage() const
    { return garbage_queue.GetFirst(); }
  inline int GetSize() const { return active_queue.GetSize(); }
  inline int GetInactiveSize() const { return inactive_queue.GetSize(); }
};


class cGenebank {
private:
  unsigned int genotype_count[MAX_CREATURE_SIZE];
  cGenotypeQueue active_genotypes[GENOTYPE_HASH_SIZE];
  cGenotypeControl * genotype_control;
  cSpeciesControl * species_control;
  cStats & stats;

private:
  cString GetLabel(int in_size, int in_num);

public:
  cGenebank(cStats & _stats);
  ~cGenebank();

  void UpdateReset();

  /** 
   * This function can be used to add a genotype that was created
   * outside the genebank. In this case, the parameter in_list_num
   * should not be given. Normally, genotypes are added through the 
   * function AddGenotype(const cGenome & in_genome, 
   * cGenotype * parent_genotype = NULL), which then calls this one.
   **/
  void AddGenotype(cGenotype *in_genotype, int in_list_num = -1 );
  cGenotype * AddGenotype(const cGenome & in_genome,
			  cGenotype * parent_genotype = NULL);
  cGenotype * FindGenotype(const cGenome & in_genome) const;
  void RemoveGenotype(cGenotype & in_genotype);
  void ThresholdGenotype(cGenotype & in_genotype);
  bool AdjustGenotype(cGenotype & in_genotype);

  bool SaveClone(std::ofstream & fp);
  bool LoadClone(std::ifstream & fp);
  bool DumpTextSummary(std::ofstream & fp);
  bool DumpDetailedSummary(std::ofstream & fp);
  bool DumpHistoricSummary(std::ofstream & fp);
  void DumpDetailedEntry(cGenotype * genotype, std::ofstream & fp);
  bool OK();

  inline int GetSize() const { return genotype_control->GetSize(); }
  inline cGenotype * GetBestGenotype() const
    { return genotype_control->GetBest(); }
  inline cGenotype * GetCoalescentGenotype() const
    { return genotype_control->GetCoalescent(); }
  inline cSpecies * GetFirstSpecies() const
    { return species_control->GetFirst(); }

  inline cGenotype * GetGenotype(int thread) const
    { return genotype_control->Get(thread); }
  inline cGenotype * NextGenotype(int thread) {
    cGenotype * next = genotype_control->Next(thread);
    return (next == genotype_control->GetBest()) ? (cGenotype*)NULL : next;
  }
  inline cGenotype * ResetThread(int thread)
    { return genotype_control->Reset(thread); }

  int CountNumCreatures();
  inline int GetNumSpecies() const { return species_control->GetSize(); }
  inline int CountSpecies() { return species_control->OK(); }
  inline int FindPos(cGenotype & in_genotype, int max_depth = -1)
    { return genotype_control->FindPos(in_genotype, max_depth); }
  inline int FindPos(cSpecies & in_species, int max_depth = -1)
    { return species_control->FindPos(in_species, max_depth); }

  unsigned int FindCRC(const cGenome & in_genome) const;

  void SpeciesTest(char * message, cGenotype & genotype);
};

#endif
