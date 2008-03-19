//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

/* geneology.hh ***************************************************************
 Geneology recording classes

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-03-09 18:07:00 travc>

 cGeneology : Handles "family trees" for genotypes
 GTHashEl : Element

 Copyright (C) 1993 - 1996 California Institute of Technology
 Read the LICENSE and README files, or contact 'charles@krl.caltech.edu',
 before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.
******************************************************************************/

#ifndef GENEOLOGY_HH
#define GENEOLOGY_HH

#include "../defs.hh"

#ifdef GENEOLOGY

// Modifying DEFINES
// #define LOG_LEAF_GENOTYPES

#include "../tools/assert.hh"
#include <iostream.h>
#include <fstream.h>

#include "../tools/tKeyList.hh"
#include "../tools/tHashTable.hh"

#include "genotype.hh"


#ifndef Boolean
#define Boolean char
#endif
#ifndef NULL
#define NULL 0
#endif
#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif


//#define GENEOLOGY_DELETES_GENOTYPES
const double GT_SAME_FIT_EPSILON = .05; // "fy=fx" if fx*(1-e)<=fy<=fx*(1+e)
const int GT_NUM_DECEND = 2;
const int GT_NUM_HASH_BINS = 2048;


// cGTElement -----------------------------------------------------------------
class cGTElement{
public:
  cGenotype * genotype;

  Boolean flag;
  int depth;            // geneotypic depth

  int birth_update;     // DOB
  int death_update;

  double fitness;

  int parent;
  tKeyList<int, int> * daughters;
  tKeyListIterator<int, int> * daughter_it;

  int parent_distance;

  int species;
  int parent_species;

  int breed_true; // # of births from this genotype to this genotype

  int num_daughter_gt;  // total number of daughter genotypes (only 1 level)
  int num_dead_daughter_gt;  // num of num_daughter_gt that are extinct

  // only valid when all daughters are dead
  int num_dead_daughter_tree;  // num of daughters that have daughters
  int sub_tree_depth;  // depth of the tree rooted at this node
  int decend[GT_NUM_DECEND]; // record of number of decendents at each level

  int breed_daughters;      // count of dead daughter's who breed (at all)

  double d_fit_sum;         // sum fitness of dead daughters
  double d_fit_sum_square;  // sum of square fitness of dead daughters

  // determined within GT_SAME_FIT_EPSILON
  int worse_fit_daughters;  // count of daughters with worse fitness
  int same_fit_daughters;   // count of daughters with same fitness
  int better_fit_daughters; // count of daughters with better fitness


  cGTElement(cGenotype* _genotype);
  ~cGTElement();

  void SetParent(int _id){ parent=_id; } // hopefully not needed
  void AddDaughter(int _id);
  void IncBreedTrue(){ ++breed_true; }
  Boolean RemoveDaughter(int _id);

  double GetFitness(){
    if( genotype!=NULL ) fitness = genotype->GetFitness();
    return fitness;
  }

  Boolean HasDaughters(){ return ( num_daughter_gt>num_dead_daughter_gt ); }
  Boolean ShouldDelete(){ return (!IsAlive() && !HasDaughters() ); }
  Boolean IsAlive(){ return ( genotype!=NULL && genotype->GetNumCPUs() ); }

  // to be called from outside (meta functions)
  Boolean KillDaughter(int _id);
  void Die(); // Called when genotype goes extinct

  void Print(ostream & out = cout);
};


// cGeneology -----------------------------------------------------------------
class cGeneology {
  typedef tHashTableIterator<int, cGTElement*> HashIterator;

public:
  ofstream debug_log; //DEBUG

private:
  cPopulation * population;
  cGenebank * genebank;

  tHashTable<int,cGTElement*> hash;
  tHashTableIterator<int,cGTElement*> hash_it;
  void PrintTreeRecurse(int root, ostream & out = cout, int depth=0);
  void PrintTrimTreeRecurse(int root, ostream & out = cout, int depth=0);

  Boolean Remove(int _id);
  void Remove(HashIterator & it);

  Boolean AddNode(cGenotype * _genotype);
  Boolean AddDaughter(int _parent_id, int _daughter_id);

  void ParentTrim(int _id);  // trimming parents when a daughter dies

  void RecordDecendants(HashIterator & pit, HashIterator & dit);

  void LogDeath(HashIterator & it);
  void LogLeafDeath(cGenotype * _genotype, HashIterator & pit);

public:
  cGeneology(cGenebank * _genebank, cPopulation * _population) :
   population(_population),
   genebank(_genebank),
   hash(GT_NUM_HASH_BINS),
   hash_it(&hash) {}

  Boolean Has(int _id){ return hash.Has(_id); }

  void AddBirth(cGenotype * _parent, cGenotype * _daughter);
  void GenotypeDeath(cGenotype * _genotype);

  void ClearFlags();

  void TrimLeaves();

  void Print(ostream & out = cout);
  void PrintDepthHistogram();
  void PrintDepthHistogram(ostream & out);
  void PrintTree(int root, ostream & out = cout);
  void PrintTrimTree(int root, ostream & out = cout);
};

#endif

#endif
