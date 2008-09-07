//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
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
  inline cGenotypeElement() {
    genotype = NULL;
    next = NULL;
    prev = NULL;
  }
  inline cGenotypeElement(cGenotype * in_genotype) {
    genotype = in_genotype;
    next = NULL;
    prev = NULL;
  }
  inline ~cGenotypeElement() { ; }

  inline cGenotype * GetGenotype() { return genotype; }
  inline cGenotypeElement * GetNext() { return next; }
  inline cGenotypeElement * GetPrev() { return prev; }

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

  void Insert(cGenotype * in_genotype);
  void Remove(cGenotype * in_genotype);
  cGenotype * Find(cCodeArray * in_code);
};

class cGenotypeControl {
private:
  int size;
  cGenotype * best;
  cGenotype * threads[GENOTYPE_THREADS];
  cGenebank * genebank;

  void Insert(cGenotype * in_genotype, cGenotype * prev_genotype);
  int CompareCode(cCodeArray * code1, cCodeArray * code2);
  int CheckPos(cGenotype * in_genotype);
public:
  cGenotypeControl(cGenebank * in_genebank);
  ~cGenotypeControl();

  int OK();
  void Remove(cGenotype * in_genotype);
  void Insert(cGenotype * new_genotype);
  int Adjust(cGenotype * in_genotype);

  inline int GetSize() { return size; }
  inline cGenotype * GetBest() { return best; }

  cGenotype * Find(cCodeArray * in_code);
  int FindPos(cGenotype * in_genotype, int max_depth = -1);

  inline cGenotype * Get(int thread) { return threads[thread]; }
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

  void InsertRear(cSpecies * new_species);
  void Remove(cSpecies * in_species);
  void Adjust(cSpecies * in_species);
  void Purge();
  int OK(int queue_type);

  inline int GetSize() { return size; }
  inline cSpecies * GetFirst() { return first; } 
};

class cSpeciesControl {
private:
  cSpeciesQueue active_queue;
  cSpeciesQueue inactive_queue;
  cSpeciesQueue garbage_queue;
  int threshold;              // Failures allowed to still be same species
  cGenebank * genebank;
public:
  cSpeciesControl(cGenebank * in_genebank);
  ~cSpeciesControl();

  void Remove(cSpecies * in_species);
  void Adjust(cSpecies * in_species);
  void SetInactive(cSpecies * in_species);
  void SetActive(cSpecies * in_species);
  void SetGarbage(cSpecies * in_species);
  void Purge();

  int OK();

  int FindPos(cSpecies * in_species, int max_depth = -1);
  cSpecies * Find(cGenotype * in_genotype, int record_level);

  inline cSpecies * GetFirst() { return active_queue.GetFirst(); }
  inline cSpecies * GetFirstInactive() { return inactive_queue.GetFirst(); }
  inline cSpecies * GetFirstGarbage() { return garbage_queue.GetFirst(); }
  inline int GetSize() { return active_queue.GetSize(); }
  inline int GetInactiveSize() { return inactive_queue.GetSize(); }
};

class cGenebank {
private:
  cPopulation * population;
  unsigned int genotype_count[MAX_CREATURE_SIZE];
  cGenotypeQueue active_genotypes[GENOTYPE_HASH_SIZE];
  cGenotypeControl * genotype_control;
  cSpeciesControl * species_control;
  cBaseCPU test_cpu;
  int threshold;
  int genotype_print;
  int species_print;
  int species_recording;
  int genotype_print_dom;

  int genotype_dom_time;  // Duration of current current dominant genotype.
  int prev_dom;   // Dominant genotype last update.

  cGenotype * temp_genotype;
  cString cur_label;
  void GetLabel(int in_size, int in_num);
public:
  cGenebank(cPopulation * in_population);
  ~cGenebank();

  void UpdateReset();

  cGenotype * AddGenotype(cCodeArray * in_code,
			  cSpecies * parent_species = NULL);
  void RemoveGenotype(cGenotype * in_genotype);
  void ThresholdGenotype(cGenotype * in_genotype);
  int AdjustGenotype(cGenotype * in_genotype);
  int OK();

  inline int GetSize() { return genotype_control->GetSize(); }
  inline int GetThreshold() { return threshold; }
  inline cGenotype * GetBestGenotype() { return genotype_control->GetBest(); }
  inline cSpecies * GetFirstSpecies()
    { return species_control->GetFirst(); }
  inline cPopulation * GetPopulation() { return population; }

  inline cGenotype * GetGenotype(int thread)
    { return genotype_control->Get(thread); }
  inline cGenotype * NextGenotype(int thread) {
    cGenotype * next = genotype_control->Next(thread);
    return (next == genotype_control->GetBest()) ? NULL : next;
  }
  inline cGenotype * ResetThread(int thread)
    { return genotype_control->Reset(thread); }

  int CountNumCreatures();
  inline int GetNumSpecies() { return species_control->GetSize(); }
  inline int CountSpecies() { return species_control->OK(); }
  unsigned int GetTotalMemory();
  inline int FindPos(cGenotype * in_genotype, int max_depth = -1)
    { return genotype_control->FindPos(in_genotype, max_depth); }
  inline int FindPos(cSpecies * in_species, int max_depth = -1)
    { return species_control->FindPos(in_species, max_depth); }

  unsigned int FindCRC(cCodeArray * in_code);

  void SpeciesTest(char * message, cGenotype * genotype);
};

// inline int CompareCode(cGenotype * genotype, cCodeArray * code);

#endif
