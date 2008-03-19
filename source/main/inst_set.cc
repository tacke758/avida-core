//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../cpu/hardware_method.hh"
#include "inst_set.hh"

#include "../tools/string_util.hh"

using namespace std;


//////////////////////
//  cInstSet
//////////////////////

// Initialize static variables
/* XXX start -- kgn */
cInstruction cInstSet::inst_default2(   0 );
cInstruction cInstSet::inst_error2  ( 255 );
/* XXX end */
const cInstruction cInstSet::inst_default(   0 );
const cInstruction cInstSet::inst_error  ( 255 );

cInstSet::cInstSet()
{
}

cInstSet::cInstSet(const cInstSet & in_inst_set)
  : inst_array(in_inst_set.inst_array)
  , nop_mods(in_inst_set.nop_mods)
  , mutation_chart(in_inst_set.mutation_chart)
{
}

cInstSet::~cInstSet()
{
}

cInstSet & cInstSet::operator=(const cInstSet & _in)
{
  /* XXX start -- kgn */
  m_inst_lib = _in.m_inst_lib;
  m_lib_name_map = _in.m_lib_name_map;
  m_lib_nopmod_map = _in.m_lib_nopmod_map;
  mutation_chart2 = _in.mutation_chart2;
  /* XXX end */
  inst_array = _in.inst_array;
  nop_mods = _in.nop_mods;
  mutation_chart = _in.mutation_chart;
  return *this;
}

bool cInstSet::OK() const
{
  //// assert(inst_array.OK());
  //// assert(nop_mods.OK());
  //assert(inst_array.GetSize() < 256);
  //assert(nop_mods.GetSize() < inst_array.GetSize());

  //// Make sure that all of the redundancies are represented the appropriate
  //// number of times.
  //tArray<int> test_redundancy(inst_array.GetSize());
  //test_redundancy.SetAll(0);
  //for (int i = 0; i < mutation_chart.GetSize(); i++) {
  //  int test_id = mutation_chart[i];
  //  test_redundancy[test_id]++;
  //}
  //for (int i = 0; i < inst_array.GetSize(); i++) {
  //  assert(inst_array[i].redundancy == test_redundancy[i]);
  //}

  /* XXX start -- kgn */
  assert(m_lib_name_map.GetSize() < 256);
  assert(m_lib_nopmod_map.GetSize() < m_lib_name_map.GetSize());

  // Make sure that all of the redundancies are represented the appropriate
  // number of times.
  tArray<int> test_redundancy2(m_lib_name_map.GetSize());
  test_redundancy2.SetAll(0);
  for (int i = 0; i < mutation_chart2.GetSize(); i++) {
    int test_id = mutation_chart[i];
    test_redundancy2[test_id]++;
  }
  for (int i = 0; i < m_lib_name_map.GetSize(); i++) {
    assert(m_lib_name_map[i].redundancy == test_redundancy2[i]);
  }
  /* XXX end */

  return true;
}

cInstruction cInstSet::GetRandomInst() const
{
  ///* XXX start -- kgn */
  //if(mutation_chart.GetSize() != mutation_chart2.GetSize()){
  //  cout << "<cInstSet::GetRandomInst> mutation_charts'.GetSize()es mismatch!" << endl;
  //}
  //unsigned int rand = g_random.GetUInt(mutation_chart.GetSize());
  //int inst_op = mutation_chart[rand];
  //int inst_op2 = mutation_chart2[rand];
  //if(inst_op != inst_op2){
  //  cout << "<cInstSet::GetRandomInst> inst_op, inst_op2 mismatch!" << endl;
  //}
  ///* XXX end */

  ////int inst_op = mutation_chart[g_random.GetUInt(mutation_chart.GetSize())];

  int inst_op = mutation_chart2[g_random.GetUInt(mutation_chart2.GetSize())];
  return cInstruction(inst_op);
}

int cInstSet::Add(const cString & _name, tHardwareMethod _fun,
		  const int redundancy, const int ft_cost, const int cost,
		  const double prob_fail)
{
  const int inst_id = inst_array.GetSize();

  assert(inst_id < 255);

  // Increase the size of the array...
  inst_array.Resize(inst_id + 1);

  // Setup the new function...
  inst_array[inst_id].name = _name;
  inst_array[inst_id].function = _fun;
  inst_array[inst_id].redundancy = redundancy;
  inst_array[inst_id].cost = cost;
  inst_array[inst_id].ft_cost = ft_cost;
  inst_array[inst_id].prob_fail = prob_fail;

  const int total_redundancy = mutation_chart.GetSize();
  mutation_chart.Resize(total_redundancy + redundancy);
  for (int i = 0; i < redundancy; i++) {
    mutation_chart[total_redundancy + i] = inst_id;
  }

  return inst_id;
}

int cInstSet::AddNop(const cString & _name, tHardwareMethod _fun,
		     const int reg, const int redundancy, const int ft_cost,
		     const int cost, const double prob_fail)
{ 
  // Assert nops are at the _beginning_ of an inst_set.
  assert(inst_array.GetSize() == nop_mods.GetSize());

  const int inst_id = Add(_name, _fun, redundancy, ft_cost, cost, prob_fail);

  nop_mods.Resize(inst_id + 1);
  nop_mods[inst_id] = reg;

  return inst_id;
}

/**/
int cInstSet::Add2(
  const int lib_fun_id,
  const int redundancy,
  const int ft_cost,
  const int cost,
  const double prob_fail
)
{
  const int inst_id = m_lib_name_map.GetSize();

  assert(inst_id < 255);

  // Increase the size of the array...
  m_lib_name_map.Resize(inst_id + 1);

  // Setup the new function...
  m_lib_name_map[inst_id].lib_fun_id = lib_fun_id;
  m_lib_name_map[inst_id].redundancy = redundancy;
  m_lib_name_map[inst_id].cost = cost;
  m_lib_name_map[inst_id].ft_cost = ft_cost;
  m_lib_name_map[inst_id].prob_fail = prob_fail;

  const int total_redundancy = mutation_chart2.GetSize();
  mutation_chart2.Resize(total_redundancy + redundancy);
  for (int i = 0; i < redundancy; i++) {
    mutation_chart2[total_redundancy + i] = inst_id;
  }

  return inst_id;
}

int cInstSet::AddNop2(
  const int lib_nopmod_id,
  const int redundancy,
  const int ft_cost,
  const int cost,
  const double prob_fail
)
{ 
  // Assert nops are at the _beginning_ of an inst_set.
  assert(m_lib_name_map.GetSize() == m_lib_nopmod_map.GetSize());

  const int inst_id = Add2(lib_nopmod_id, redundancy, ft_cost, cost, prob_fail);

  m_lib_nopmod_map.Resize(inst_id + 1);
  m_lib_nopmod_map[inst_id] = lib_nopmod_id;

  return inst_id;
}
/**/

cString cInstSet::FindBestMatch(const cString & in_name) const
{
  int best_dist = 1024;
  cString best_name("");
  
  for (int i = 0; i < inst_array.GetSize(); i++) {
    const cString & cur_name = inst_array[i].name;
    const int cur_dist = cStringUtil::EditDistance(cur_name, in_name);
    if (cur_dist < best_dist) {
      best_dist = cur_dist;
      best_name = cur_name;
    }
    if (cur_dist == 0) break;
  }

  return best_name;
}

