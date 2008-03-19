//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "mutations.hh"

#include "../tools/tools.hh"
#include <iostream>


using namespace std;


///////////////
//  cMutation
///////////////

cMutation::cMutation(const cString & _name, int _id, int _trigger,
		     int _scope, int _type, double _rate)
  : name(_name)
    , id(_id)
    , trigger(_trigger)
    , scope(_scope)
    , type(_type)
    , rate(_rate)
{
}

cMutation::~cMutation()
{
}


//////////////////
//  cMutationLib
//////////////////

cMutationLib::cMutationLib()
  : trigger_list_array(NUM_MUTATION_TRIGGERS)
{
}

cMutationLib::~cMutationLib()
{
  // Get rid of the trigger lists...  This is not required, but since we
  // are deleting the mutations after this, we should first remove all
  // references to them.
  trigger_list_array.Resize(0);

  // Now delete the actual mutations.
  for (int i = 0; i < mutation_array.GetSize(); i++) {
    delete mutation_array[i];
  }
}

cMutation * cMutationLib::AddMutation(const cString & name, int trigger,
				      int scope, int type, double rate)
{
  // Build the new mutation and drop it in the proper trigger list.
  const int id = GetSize();
  mutation_array.Resize(id+1);
  mutation_array[id] = new cMutation(name, id, trigger, scope, type, rate);
  trigger_list_array[trigger].Push(mutation_array[id]);

  return mutation_array[id];
}


/////////////////////
//  cLocalMutations
/////////////////////

cLocalMutations::cLocalMutations(const cMutationLib & _lib, int genome_length)
  : mut_lib(_lib)
  , rates(_lib.GetSize())
{
  // Setup the rates for this specifc organism.
  const tArray<cMutation *> & mut_array = mut_lib.GetMutationArray();
  for (int i = 0; i < rates.GetSize(); i++) {
    if (mut_array[i]->GetScope() == MUTATION_SCOPE_PROP ||
	mut_array[i]->GetScope() == MUTATION_SCOPE_SPREAD) {
      rates[i] = mut_array[i]->GetRate() / (double) genome_length;
    }
    else {
      rates[i] = mut_array[i]->GetRate();
    }
  }

  // Setup the mutation count array.
  counts.Resize(mut_lib.GetSize(), 0);
}

cLocalMutations::~cLocalMutations()
{
}


////////////////////
//  cMutationRates
////////////////////

cMutationRates::cMutationRates()
{
  Clear();
}

cMutationRates::cMutationRates(const cMutationRates & in_muts)
{
  Copy(in_muts);
}

cMutationRates::~cMutationRates()
{
}

void cMutationRates::Clear()
{
  exec.point_mut_prob = 0.0;
  copy.copy_mut_prob = 0.0;
  divide.ins_mut_prob = 0.0;
  divide.del_mut_prob = 0.0;
  divide.div_mut_prob = 0.0;
  divide.divide_mut_prob = 0.0;
  divide.divide_ins_prob = 0.0;
  divide.divide_del_prob = 0.0;
  divide.parent_mut_prob = 0.0;
  divide.crossover_prob = 0.0;
  divide.aligned_cross_prob = 0.0;
}

void cMutationRates::Copy(const cMutationRates & in_muts)
{
  exec.point_mut_prob = in_muts.exec.point_mut_prob;
  copy.copy_mut_prob = in_muts.copy.copy_mut_prob;
  divide.ins_mut_prob = in_muts.divide.ins_mut_prob;
  divide.del_mut_prob = in_muts.divide.del_mut_prob;
  divide.div_mut_prob = in_muts.divide.div_mut_prob;
  divide.divide_mut_prob = in_muts.divide.divide_mut_prob;
  divide.divide_ins_prob = in_muts.divide.divide_ins_prob;
  divide.divide_del_prob = in_muts.divide.divide_del_prob;
  divide.parent_mut_prob = in_muts.divide.parent_mut_prob;
  divide.crossover_prob = in_muts.divide.crossover_prob;
  divide.aligned_cross_prob = in_muts.divide.aligned_cross_prob;

  //  if (copy.copy_mut_prob != 0) cerr << "Copying non-zero copy mut rate!" << endl;
}

bool cMutationRates::TestPointMut() const
{
  return g_random.P(exec.point_mut_prob);
}

bool cMutationRates::TestCopyMut() const
{
  return g_random.P(copy.copy_mut_prob);
}

bool cMutationRates::TestDivideMut() const
{
  return g_random.P(divide.divide_mut_prob);
}

bool cMutationRates::TestDivideIns() const
{
  return g_random.P(divide.divide_ins_prob);
}

bool cMutationRates::TestDivideDel() const
{
  return g_random.P(divide.divide_del_prob);
}

bool cMutationRates::TestParentMut() const
{
  return g_random.P(divide.parent_mut_prob);
}

bool cMutationRates::TestCrossover() const
{
  return g_random.P(divide.crossover_prob);
}

bool cMutationRates::TestAlignedCrossover() const
{
  return g_random.P(divide.aligned_cross_prob);
}
