//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "phenotype.hh"
#include "../cpu/cpu.hh"
#include "../cpu/head.ii"

cPhenotype::cPhenotype(const cTaskLib * _task_lib) : 
 task_lib(_task_lib),
 task_count(_task_lib->GetNumTasks()) {
   // Initialize merits to 1.
   merit = 1;
   bonus = BONUS_BASE;
}

cPhenotype::~cPhenotype() {}

// The following function is run whenever a new creature takes over the
// CPU.  It erases all of the data from the previous creatures.
void cPhenotype::Clear(int in_size) {
  task_count.Clear();

  merit_base = in_size;
  bonus = BONUS_BASE;

  merit = (UINT) ( bonus * merit_base + .5 );
}

// The following function is run whenever a new creature takes over the
// CPU.  It erases all of the data from the previous creatures.
void cPhenotype::Clear(const sReproData & repro_data)
{
  int in_size = CalcSizeMerit(repro_data.child_memory.GetSize(),
			      repro_data.copied_size,
			      repro_data.executed_size);
  Clear(in_size);
}

void cPhenotype::Clear(const sReproData & repro_data,
		       const cMerit & parent_merit)
{
  Clear(repro_data);
  merit = parent_merit;
}

void cPhenotype::Clear(int in_size, const cMerit & parent_merit)
{
  Clear(in_size);
  merit = parent_merit;
}

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
