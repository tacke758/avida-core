/*
 *  cInheritedInstSet.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/4/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cInheritedInstSet.h"
#include "cInstSet.h"
#include "cRandom.h"
#include "cWorld.h"
#include <cassert>


cInheritedInstSet::cInheritedInstSet(const cInheritedInstSet* in) : cInstSet(* ((cInstSet*) in) )
{

  m_redundancies = in->m_redundancies;
  m_allowed_redundancies = in->m_allowed_redundancies;
}



cInheritedInstSet::cInheritedInstSet(const cInstSet* in, int init_val, const tArray< tArray<int> >& allowed_redundancies) : cInstSet(*in)
{
  m_allowed_redundancies = allowed_redundancies;
  if (init_val == 0)
    InitRedRandomly();
  else if (init_val < 0)
    InitRedByBaseInstSet();
  else
    InitRedByValue(init_val);
  Sync();
}



void cInheritedInstSet::InitRedRandomly()
{
  m_redundancies = tArray<int>(m_allowed_redundancies.GetSize(),-1);
  for (int x = 0; x < m_redundancies.GetSize(); x++)
    m_redundancies[x] = GetRandomRedundancy(x);
  return;
}



void cInheritedInstSet::InitRedByBaseInstSet()
{
  m_redundancies = tArray<int>(m_allowed_redundancies.GetSize(), -1);
  for (int id = 0; id < m_lib_name_map.GetSize(); id++){
    m_redundancies[id] = m_lib_name_map[id].redundancy;
  }
  return;
}


void cInheritedInstSet::InitRedByValue(int val)
{
  m_redundancies = tArray<int>(m_allowed_redundancies.GetSize(), -1);
  for (int x = 0; x < m_redundancies.GetSize(); x++){
    m_redundancies[x] = val;
  }
  return;
}



void cInheritedInstSet::DoMutation(eIIS_MUT_TYPE type, double p)
{
  bool did_mutate = false;
  if (type == PER_INST){
    did_mutate = MutateAllInsts(p);
  } else if (type == PER_INSTSET){
    if (RandProceed(p))
      did_mutate = MutateSingleInst();
  }
  if (did_mutate)
    Sync();
  return;
}


bool cInheritedInstSet::MutateAllInsts(double p)
{
  bool did_mutate = false;
  for (int id = 0; id < m_redundancies.GetSize(); id++)
    if (RandProceed(p)){
      m_redundancies[id] = GetRandomRedundancy(id);
      did_mutate = true;
    }
  return did_mutate;
}


bool cInheritedInstSet::MutateSingleInst()
{
  int id = m_world->GetRandom().GetUInt(0,m_redundancies.GetSize());
  m_redundancies[id] = GetRandomRedundancy(id);
  return true;
}



void cInheritedInstSet::Sync()
{
  int sum = 0;
  for (int x = 0; x < m_redundancies.GetSize(); x++)
    sum += m_redundancies[x];
  assert(sum <= 255);
  m_mutation_chart = tArray<int>(sum, -1);
  int ndx = 0;
  for (int id = 0; id < m_redundancies.GetSize(); id++)
    for (int n = 0; n < m_redundancies[id]; n++)
      m_mutation_chart[ndx++] = id;
}



inline bool cInheritedInstSet::RandProceed(double p){
  return (m_world->GetRandom().GetDouble(0,1) < p);
}



inline int cInheritedInstSet::GetRandomRedundancy(int id){
  return m_allowed_redundancies[id][m_world->GetRandom().GetUInt(0,m_allowed_redundancies[id].GetSize())];
}



