//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "phenotype.hh"
#include "../cpu/cpu.hh"
#include "../cpu/head.ii"

cPhenotype::cPhenotype(const cTaskLib * _task_lib) :
 task_lib(_task_lib),
 task_count(_task_lib->GetNumTasks()),
 last_tasks(_task_lib->GetNumTasks()) {
   // Initialize merits to 1.
   merit = 1;
   bonus = BONUS_BASE;
   // mark unset values
   last_merit_base = -1;
   last_gestation_time = -1;
}

cPhenotype::~cPhenotype() {}

// The following function is run whenever a new creature takes over the
// CPU.  It erases all of the data from the previous creatures.
void cPhenotype::Clear( double base_merit, double initial_bonus ) {
  task_count.Clear();

  merit_base = base_merit;
  bonus = initial_bonus;

  merit = bonus * merit_base;
}

// The following function is run whenever a new creature takes over the
// CPU.  It erases all of the data from the previous creatures.
// Propagates information to "last" variables
void cPhenotype::Clear(const sReproData & repro_data)
{
  int in_size = CalcSizeMerit(repro_data.child_memory.GetSize(),
			      repro_data.copied_size,
			      repro_data.executed_size);
  assert( last_tasks.GetSize() == repro_data.tasks->GetSize() );
  for( int i=0; i<last_tasks.GetSize(); ++i ){
    last_tasks[i] = (*repro_data.tasks)[i];
  }
  last_merit_base = repro_data.parent_merit_base;
  last_gestation_time = repro_data.gestation_time;
  Clear(in_size);
}

void cPhenotype::Clear(const sReproData & repro_data,
		       const cMerit & parent_merit)
{
  Clear(repro_data);
  merit = parent_merit;
}

/* // Depricated -- @TCC
void cPhenotype::Clear(int in_size, const cMerit & parent_merit)
{
  assert(FALSE);
  Clear(in_size);
  merit = parent_merit;
}
*/


// This function is run whenever a creature executes a successful divide.
// It sets the creature up for the next gestation cycle without any change
// in what creature controls this CPU.
void cPhenotype::DivideReset(const sReproData & repro_data)
{
  cMerit last_merit = GetCurMerit();
  Clear(repro_data);
  merit = last_merit;
}


// This function runs whenever a *test* CPU divides.  It processes much of
// the information for that CPU in order to actively reflect its executed
// and copied size in its metit.

void cPhenotype::TestDivideReset(int _size, int _copied_size, int _exe_size)
{
  merit_base = CalcSizeMerit(_size, _copied_size, _exe_size);
}



///// For Loading and Saving State: /////


cMerit & cPhenotype::ReCalcMerit(){
  // Recalculate merit based on the last_task and last_merit_base
  // Does Not Handled Resources
  // Does Not Handled MAX_NUM_TASKS_REWARDED

  int i,j;

  if( last_merit_base != -1 ){ // make sure it is initialized

    double new_bonus = 1;
    double task_bonus;
    double base_bonus;   // Used for diminishing returns...
    char task_bonus_type;

    for( i=0; i<last_tasks.GetSize(); ++i ){
      for( j=1; j<=last_tasks[i]; ++j ){
	task_bonus = task_lib->GetTaskBonus(i,j-1);
	task_bonus_type = task_lib->GetTaskBonusType(i,j-1);
	
	switch( task_bonus_type ){
	  case '*': {
	    new_bonus *= 1+task_bonus;
	    break;
	  }
	  case '+': {
	    new_bonus += task_bonus;
	    break;
	  }
	  case '^': {
	    new_bonus = pow(bonus, task_bonus);
	    break;
	  }
	  case 'd': {
	    assert(j >= 2);
	    base_bonus = task_lib->GetTaskBonus(i,j-2);
	    // Do this for all remaining bonuses...

	    while (j <= last_tasks[i]) {
	      base_bonus *= task_bonus;
	      new_bonus *= 1+base_bonus;
	      j++;
	    }
	    break;
	  }
	  default: {
	    cerr<<"Unknown task bonus type \""
		<<task_bonus_type<<"\""<<endl;
	    abort();
	  }
	}
      }
    }
    merit = last_merit_base * new_bonus;
  }
  return merit;
}


int cPhenotype::SaveState(ofstream & fp){
  int i;
  assert(fp.good());
  if( !fp.good() ) return FALSE;
  fp<<"cPhenotype"<<endl;
  fp<<merit_base<<" ";
  fp<<bonus<<" ";
  fp<<merit.GetDouble()<<" ";
  for( i=0; i<GetNumTasks(); ++i ){
    fp<<GetTaskCount(i)<<" ";
  }

  fp<<last_merit_base<<" ";
  fp<<last_gestation_time<<" ";
  for( i=0; i<GetNumTasks(); ++i ){
    fp<<last_tasks[i]<<" ";
  }
  fp<<endl;
  return TRUE;
}


int cPhenotype::LoadState(ifstream & fp){
  int i;
  int tmp_count;
  double tmp_merit;
  assert(fp.good());
  if( !fp.good() ) return FALSE;

  cString foo;
  fp>>foo;
  assert(foo == "cPhenotype");

  fp>>merit_base;
  fp>>bonus;
  fp>>tmp_merit;
  for( i=0; i<GetNumTasks(); ++i ){
    fp>>tmp_count;
    task_count.SetCount(i,tmp_count);
  }

  fp>>last_merit_base;
  fp>>last_gestation_time;
  for( i=0; i<GetNumTasks(); ++i ){
    fp>>last_tasks[i];
  }
  return TRUE;
}

