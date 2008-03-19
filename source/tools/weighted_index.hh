//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2002 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef T_WEIGHTED_INDEX_HH
#define T_WEIGHTED_INDEX_HH

#include <assert.h>

#include "tArray.hh"

#ifndef NULL
#define NULL 0
#endif

/**
 * This class allows indecies to be assigned a "weight" and then indexed by
 * that weight.
 **/

class cWeightedIndex {
protected:
  int size;
  tArray<double> item_weight;
  tArray<double> subtree_weight;

  void AdjustSubtree(int id, double weight_change);
public:
  cWeightedIndex(int in_size);
  ~cWeightedIndex();

  void SetWeight(int id, double weight);
  double GetWeight(int id) { return item_weight[id]; }

  double GetTotalWeight() { return subtree_weight[0]; }
  int FindPosition(double position, int root_id=0);

  int GetParent(int id)     { return (id-1) / 2; }
  int GetLeftChild(int id)  { return 2*id + 1; }
  int GetRightChild(int id) { return 2*id + 2; }
};

#endif
