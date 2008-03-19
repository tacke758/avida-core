//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../defs.hh"
#include "environment.hh"
#include "head.ii"
#include "cpu.ii"


//////////////////
//  cEnvironment
//////////////////

cEnvironment::cEnvironment(cBaseCPU * in_cpu)
{
  generation = 0;
  creature_count = 0;
  cpu = in_cpu;
  
  for (int i = 0; i < NUM_RESOURCES; i++) resource[i] = 0;

  point_mut_prob = 0.0;
  copy_mut_prob = 0.0;
  ins_mut_prob = 0.0;
  del_mut_prob = 0.0;
  divide_mut_prob = 0.0;
  divide_ins_prob = 0.0;
  divide_del_prob = 0.0;
  crossover_prob = 0.0;
  aligned_cross_prob = 0.0;
  exe_err_prob = 0.0;
}

cEnvironment::~cEnvironment()
{
}


// By default, this method just loops within the main creature.
cCPUHead cEnvironment::GetHeadPosition(cBaseCPU * cur_cpu, int offset)
{
  if (offset > cur_cpu->GetMemorySize()) offset -= cur_cpu->GetMemorySize();
  offset %= cpu->GetMemorySize();
  if (offset < 0) offset += cpu->GetMemorySize();
  return cCPUHead(cpu, offset);
}

int cEnvironment::OK()
{
  return TRUE;
}


//////////////////////
//  cMainEnvironment
//////////////////////

cMainEnvironment::cMainEnvironment(cBaseCPU * in_cpu) : cEnvironment(in_cpu)
{
  connection_list = NULL;
}

cMainEnvironment::~cMainEnvironment()
{
  if (connection_list) delete connection_list;
}

void cMainEnvironment::Rotate(cBaseCPU * cpu_dir)
{
  // @CAO Should add some error checking to make sure this doesn't go on
  // forever...

  while ((cBaseCPU *) connection_list->GetCurrent() != cpu_dir) {
    connection_list->CircNext();
  }
}

cCPUHead cMainEnvironment::GetHeadPosition(cBaseCPU * cur_cpu, int offset)
{
  return population->GetHeadPosition(cur_cpu, offset);
}

cBaseCPU * cMainEnvironment::GetNeighbor()
{
  // If a neighborhood exists, pick a random neighbor.  Else, pick anyone.

  if (connection_list == NULL) {
    return population->GetRandomCPU();
  }
  else {
    int neighbor = g_random.GetUInt(connection_list->GetSize());
    return (cBaseCPU *) connection_list->Get(neighbor);
  }
}

void cMainEnvironment::SetID(cPopulation * in_population, int in_id_num)
{
  population = in_population;
  id_num = in_id_num;

  cEnvironment * def_environment = in_population->GetDefaultEnvironment();

  // Get information about the environment.
  point_mut_prob  = def_environment->GetPointMutProb();
  copy_mut_prob   = def_environment->GetCopyMutProb();
  ins_mut_prob    = def_environment->GetInsMutProb();
  del_mut_prob    = def_environment->GetDelMutProb();
  copy_mut_prob   = def_environment->GetCopyMutProb();
  divide_mut_prob = def_environment->GetDivideMutProb();
  divide_ins_prob = def_environment->GetDivideInsProb();
  divide_del_prob = def_environment->GetDivideDelProb();
  crossover_prob  = def_environment->GetCrossoverProb();
  aligned_cross_prob = def_environment->GetAlignedCrossProb();
  exe_err_prob    = def_environment->GetExeErrProb();

  for (int i = 0; i < NUM_RESOURCES; i++) {
    resource[i] = def_environment->GetResource(i);
  }
}

int cMainEnvironment::OK()
{
  int result = TRUE;

  if (id_num < -1 || id_num > population->GetSize()) {
    g_debug.Warning("CPU ID is out of range!");
    result = FALSE;
  }
  
  if (!cEnvironment::OK()) result = FALSE;

  return result;
}


//////////////////////
//  cTestEnvironment
//////////////////////

cTestEnvironment::cTestEnvironment(cBaseCPU * in_cpu) : cEnvironment(in_cpu)
{
}

cTestEnvironment::~cTestEnvironment()
{
}

void cTestEnvironment::ActivateChild(sReproData & repro_data)
{
  (void) repro_data;
  generation++;
  creature_count++;
  cpu->ActivateChild();
}


int cTestEnvironment::OK()
{
  int result = TRUE;
  
  if (!cEnvironment::OK()) result = FALSE;

  return result;
}
