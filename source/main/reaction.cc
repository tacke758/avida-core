//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////


// porting to gcc 3.1 -- k

#include "reaction.hh"


using namespace std;


//////////////////////
//  cReactionProcess
//////////////////////

cReactionProcess::cReactionProcess()
  : resource(NULL)
  , value(1.0)
  , type(REACTION_PROCTYPE_ADD)
  , max_number(1.0)
  , min_number(0.0)
  , max_fraction(1.0)
  , product(NULL)
  , conversion(1.0)
{
}

cReactionProcess::~cReactionProcess()
{
}


////////////////////////
//  cReactionRequisite
////////////////////////

cReactionRequisite::cReactionRequisite()
  : min_task_count(0)
  , max_task_count(INT_MAX)
{
}

cReactionRequisite::~cReactionRequisite()
{
}


///////////////
//  cReaction
///////////////

cReaction::cReaction(const cString & _name, int _id)
  : name(_name)
  , id(_id)
  , task(NULL)
{
}

cReaction::~cReaction()
{
  while (process_list.GetSize() != 0) delete process_list.Pop();
  while (requisite_list.GetSize() != 0) delete requisite_list.Pop();
}

cReactionProcess * cReaction::AddProcess()
{
  cReactionProcess * new_process = new cReactionProcess();
  process_list.PushRear(new_process);
  return new_process;
}

cReactionRequisite * cReaction::AddRequisite()
{
  cReactionRequisite * new_requisite = new cReactionRequisite();
  requisite_list.PushRear(new_requisite);
  return new_requisite;
}



//////////////////
//  cReactionLib
//////////////////

cReactionLib::~cReactionLib()
{
  for (int i = 0; i < reaction_array.GetSize(); i++) {
    delete reaction_array[i];
  }
}


cReaction * cReactionLib::GetReaction(const cString & name) const
{
  for (int i = 0; i < reaction_array.GetSize(); i++) {
    if (reaction_array[i]->GetName() == name) return reaction_array[i];
  }
  return NULL;
}

cReaction * cReactionLib::GetReaction(int id) const
{
  return reaction_array[id];
}


cReaction * cReactionLib::AddReaction(const cString & name)
{
  // If this reaction already exists, just return it.
  cReaction * found_reaction = GetReaction(name);
  if (found_reaction != NULL) return found_reaction;

  // Create a new reaction...
  const int new_id = reaction_array.GetSize();
  cReaction * new_reaction = new cReaction(name, new_id);
  reaction_array.Resize(new_id + 1);
  reaction_array[new_id] = new_reaction;
  return new_reaction;
}
