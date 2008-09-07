//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "phenotype.hh"
#include "../cpu/cpu.hh"
#include "../cpu/head.ii"

cPhenotype::cPhenotype()
{
  int i;

  // Clear out the private variables...

  get_count = 0;
  put_count = 0;
  ggp_count = 0;
  flags0.Clear();
  task_flags.Clear();
  for (i = 0; i < NUM_TASKS; i++) {
    task_count[i] = 0;
  }

  // Load in all of the initialization values from the task_set file
  // (only need to do this ONCE since it will always be the same!)

  static int init = FALSE;
  static int in_get_bonus;
  static int in_put_bonus;
  static int in_ggp_bonus;
  static int in_bonus_chart[NUM_TASKS];
  if (!init) {
    cGenesis task_genesis(stats.GetTaskFilename()());
    in_get_bonus = task_genesis.ReadInt("get");
    in_put_bonus = task_genesis.ReadInt("put");
    in_ggp_bonus = task_genesis.ReadInt("ggp");
    in_bonus_chart[TASK_ECHO] = task_genesis.ReadInt("echo");
    in_bonus_chart[TASK_NOT]  = task_genesis.ReadInt("not");
    in_bonus_chart[TASK_NAND] = task_genesis.ReadInt("nand");
    in_bonus_chart[TASK_AND]  = task_genesis.ReadInt("and");
    in_bonus_chart[TASK_ORN]  = task_genesis.ReadInt("or_n");
    in_bonus_chart[TASK_OR]   = task_genesis.ReadInt("or");
    in_bonus_chart[TASK_ANDN] = task_genesis.ReadInt("and_n");
    in_bonus_chart[TASK_NOR]  = task_genesis.ReadInt("nor");
    in_bonus_chart[TASK_EQU]  = task_genesis.ReadInt("equ");
    in_bonus_chart[TASK_XOR]  = task_genesis.ReadInt("xor");
    init = TRUE;
  }

  // Initialize the bonuses.

  get_bonus = in_get_bonus;
  put_bonus = in_put_bonus;
  ggp_bonus = in_ggp_bonus;
  for (i = 0; i < NUM_TASKS; i++) {
    bonus_chart[i] = in_bonus_chart[i];
  }

  // Initialize merits to 1.

  cur_merit = 1;
  merit = 1;
}

cPhenotype::~cPhenotype()
{
}

// The following function is run whenever a new creature takes over the
// CPU.  It erases all of the data from the previous creatures.
void cPhenotype::Clear(int in_size)
{
  int i;

  get_count = 0;
  put_count = 0;
  ggp_count = 0;
  for (i = 0; i < NUM_TASKS; i++) {
    task_count[i] = 0;
  }
  flags0.Clear();
  task_flags.Clear();

  cur_merit = in_size;
  merit = in_size;
}

// The following function is run whenever a new creature takes over the
// CPU.  It erases all of the data from the previous creatures.
void cPhenotype::Clear(const sCreatureInfo & cpu_info)
{
  int in_size = 1;

  switch (stats.GetSizeMeritMethod()) {
  case SIZE_MERIT_OFF:
    in_size = 1;
    break;
  case SIZE_MERIT_COPIED:
    in_size = cpu_info.copied_size;
    break;
  case SIZE_MERIT_EXECUTED:
    in_size = cpu_info.executed_size;
    break;
  case SIZE_MERIT_FULL:
    in_size = cpu_info.active_genotype->GetLength();
    break;
  case SIZE_MERIT_LEAST:
    in_size = cpu_info.active_genotype->GetLength();
    if( in_size > cpu_info.copied_size ) 
      in_size = cpu_info.copied_size;
    if( in_size > cpu_info.executed_size ) 
      in_size = cpu_info.executed_size;
    break;
  }

  Clear(in_size);
}

void cPhenotype::Clear(const sCreatureInfo & cpu_info,
		       const cMerit & parent_merit)
{
  Clear(cpu_info);
  merit = parent_merit;
}
  
void cPhenotype::Clear(const sReproData & repro_data)
{
  int in_size = 1;

  switch (stats.GetSizeMeritMethod()) {
  case SIZE_MERIT_OFF:
    in_size = 1;
    break;
  case SIZE_MERIT_COPIED:
    in_size = repro_data.copied_size;
    break;
  case SIZE_MERIT_EXECUTED:
    in_size = repro_data.executed_size;
    break;
  case SIZE_MERIT_FULL:
    in_size = repro_data.child_memory.GetSize();
    break;
  case SIZE_MERIT_LEAST:
    in_size = repro_data.child_memory.GetSize();
    if( in_size > repro_data.copied_size ) 
      in_size = repro_data.copied_size;
    if( in_size > repro_data.executed_size ) 
      in_size = repro_data.executed_size;
    break;
  }

  Clear(in_size);
}

void cPhenotype::Clear(const sReproData & repro_data,
		       const cMerit & parent_merit)
{
  Clear(repro_data);
  merit = parent_merit;
}

// This function is run whenever a creature executes a successful divide.
// It sets the creature up for the next gestation cycle without any change
// in what creature controls this CPU.
void cPhenotype::DivideReset(const sReproData & repro_data)
{
  merit = cur_merit;

  switch (stats.GetSizeMeritMethod()) {
  case SIZE_MERIT_OFF:
    cur_merit = 1;
    break;
  case SIZE_MERIT_COPIED:
    cur_merit = repro_data.copied_size;
    break;
  case SIZE_MERIT_EXECUTED:
    cur_merit = repro_data.executed_size;
    break;
  case SIZE_MERIT_FULL:
    cur_merit = repro_data.child_memory.GetSize();
    break;
  case SIZE_MERIT_LEAST:
    cur_merit = repro_data.child_memory.GetSize();
    if( cur_merit > repro_data.copied_size ) 
      cur_merit = repro_data.copied_size;
    if( cur_merit > repro_data.executed_size ) 
      cur_merit = repro_data.executed_size;
    break;
  }

  get_count = 0;
  put_count = 0;
  ggp_count = 0;
  for (int i = 0; i < NUM_TASKS; i++) {
    task_count[i] = 0;
  }
}

void cPhenotype::SetTasks(int * input_buffer, int buf_size, int value)
{
  int i, j;

  for (i = 0; i < buf_size; i++) {
    if (value == input_buffer[i])  AssignTask(TASK_ECHO);
    if (~value == input_buffer[i]) AssignTask(TASK_NOT);
  }

  for (i = 1; i < buf_size; i++) {
    for (j = 0; j < i; j++) {
      if (value == ~(input_buffer[i] & input_buffer[j])) AssignTask(TASK_NAND);
      if (value == (input_buffer[i] & input_buffer[j]))  AssignTask(TASK_AND);
      if (value == (~input_buffer[i] | input_buffer[j]) ||
	  value == (input_buffer[i] | ~input_buffer[j])) AssignTask(TASK_ORN);
      if (value == (~input_buffer[i] & input_buffer[j]) ||
	  value == (input_buffer[i] & ~input_buffer[j])) AssignTask(TASK_ANDN);
      if (value == (input_buffer[i] | input_buffer[j]))  AssignTask(TASK_OR);
      if (value == ~(input_buffer[i] | input_buffer[j])) AssignTask(TASK_NOR);
      if (value == (input_buffer[i] ^ input_buffer[j]))  AssignTask(TASK_XOR);
      if (value == ~(input_buffer[i] ^ input_buffer[j])) AssignTask(TASK_EQU);
    }
  }
}

cMerit cPhenotype::GetGetBonus() const
{
  cMerit bonus = cur_merit;

  if (stats.GetTaskMeritMethod() == TASK_MERIT_EXPONENTIAL) {
    if (get_count && get_count <= 2) bonus.QShift();
    else if (!get_count) bonus.QShift(get_bonus);
  }
  else bonus = 0;

  return bonus;
}

cMerit cPhenotype::GetPutBonus() const
{
  cMerit bonus = cur_merit;

  if (stats.GetTaskMeritMethod() == TASK_MERIT_EXPONENTIAL) {
    if (put_count && put_count <= 2) bonus.QShift();
    else if (!put_count) bonus.QShift(put_bonus);
  }
  else bonus = 0;

  return bonus;
}

cMerit cPhenotype::GetGGPBonus() const
{
  cMerit bonus = cur_merit;

  if (stats.GetTaskMeritMethod() == TASK_MERIT_EXPONENTIAL) {
    if (!ggp_count) bonus.QShift(ggp_bonus);
  }
  else bonus = 0;

  return bonus;
}

cMerit cPhenotype::GetTaskBonus(int task_num) const
{
  cMerit bonus = cur_merit;

  if (stats.GetTaskMeritMethod() == TASK_MERIT_EXPONENTIAL) {
    if (!task_count[task_num]) bonus.QShift(bonus_chart[task_num]);
    else if (task_count[task_num] <= 2) bonus.QShift();
  }
  else bonus = 0;

  return bonus;
}
