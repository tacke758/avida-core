//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef PLAYBACK_HH
#define PLAYBACK_HH

#include "population.hh"

class cPlaybackPopulation;
class cPlaybackCPU;
class cPlaybackGenotype;
class cPlaybackGenebank;

class cPlaybackPopulation : public cPopulation {
private:
  cPlaybackCPU * cpu_array;
  int world_x;
  int world_y;
  int num_creatures;

  cPlaybackGenebank * pb_genebank;
  int next_UD;
  int next_loc;
  int next_gen_id;

  FILE * fp_creatures;
  FILE * fp_genotypes;
  FILE * fp_threshold;
  FILE * fp_species;
public:
  cPlaybackPopulation(const cGenesis & in_genesis);
  ~cPlaybackPopulation();

  void DoUpdate();

  inline int GetWorldX() { return world_x; }
  inline int GetWorldY() { return world_y; }

  // virtual functions

  unsigned int GetTotalMemory() { return 0; }
  int GetNumGenotypes();
  int GetNumCreatures() { return num_creatures; }
  int GetNumSpecies() { return 0; }

  // Called from viewer...

  char * GetBasicGrid();
  char * GetResourceGrid();
  inline cPlaybackGenebank * GetPBGenebank() { return pb_genebank; }
};

class cPlaybackCPU {
private:
  int genotype_id;
  cPlaybackGenotype * genotype;
public:
  cPlaybackCPU();

  inline void SetGenotypeID(int in_id) { genotype_id = in_id; }
  inline int GetID() { return genotype_id; }

  inline void SetGenotype(cPlaybackGenotype * in_gen) { genotype = in_gen; }
  inline cPlaybackGenotype * GetGenotype() { return genotype; }
};

class cPlaybackGenotype {
private:
  int genotype_id;
  int num_CPUs;
  cFlags flags;
  char symbol;
  int position;

  cPlaybackGenotype * next;
  cPlaybackGenotype * prev;
public:
  cPlaybackGenotype(int in_id);
  ~cPlaybackGenotype();

  inline void IncCPUs() { num_CPUs++; }
  inline void DecCPUs() { num_CPUs--; }
  inline void SetFlag(int flag_num) { flags.SetFlag(flag_num); }
  inline void SetSymbol(char in_sym) { symbol = in_sym; }
  inline void SetPosition(int in_pos) { position = in_pos; }
  inline void SetNext(cPlaybackGenotype * in_next) { next = in_next; }
  inline void SetPrev(cPlaybackGenotype * in_prev) { prev = in_prev; }

  inline int GetID() { return genotype_id; }
  inline int GetNumCPUs() { return num_CPUs; }
  inline int GetFlag(int flag_num) { return flags.GetFlag(flag_num); }
  inline char GetSymbol() { return symbol; }
  inline int GetPosition() { return position; }
  inline cPlaybackGenotype * GetNext() { return next; }
  inline cPlaybackGenotype * GetPrev() { return prev; }
};

#define NUM_PB_SYMBOLS 20
#define PB_SYMBOL_THRESHOLD 18

class cPlaybackGenebank {
private:
  cPlaybackGenotype * head;
  int size;
  int threshold;
  char symbol[NUM_PB_SYMBOLS];
  cPlaybackGenotype * symbol_loc[NUM_PB_SYMBOLS];
  
  cPlaybackGenotype * Find(int id_num);
  cPlaybackGenotype * Insert(int id_num);
  void Remove(cPlaybackGenotype * in_pb_genotype);
  void ShiftNext(cPlaybackGenotype * in_pb_genotype);
  void ShiftPrev(cPlaybackGenotype * in_pb_genotype);
  
  int OK();
public:
  cPlaybackGenebank();
 
  cPlaybackGenotype * Inc(int id_num);
  void Dec(int id_num);
  void AssignSymbols();

  int GetBestID();
  inline int GetSize() { return size; }
};

#endif
