//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* task.hh *******************************************************************
 Tasks library and associated classes
 Time-stamp: <2000-03-23 16:22:53 travc>

 cTaskLib::
  -- Initialization --
  Load(const cString & filename)

  -- Access Task Info --
  int GetNumTasks() const
  const cString & GetTaskName(const int i) const
  char GetTaskType(const int i) const
  int GetTaskNumBonus(const int i) const
  double GetTaskBonus(const int task_num, int bonus_num) const

  -- Called by pheontype on particular events --
  double TestInput(const int _input, cTaskCount & task_count,
		   const double in_bonus) const
  double TestOutput(const int _output, cTaskCount & task_count,
		     const double in_bonus) const

 Notes:

  Adding new Type (Task group type)
    insert a variable in the class declaration like "task_group_i"
    insert a line in ~cTaskLib to DeleteTaskGroupList
    add to case in AddTaskToGroup
    add function like TestInput or TestOutput
    have that test function called whenever it should be triggered

*****************************************************************************/

#ifndef TASKS_HH
#define TASKS_HH

#include "../defs.hh"
#include "../tools/assert.hh"
#include "../tools/string.hh"
#include "../tools/tArray.hh"
#include "task_buffer.hh"


// ************ Resource Manager **************
class cResource {
private:
public:
  int id;
  double conc;
  double init_conc;
  double inflow;
  double outflow;
};



// ************ Task defines and types**************
class cTaskLib;
class cTaskEntry;
class cTaskGroup;
typedef void (cTaskLib::*tTaskMethod)(int * done_tasks) const;



class cTaskGroup {
friend class cTaskLib;
private:
  cString name;
  int num_tasks;
  cTaskEntry ** task;
  tTaskMethod test;
  int num_return_values;
  cTaskGroup * next;

public:
  cTaskGroup(const cString & _name, const int _num_tasks,
	     const tTaskMethod _test, const int _num_return_values,
	     cTaskGroup * _next);
  ~cTaskGroup(){ ; }

  void Perturb(double max_factor);
  void Change(double prob_change, double min_bonus, double max_bonus);

  const cString &    GetName() const { return name; }
  int                GetNumTasks() const { return num_tasks; }
  const tTaskMethod  GetTest() const { return test; }
  int                GetNumReturnValues() const { return num_return_values; }
  const cTaskGroup * GetNext() const { return next; }

  const cTaskEntry * operator[] (const int i) const {
    assert( i >= 0  &&  i < num_tasks );
    return task[i]; }
  void SetTask(const int i, const cTaskEntry & _task){
    assert( i >= 0  &&  i < num_tasks );
    task[i] = const_cast(cTaskEntry*, &_task); }

};


class cTaskEntry {
private:
  cString name;
  cString keyword;  // For human-understandable output...
  int id;
  const cTaskGroup * group;
  int num_bonus;
  char * bonus_type;
  double * bonus;

public:
  tArray<bool> * resource_dependency;
  tArray<double> * resource_effect;

  cTaskEntry() : name("uninitialized"), keyword("[NONE]"), id(-1),
    group(NULL), num_bonus(0), bonus_type(NULL), bonus(NULL),
    resource_dependency(NULL), resource_effect(NULL) {;}
  ~cTaskEntry() {
    if( bonus )  delete [] bonus;
    if( resource_dependency )  delete resource_dependency;
    if( resource_effect )  delete resource_effect; }

  void Set(const cString & _name, const int id, const int _num_bonus,
	   const char * _bonus_type, const double * _bonus);
  void SetKeyword(const cString & _key) { keyword = _key; }
  void SetGroup(const cTaskGroup * _group){ group = _group; }

  void Perturb(double max_factor);
  void Change(double prob_change, double min_bonus, double max_bonus);

  const cString &    GetName()       const { return name; }
  const cString &    GetKeyword()    const { return keyword; }
  const int          GetID()         const { return id; }
  const cTaskGroup * GetGroup()      const {assert(group!=NULL); return group;}
  int                GetNumBonus()   const { return num_bonus; }

  double             GetBonus(int i) const {
    if( i >= num_bonus )  i=num_bonus-1;  // Last bonus is for all thereafter
    assert(i>=0 && i<num_bonus);  return bonus[i]; }
  char               GetBonusType(int i) const {
    if( i >= num_bonus )  i=num_bonus-1;  // Last bonus is for all thereafter
    assert(i>=0 && i<num_bonus);  return bonus_type[i]; }
};



class cTaskLib {
private:
  int num_tasks;
  cTaskEntry * task;  // Array of tasks

  cTaskGroup * task_group_i;  // Single Linked List of task group pointers
  cTaskGroup * task_group_o;  // Single Linked List of task group pointers

  const cIOBuf * cur_inputs;
  const cIOBuf * cur_outputs;
  const cTaskGroup * cur_group;

  tArray<cResource> * resource;

public:
  cTaskLib();
  ~cTaskLib();

  void Load(cString filename);
  void Perturb(double max_factor);
  void Change(double prob_change, double min_bonus, double max_bonus);
  void PrintBonus(ofstream & fp) const;

  void LoadResources(const cString & filename);
  void UpdateResources();
  void PrintResourceData(const cString & filename);

  // Access task info
  int GetNumTasks() const { return num_tasks; }

  const cString GetTaskName(const int i) const;
  const cString & GetTaskKeyword(const int i) const;

  const cString & GetTaskGroupName(const int i) const {
    assert( i>=0 && i<num_tasks );
    assert( task[i].GetGroup() != NULL );
    return task[i].GetGroup()->GetName(); }

  int GetTaskNumBonus(const int i) const {
    assert( i>=0 && i<num_tasks );
    return task[i].GetNumBonus(); }

  double GetTaskBonus(const int task_num, int bonus_num) const {
    assert( task_num>=0 && task_num<num_tasks );
    return task[task_num].GetBonus(bonus_num); }

  char GetTaskBonusType(const int task_num, int bonus_num) const {
    assert( task_num>=0 && task_num<num_tasks );
    return task[task_num].GetBonusType(bonus_num); }


  // Called by pheontype on particular events
  double TestInput(cIOBuf & inputs, cIOBuf & outputs,
		   cTaskCount & task_count, const double in_bonus) const {
    return Test(task_group_i, inputs, outputs, task_count, in_bonus); }

  double TestOutput(cIOBuf & inputs, cIOBuf & outputs,
		    cTaskCount & task_count, const double in_bonus) const {
    return Test(task_group_o, inputs, outputs, task_count, in_bonus); }

  // ReCalc Merit Bonus ...
  double ReCalcMeritBonus(tArray<int> & task_count);


  // Internals
private:
  double Test(const cTaskGroup * group_list_head,
	      cIOBuf & inputs, cIOBuf & outputs,
	      cTaskCount & task_count, const double in_bonus) const ;
  double TestGroup(const cTaskGroup & group, cIOBuf & inputs, cIOBuf & outputs,
		   cTaskCount & task_count, const double in_bonus) const ;

  int InputCount(int in_group_id) const {
    assert( cur_inputs != NULL );
    assert( cur_group != NULL );
    assert( in_group_id >= 0 );
    assert( in_group_id < cur_group->GetNumTasks() );
    return cur_inputs->
      GetNum((*cur_group)[in_group_id]->GetID()); }

  int OutputCount(int in_group_id) const {
    assert( cur_outputs != NULL );
    assert( cur_group != NULL );
    assert( in_group_id >= 0 );
    assert( in_group_id < cur_group->GetNumTasks() );
    return cur_outputs->
      GetNum((*cur_group)[in_group_id]->GetID()); }

  int TotalInputCount () const { return cur_inputs->GetTotal(); }
  int TotalOutputCount() const { return cur_outputs->GetTotal(); }

  UINT Input (int index) const { return (*cur_inputs)[index]; }
  UINT Output(int index) const { return (*cur_outputs)[index]; }


  void AddTask(cTaskEntry & task, const cString & group_name);

  void AddTaskToGroup(cTaskEntry & task, const cString & group_name,
		      const char type, const int size,
		      const tTaskMethod test_fun, const int num_return_values,
		      const int index );

  void DeleteTaskGroupList(cTaskGroup * head);  // for Cleanup


  // ****************************  Actual Tasks  ******************************

  // Individual Tasks
  void Test_Get      (int * done_tasks) const;
  void Test_Put      (int * done_tasks) const;
  void Test_GGP      (int * done_tasks) const;
  void Test_Echo     (int * done_tasks) const;
  void Test_Not      (int * done_tasks) const;
  void Test_Nand     (int * done_tasks) const;
  void Test_OrN      (int * done_tasks) const;
  void Test_And      (int * done_tasks) const;
  void Test_Or       (int * done_tasks) const;
  void Test_AndN     (int * done_tasks) const;
  void Test_Nor      (int * done_tasks) const;
  void Test_Xor      (int * done_tasks) const;
  void Test_Equals   (int * done_tasks) const;

  // Task Groups

  void Test_Logic    (int * done_tasks) const;

};

#endif
