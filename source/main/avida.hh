//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef AVIDA_HH
#define AVIDA_HH

#include "../tools/assert.hh"
#include "population.hh"

void CreatePopulations(cPopulation ** pop_array, int num_pops, int in_mode);

cGenesis * ProcessConfiguration(int argc, char * argv[]);
void CollectData(cGenebank & genebank);

#endif
