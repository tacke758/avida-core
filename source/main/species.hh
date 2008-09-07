//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef SPECIES_HH
#define SPECIES_HH

#include "code_array.hh"
#include "../tools/tools.hh"

#define SPECIES_FLAG_COPY_TRUE 0

#define SPECIES_QUEUE_NONE     0
#define SPECIES_QUEUE_ACTIVE   1
#define SPECIES_QUEUE_INACTIVE 2
#define SPECIES_QUEUE_GARBAGE  3

#define SPECIES_MAX_DISTANCE 20

class cSpecies {
private:
  int id_num;
  int parent_id;
  cCodeArray code;
  int update_born;

  int total_creatures;
  int total_genotypes;
  int num_threshold;
  int num_genotypes;
  int num_creatures;
  int queue_type;
  char symbol;
  cFlags flags;
  int genotype_distance[SPECIES_MAX_DISTANCE];

  cSpecies * next;
  cSpecies * prev;
public:
  cSpecies(cCodeArray * in_code);
  ~cSpecies();

  int Compare(cCodeArray * test_code, int max_fail_count);
  int OK();

  void AddThreshold(cGenotype * in_genotype);
  void RemoveThreshold(cGenotype * in_genotype);
  void AddGenotype();
  void RemoveGenotype();

  inline void AddCreatures(int in_num) { num_creatures += in_num; }
  inline void ResetStats() { num_creatures = 0; }

  inline cSpecies * GetNext() { return next; }
  inline cSpecies * GetPrev() { return prev; }
  inline int GetNumGenotypes() { return num_genotypes; }
  inline int GetNumThreshold() { return num_threshold; }
  inline int GetNumCreatures() { return num_creatures; }
  inline int GetTotalCreatures() { return total_creatures; }
  inline int GetTotalGenotypes() { return total_genotypes; }
  inline int GetQueueType() { return queue_type; }
  inline int GetID() { return id_num; }
  inline char GetSymbol() { return symbol; }
  inline int GetAge() { return stats.GetUpdate() - update_born; }
  inline int GetParentID() { return parent_id; }

  inline void SetQueueType(int in_qt) { queue_type = in_qt; }
  inline void SetNext(cSpecies * in_next) { next = in_next; }
  inline void SetPrev(cSpecies * in_prev) { prev = in_prev; }
  inline void SetSymbol(char in_symbol) { symbol = in_symbol; }
  inline void SetParentID(int in_id) { parent_id = in_id; }

#ifdef DEBUG
  // These are used in cGenebank::OK()
  int debug_num_genotypes;
  int debug_num_threshold;
#endif
};

#endif
