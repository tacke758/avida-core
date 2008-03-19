//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef GENEBANK_HH
#define GENEBANK_HH

#include "../defs.hh"
#include "genotype.hh"
#include "../cpu/cpu.hh"
// #include "../cpu/head.ii"  @CAO Why is this here?

#define GENOTYPE_HASH_SIZE 307    // @CAO Should this be so high?
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

  int OK();

  void Insert(cGenotype & in_genotype);
  void Remove(cGenotype & in_genotype);
  cGenotype * Find(const cCodeArray & in_code) const;
};

class cGenotypeControl {
private:
  int size;
  cGenotype * best;
  cGenotype * threads[GENOTYPE_THREADS];
  cGenebank & genebank;

  void Insert(cGenotype & in_genotype, cGenotype * prev_genotype);
  int CompareCode(cCodeArray & code1, cCodeArray & code2);
  int CheckPos(cGenotype & in_genotype);
public:
  cGenotypeControl(cGenebank & in_gb);
  ~cGenotypeControl();

  int OK();
  void Remove(cGenotype & in_genotype);
  void Insert(cGenotype & new_genotype);
  int Adjust(cGenotype & in_genotype);

  inline int GetSize() const { return size; }
  inline cGenotype * GetBest() const { return best; }

  cGenotype * Find(const cCodeArray & in_code) const;
  int FindPos(cGenotype & in_genotype, int max_depth = -1);

  inline cGenotype * Get(int thread) const { return threads[thread]; }
  inline cGenotype * Next(int thread)
    { return threads[thread] = threads[thread]->GetNext(); }
  inline cGenotype * Reset(int thread)
    { return threads[thread] = best; }

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
  int OK(int queue_type);

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
  void Purge();

  int OK();

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
  cPopulation * population;
  unsigned int genotype_count[MAX_CREATURE_SIZE];
  cGenotypeQueue active_genotypes[GENOTYPE_HASH_SIZE];
  cGenotypeControl * genotype_control;
  cSpeciesControl * species_control;

private:
  cString GetLabel(int in_size, int in_num);

public:
  cGenebank(cPopulation * in_population);
  ~cGenebank();

  void UpdateReset();

  cGenotype * AddGenotype(const cCodeArray & in_code,
			  cSpecies * parent_species = NULL);
  cGenotype * FindGenotype(const cCodeArray & in_code) const;
  void RemoveGenotype(cGenotype & in_genotype);
  void ThresholdGenotype(cGenotype & in_genotype);
  int AdjustGenotype(cGenotype & in_genotype);

  int SaveClone(ofstream & fp);
  int LoadClone(ifstream & fp);
  int OK();

  inline int GetSize() const { return genotype_control->GetSize(); }
  inline cGenotype * GetBestGenotype() const
    { return genotype_control->GetBest(); }
  inline cSpecies * GetFirstSpecies() const
    { return species_control->GetFirst(); }
  inline cPopulation * GetPopulation() const { return population; }

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
  unsigned int GetTotalMemory();
  inline int FindPos(cGenotype & in_genotype, int max_depth = -1)
    { return genotype_control->FindPos(in_genotype, max_depth); }
  inline int FindPos(cSpecies & in_species, int max_depth = -1)
    { return species_control->FindPos(in_species, max_depth); }

  unsigned int FindCRC(const cCodeArray & in_code) const;

  void SpeciesTest(char * message, cGenotype & genotype);
};

// inline int CompareCode(cGenotype & genotype, cCodeArray & code);

#endif
