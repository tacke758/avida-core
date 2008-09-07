//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
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
  cpu = in_cpu;
  
  for (int i = 0; i < NUM_RESOURCES; i++) resource[i] = 0;

  copy_mut_rate = 0;
  point_mut_rate = 0;
  divide_mut_rate = 0;
  divide_ins_rate = 0;
  divide_del_rate = 0;
}

cEnvironment::~cEnvironment()
{
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
  copy_mut_rate   = def_environment->GetCopyMutRate();
  point_mut_rate  = def_environment->GetPointMutRate();
  divide_mut_rate = def_environment->GetDivideMutRate();
  divide_ins_rate = def_environment->GetDivideInsRate();
  divide_del_rate = def_environment->GetDivideDelRate();

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

void cTestEnvironment::ActivateChild(sReproData * repro_data)
{
  (void) repro_data;
  generation++;
  cpu->ActivateChild();
}

int cTestEnvironment::OK()
{
  int result = TRUE;
  
  if (!cEnvironment::OK()) result = FALSE;

  return result;
}
