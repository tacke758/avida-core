//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "../tools/file.hh"

#include "tasks.hh"
#include "stats.hh"

static const int MAX_NUM_BONUSES_PER_TASK = 64;  // used for loading
static const int MAX_NUM_RESOURCES = 1024;  // used for loading


cTaskGroup::cTaskGroup(const cString & _name, const int _num_tasks,
		       const tTaskMethod _test, const int _num_return_values,
		       cTaskGroup * _next) :
  name(_name), num_tasks(_num_tasks),
  task(new cTaskEntry*[num_tasks]), test(_test),
  num_return_values(_num_return_values), next(_next) {
      for( int i=0; i<num_tasks; ++i ){
	task[i] = NULL;
      }
}

void cTaskGroup::Perturb(double max_factor)
{
  // Loop through all of the tasks, perturbing the bonuses for each.
  for (int i = 0; i < num_tasks; i++) {
    if (task[i] != NULL) task[i]->Perturb(max_factor);
  }
}

void cTaskGroup::Change(double prob_change, double min_bonus, double max_bonus)
{
  // Loop through all of the tasks, perturbing the bonuses for each.
  for (int i = 0; i < num_tasks; i++) {
    if (task[i] != NULL) task[i]->Change(prob_change, min_bonus, max_bonus);
  }
}

void cTaskEntry::Set(const cString & _name, const int _id,
	const int _num_bonus, const char * _bonus_type, const double * _bonus){
  name = _name;
  id = _id;
  num_bonus = _num_bonus;
  assert( bonus == NULL && bonus_type == NULL );
  assert( num_bonus > 0 );
  bonus_type = new char[num_bonus];
  bonus      = new double[num_bonus];
  for( int i=0; i<num_bonus; ++i ){
    bonus_type[i] = _bonus_type[i];
    bonus[i]      = _bonus[i];
  }
}

void cTaskEntry::Perturb(double max_factor)
{
  // Loop through each bonus, perturbing it.
  for (int i = 0; i < num_bonus; i++) {
    double mod_factor = pow(max_factor, g_random.GetDouble(-1.0, 1.0));
    bonus[i] *= mod_factor;
    // @CAO arbitrary limits of 0.5 and 2.0
    //if (bonus[i] < 0.5) bonus[i] = 0.5;
    //else if (bonus[i] > 2.0) bonus[i] = 2.0;
  }
}

void cTaskEntry::Change(double prob_change, double min_bonus, double max_bonus)
{
  // Loop through each bonus, perturbing it.
  for (int i = 0; i < num_bonus; i++) {
    if (g_random.P(prob_change)) {
      double new_bonus = g_random.GetDouble(min_bonus, max_bonus);
      bonus[i] = new_bonus;
    }
  }
}

cTaskLib::cTaskLib() :
 num_tasks(0),
 task(NULL),
 task_group_i(NULL),
 task_group_o(NULL),
 cur_inputs(NULL), cur_outputs(NULL), cur_group(NULL),
 resource(NULL) {;}


cTaskLib::~cTaskLib(){
  if( task != NULL )  delete [] task;
  DeleteTaskGroupList( task_group_i );
  DeleteTaskGroupList( task_group_o );
  if( resource != NULL ) delete resource;
}


void cTaskLib::DeleteTaskGroupList(cTaskGroup * head){
  cTaskGroup * cur_task_group=head;
  cTaskGroup * next_task_group=NULL;
  while( cur_task_group != NULL ){
    next_task_group = const_cast(cTaskGroup*,cur_task_group->GetNext());
    delete cur_task_group;
    cur_task_group = next_task_group;
  }
}


const cString cTaskLib::GetTaskName(const int i) const {
  assert( i>=0 && i<num_tasks );
  cString name = task[i].GetGroup()->GetName();
  if( task[i].GetName() != "-" ){
    name += ":";
    name += task[i].GetName();
  }
  return name;
}

const cString & cTaskLib::GetTaskKeyword(const int i) const {
  assert( i>=0 && i<num_tasks );
  return task[i].GetKeyword();
}




void cTaskLib::Load(cString filename){
  cString line;
  cString name;   // name of task
  cString group_name;   // name of task's group
  int num_bonus;  // number of bonuses
  double bonus[MAX_NUM_BONUSES_PER_TASK];
  char bonus_type[MAX_NUM_BONUSES_PER_TASK];  // *,+, or ^

  cString tmpstr;  // temp string

  // clear the old tasks list and variables
  if( task != NULL)  delete [] task;
  num_tasks = 0;

  // Get the file
  cInitFile infile(filename);
  if (infile.Good() == false) {
    cerr << "Error: Failed to load task library '" << filename << "'." << endl;
    exit(1);
  }

  infile.Load();
  infile.Close();
  infile.Compress();

  task = new cTaskEntry[infile.GetNumLines()];  // Allocate the task array

  // foreach line in tasks file, read in task name and bonuses, then add task
  while( infile.GetNumLines() ){
    line = infile.RemoveLine();
    group_name = line.PopWord();
    name = line.PopWord();
    num_bonus = 0;
    // Read in the Task
    while( line.GetSize() ){
      bonus_type[num_bonus] = line.PopWord()[0];
      // Verify bonus type is valid
      if( bonus_type[num_bonus] != '*' &&
	  bonus_type[num_bonus] != '+' &&
	  bonus_type[num_bonus] != '^' &&
	  bonus_type[num_bonus] != 'd' ){
	cerr<<"Error reading \""<<filename
	    <<"\": Unknown task bonus type: \""
	    <<bonus_type[num_bonus]<<"\" for task \""
	    <<name<<"\""<<endl;
	abort();
      }
      // Don't read the bonus if there isn't on there!
      if( line.GetSize() ){
	bonus[num_bonus] = line.PopWord().AsDouble();
      }else{
	cerr<<"Error reading \""<<filename
	    <<"\": Bonus type defined for task \""
	    <<name<<"\" but no bonus specified!"<<endl;
	abort();
      }

      // If bonus type is '*', subtract 1 from bonus since we add it back
      // in the actual calculation --@TCC ?? should we do this way?
      if( bonus_type[num_bonus] == '*' )  bonus[num_bonus] -= 1;

      ++num_bonus;
    }

    // If there are no bonuses listed.. Error!
    if( num_bonus <= 0 ){
      cerr<<"Error reading \""<<filename
	  <<"\":  At least one bonus value must be listed for each task"
	  <<endl;	
      abort();
    }

    // Set the task entry object
    task[num_tasks].Set(name, num_tasks, num_bonus, bonus_type, bonus);

    // Add the task entry to the task group too!
    AddTask(task[num_tasks], group_name);

    ++num_tasks;  // On to the next task
  }

  // Debugging output
#ifdef DEBUG
  for( int i=0; i<num_tasks; ++i ){
    cerr<<task[i].GetID()<<"\t";
    cerr<<task[i].GetGroup()->GetName()<<"\t";
    cerr<<task[i].GetName()<<"\t";
    for( int j=0; j<task[i].GetNumBonus(); ++j ){
      cerr<<task[i].GetBonusType(j)
	  <<task[i].GetBonus(j)<<"\t";
    }
    cerr<<endl;
  }
#endif // DEBUG
}


void cTaskLib::LoadResources(const cString & filename){
  int i;
  cString line;
  cString name;   // name of task
  cString group_name;   // name of task's group
  int num_resources;  // number of resources

  cString tmpstr;  // temp string

  // clear the old resources and variables
  if( resource != NULL)  delete resource;
  num_resources = 0;

  // Get the file
  cInitFile infile(filename);
  infile.Load();
  infile.Close();
  infile.Compress();

  // Figure out how many resources we have
  line = infile.RemoveLine();
  num_resources = line.CountNumWords() -1;

  // Allocate resource array
  resource = new tArray<cResource>(num_resources);

  // Init Conc
  line.PopWord(); // throw out label
  for( i=0; i<num_resources; ++i ){
    (*resource)[i].id = i;
    (*resource)[i].init_conc = line.PopWord().AsDouble();
    (*resource)[i].conc = (*resource)[i].init_conc;
  }

  // In Flow
  line = infile.RemoveLine();
  line.PopWord(); // throw out label
  for( i=0; i<num_resources; ++i ){
    (*resource)[i].inflow = line.PopWord().AsDouble();
  }

  // Out Flow
  line = infile.RemoveLine();
  line.PopWord(); // throw out label
  for( i=0; i<num_resources; ++i ){
    (*resource)[i].outflow = line.PopWord().AsDouble();
  }

  // Add the entries for the tasks.
  while( infile.GetNumLines() ){
    // Task name and group
    line = infile.RemoveLine();
    group_name = line.PopWord();
    name = line.PopWord();
    if( name != "-" ){
      group_name += ":";
      group_name += name;
    }
    name = group_name;

    // Lookup the figgin task entry...  There should be a better way @TCC
    i=0;
    while( i<GetNumTasks()  &&
	   GetTaskName(i) != name ){
      ++i;
    }
    assert( i < GetNumTasks() );
    //cerr<<name<<" ?= "<<GetTaskName(i)<<endl;

    cTaskEntry * task_entry = &(task[i]);
    // Initialize the resource arrays
    task_entry->resource_dependency = new tArray<bool>(num_resources);
    task_entry->resource_effect = new tArray<double>(num_resources);


    // Add to the task entry
    for( i=0; i<num_resources; ++i ){
      tmpstr = line.PopWord();

      (*task_entry->resource_dependency)[i] = false;
      (*task_entry->resource_effect)[i] = 0;

      if( tmpstr[0] == 'd' ){
	(*task_entry->resource_dependency)[i] = true;
	tmpstr.ClipFront(1);
      }
      if( tmpstr[0] == '-' || tmpstr[0] == '+' ){
	(*task_entry->resource_effect)[i] = tmpstr.AsDouble();
      }
      //cerr<<(*task_entry->resource_dependency)[i]<<" ";
      //cerr<<(*task_entry->resource_effect)[i]<<" = "<<tmpstr()<<endl;
    }

  }
}


double cTaskLib::ReCalcMeritBonus(tArray<int> & task_count){
  assert(false);
  return 1;
}



void cTaskLib::UpdateResources(){
  int i;
  double tscale;

  // Time rescaling
  if( cStats::GetAveMerit() == 0 ){
    tscale = 1;
  }else{
    tscale = 1/cStats::GetAveMerit();
  }

  for( i=0; i< resource->GetSize(); ++i ){
    double in = tscale *
      (*resource)[i].init_conc * (*resource)[i].inflow;
    double out = tscale *
      (*resource)[i].conc * (*resource)[i].outflow;
    (*resource)[i].conc -= out;
    (*resource)[i].conc += in;
    //(*resource)[i].conc = (*resource)[i].init_conc;
  }
}


void cTaskLib::PrintResourceData(const cString & filename){
    ofstream & fp = cStats::GetDataFileOFStream(filename);
    assert(fp.good());
    if( cConfig::GetResources() == 1 ){
      fp<< cStats::GetUpdate();         // 1
      for( int i=0; i< resource->GetSize(); ++i ){
	fp<<" "<< (*resource)[i].conc;
      }
      fp<<endl;
    }else{
      fp<<"none"<<endl;
    }
}


void cTaskLib::Perturb(double max_factor)
{
  // Go through all of the output groups. (for now...)
  cTaskGroup * group = task_group_o;
  while (group != NULL) {
    group->Perturb(max_factor);
    group = group->next;
  }
}

void cTaskLib::Change(double prob_change, double min_bonus, double max_bonus)
{
  // Go through all of the output groups. (for now...)
  cTaskGroup * group = task_group_o;
  while (group != NULL) {
    group->Change(prob_change, min_bonus, max_bonus);
    group = group->next;
  }
}

void cTaskLib::PrintBonus(ofstream & fp) const
{
  fp << cStats::GetUpdate();

  const cTaskGroup * group = task_group_o;
  while (group != NULL) {
    for (int i = 0; i < group->GetNumTasks(); i++) {
      cTaskEntry * cur_task = group->task[i];
      if (cur_task != NULL) {
	for (int j = 0; j < cur_task->GetNumBonus(); j++) {
	  fp << " " << cur_task->GetBonus(j);
	}
      }
    }

    group = group->next;
  }

  fp << endl;
}

double cTaskLib::Test(const cTaskGroup * group_list_head,
		      cIOBuf & inputs, cIOBuf & outputs,
		      cTaskCount & task_count, const double in_bonus) const {
  double bonus = in_bonus;

  // Iterate through task groups
  const cTaskGroup * group = group_list_head;
  while( group != NULL ){

    // Test Group
    bonus = TestGroup((*group), inputs, outputs, task_count, bonus);

    // Next Group
    group = group->GetNext();

  } // End while( group != NULL )


  // If we have task merit turned off just return in_bonus
  if( cConfig::GetTaskMeritMethod() == TASK_MERIT_NONE ){
    bonus = in_bonus;
  }

  return bonus;
}


double cTaskLib::TestGroup(const cTaskGroup & group, cIOBuf & inputs,
			   cIOBuf & outputs, cTaskCount & task_count,
			   const double in_bonus) const {
  assert( group.GetNumTasks() > 0 );
  assert( group.GetNumReturnValues() > 0 );
  double bonus = in_bonus;
  const cTaskEntry * ctask = NULL;
  int ctask_id = -1;
  int * done_tasks = new int[group.GetNumReturnValues()+1];
  done_tasks[0] = -1;

  // so we don't have to pass to test functions
  cTaskLib * const fake_this = const_cast(cTaskLib * const, this);
  fake_this->cur_inputs  = &inputs;
  fake_this->cur_outputs = &outputs;
  fake_this->cur_group   = &group;

  // Test (for all tasks in group)
  // returns array of task id (id in group) for tasks done terminated with -1
  const tTaskMethod testfunc = group.GetTest();
  (this->*testfunc)(done_tasks);

  // for each done_tasks (up to -1)
  for( int i=0; i<group.GetNumReturnValues() && done_tasks[i] != -1; ++i ){
    ctask = group[done_tasks[i]];

    // If the task is active
    if( ctask != NULL ){
      ctask_id = ctask->GetID();

      // If the max number of tasks done has not been reached...
      if( cConfig::GetMaxNumTasksRewarded() < 0  ||
	  task_count.GetTotalNumTasksDone() < cConfig::GetMaxNumTasksRewarded() ){

	// Add it to the task_exe_count in stats
	cStats::IncTaskExeCount(ctask_id);

	// Assign the bonus
	double task_bonus = ctask->GetBonus(task_count[ctask_id]);

	// Resouce Dependence and depletion (or creation)
	// @TCC -- Restructure so index is saved in task entry
	if( cConfig::GetResources() == 1 ){
	  if( ctask->resource_dependency != NULL ){
	    for( int i=0; i<ctask->resource_dependency->GetSize(); ++i ){
	      if( (*ctask->resource_dependency)[i] == true ){
		task_bonus *= (*resource)[i].conc / (*resource)[i].init_conc;
	      }
	      (*resource)[i].conc += (*ctask->resource_effect)[i];
	      if( (*resource)[i].conc < 0 ){
		(*resource)[i].conc = 0;
	      }
	    }
	    // Deplete the resouce -- @TCC to add
	    //cerr<<"task_bonus "<<ctask->GetID()<<" "<<
	    // ctask->GetBonus(task_count[ctask_id])<<" -> "<<task_bonus<<endl;
	  }
	}

	switch ( ctask->GetBonusType(task_count[ctask_id]) ) {
	  case '*': {
	    bonus *= 1+task_bonus;
	    break;
	  }
	  case '+': {
	    bonus += task_bonus;
	    break;
	  }
	  case '^': {
	    bonus = pow(bonus, task_bonus);
	    break;
	  }
	  case 'd': { // diminishing!
	    assert( task_count[ctask_id] > 0 );
	    assert( task_count[ctask_id] - ctask->GetNumBonus() + 2 > 0 );
	    // Only works for previous task being multiplicative!!! @TCC
	    assert( ctask->GetBonusType(task_count[ctask_id]-1) == '*' );
	    double base_bonus = ctask->GetBonus(task_count[ctask_id]-1);
	    double bonus_mod = task_bonus;
	    int num = task_count[ctask_id] - ctask->GetNumBonus() + 2;
	    task_bonus = base_bonus * pow(bonus_mod, num);
	    bonus *= 1 + task_bonus;
	    //cerr<<"num "<<num<<endl;
	    //cerr<<"bonus base "<<base_bonus<<endl;
	    //cerr<<"bonus mod "<<bonus_mod<<endl;
	    //cerr<<"task bonus "<<task_bonus<<endl;
	    //cerr<<"bonus "<<bonus<<endl;
	    break;
	  }
	  default: {
	    cerr<<"Unknown task bonus type: \""
		<<ctask->GetBonusType(task_count[ctask_id])<<"\""<<endl;
	    abort();
	  }
	}
	
	// Increment the task count
	task_count.IncCount(ctask_id);
	// Adjust the Input and Output buffers
	inputs.ZeroNumAdds(ctask_id);
	outputs.ZeroNumAdds(ctask_id);
      } // if < max_num_tasks_rewarded
    } // if task active (!=NULL)
  } // for each done_tasks

  delete [] done_tasks;

  // Just to make sure we don't use them outside this function
  // This should probably be in debugging only - @TCC
  fake_this->cur_inputs  = NULL;
  fake_this->cur_outputs = NULL;
  fake_this->cur_group   = NULL;

  return bonus;
}


void cTaskLib::AddTaskToGroup(cTaskEntry & task, const cString & group_name,
			      const char type, const int size,
			      const tTaskMethod test_fun,
			      const int num_return_values, const int index ){
  cTaskGroup * group = NULL;
  cTaskGroup * * task_group_head = NULL;

  // Get the head to the appropriate task_group list based on type
  switch( type ){
    case 'i':
      task_group_head = &task_group_i;
      break;
    case 'o':
      task_group_head = &task_group_o;
      break;
    default:
      cerr<<"ERROR: Undefined Task Group Type \""<<type<<"\""<<endl;
      abort();
  }

  group = *task_group_head;

  // Find the task group if it exists
  cTaskGroup * next_task_group=NULL;
  while( group != NULL ){
    next_task_group = const_cast(cTaskGroup*, group->GetNext());
    if( group->GetName() == group_name ){
      break;
    }else{
      group = next_task_group;
    }
  }

  // If task group does not exist, make it
  if( group == NULL ){
    group = new cTaskGroup(group_name, size, test_fun, num_return_values,
			   *task_group_head);
    // Add to task_group list (single linked list add to front)
    *task_group_head = group;
  }

  // Add task to group
  assert( index < group->GetNumTasks() );
  assert( (*group)[index] == NULL );
  task.SetGroup(group);
  group->SetTask(index, task);
}



// **************** Actual Tasks ****************

void cTaskLib::AddTask(cTaskEntry & task, const cString & group_name){
  const cString & name = task.GetName();
  tTaskMethod test_fun;
  int size;  // Total number of tasks in the group
  int num_return_values;  // Number of tasks that can be completed in one test
  char type;  // 'i' = input, 'o' = ouptut ... what condition calles the test
  int index;  // index (in group) of the task

  // Groups with just one task
  size = 1;  num_return_values = 1;  index = 0;
  if(      group_name == "get"   ){ test_fun=&cTaskLib::Test_Get;   type='i'; }
  else if( group_name == "put"   ){ test_fun=&cTaskLib::Test_Put;   type='o'; }
  else if( group_name == "ggp"   ){ test_fun=&cTaskLib::Test_GGP;   type='o'; }
  else if( group_name == "echo"  ){ test_fun=&cTaskLib::Test_Echo;  type='o'; }
  else if( group_name == "not"   ){ test_fun=&cTaskLib::Test_Not;   type='o'; }
  else if( group_name == "nand"  ){ test_fun=&cTaskLib::Test_Nand;  type='o'; }
  else if( group_name == "or_n"  ){ test_fun=&cTaskLib::Test_OrN;   type='o'; }
  else if( group_name == "and"   ){ test_fun=&cTaskLib::Test_And;   type='o'; }
  else if( group_name == "or"    ){ test_fun=&cTaskLib::Test_Or;    type='o'; }
  else if( group_name == "and_n" ){ test_fun=&cTaskLib::Test_AndN;  type='o'; }
  else if( group_name == "nor"   ){ test_fun=&cTaskLib::Test_Nor;   type='o'; }
  else if( group_name == "xor"   ){ test_fun=&cTaskLib::Test_Xor;   type='o'; }
  else if( group_name == "equ"   ){ test_fun=&cTaskLib::Test_Equals; type='o';}

  // Task Groups ///////////////////////

  // Logic
  else if( group_name == "logic"         ){
    size = 80;
    test_fun = &cTaskLib::Test_Logic;
    type = 'o';
    num_return_values = 1;
    if(      name == "0a"  ){ index = 0; }
    else if( name == "0b"  ){ index = 1; }

    else if( name == "1a"  ){ index = 2;  task.SetKeyword("ECHO"); }
    else if( name == "1b"  ){ index = 3;  task.SetKeyword("NOT"); }

    else if( name == "2a"  ){ index = 4;  task.SetKeyword("AND"); }
    else if( name == "2b"  ){ index = 5;  task.SetKeyword("OR"); }
    else if( name == "2c"  ){ index = 6;  task.SetKeyword("OR-N"); }
    else if( name == "2d"  ){ index = 7;  task.SetKeyword("AND-N"); }
    else if( name == "2e"  ){ index = 8;  task.SetKeyword("NOR"); }
    else if( name == "2f"  ){ index = 9;  task.SetKeyword("NAND"); }
    else if( name == "2g"  ){ index = 10; task.SetKeyword("XOR"); }
    else if( name == "2h"  ){ index = 11; task.SetKeyword("EQU"); }

    else if( name == "3a"  ){ index = 12; }
    else if( name == "3b"  ){ index = 13; }
    else if( name == "3c"  ){ index = 14; }
    else if( name == "3d"  ){ index = 15; }
    else if( name == "3e"  ){ index = 16; }
    else if( name == "3f"  ){ index = 17; }
    else if( name == "3g"  ){ index = 18; }
    else if( name == "3h"  ){ index = 19; }
    else if( name == "3i"  ){ index = 20; }
    else if( name == "3j"  ){ index = 21; }
    else if( name == "3k"  ){ index = 22; }
    else if( name == "3l"  ){ index = 23; }
    else if( name == "3m"  ){ index = 24; }
    else if( name == "3n"  ){ index = 25; }
    else if( name == "3o"  ){ index = 26; }
    else if( name == "3p"  ){ index = 27; }
    else if( name == "3q"  ){ index = 28; }
    else if( name == "3r"  ){ index = 29; }
    else if( name == "3s"  ){ index = 30; }
    else if( name == "3t"  ){ index = 31; }
    else if( name == "3u"  ){ index = 32; }
    else if( name == "3v"  ){ index = 33; }
    else if( name == "3w"  ){ index = 34; }
    else if( name == "3x"  ){ index = 35; }
    else if( name == "3y"  ){ index = 36; }
    else if( name == "3z"  ){ index = 37; }
    else if( name == "3aa" ){ index = 38; }
    else if( name == "3ab" ){ index = 39; }
    else if( name == "3ac" ){ index = 40; }
    else if( name == "3ad" ){ index = 41; }
    else if( name == "3ae" ){ index = 42; }
    else if( name == "3af" ){ index = 43; }
    else if( name == "3ag" ){ index = 44; }
    else if( name == "3ah" ){ index = 45; }
    else if( name == "3ai" ){ index = 46; }
    else if( name == "3aj" ){ index = 47; }
    else if( name == "3ak" ){ index = 48; }
    else if( name == "3al" ){ index = 49; }
    else if( name == "3am" ){ index = 50; }
    else if( name == "3an" ){ index = 51; }
    else if( name == "3ao" ){ index = 52; }
    else if( name == "3ap" ){ index = 53; }
    else if( name == "3aq" ){ index = 54; }
    else if( name == "3ar" ){ index = 55; }
    else if( name == "3as" ){ index = 56; }
    else if( name == "3at" ){ index = 57; }
    else if( name == "3au" ){ index = 58; }
    else if( name == "3av" ){ index = 59; }
    else if( name == "3aw" ){ index = 60; }
    else if( name == "3ax" ){ index = 61; }
    else if( name == "3ay" ){ index = 62; }
    else if( name == "3az" ){ index = 63; }
    else if( name == "3ba" ){ index = 64; }
    else if( name == "3bb" ){ index = 65; }
    else if( name == "3bc" ){ index = 66; }
    else if( name == "3bd" ){ index = 67; }
    else if( name == "3be" ){ index = 68; }
    else if( name == "3bf" ){ index = 69; }
    else if( name == "3bg" ){ index = 70; }
    else if( name == "3bh" ){ index = 71; }
    else if( name == "3bi" ){ index = 72; }
    else if( name == "3bj" ){ index = 73; }
    else if( name == "3bk" ){ index = 74; }
    else if( name == "3bl" ){ index = 75; }
    else if( name == "3bm" ){ index = 76; }
    else if( name == "3bn" ){ index = 77; }
    else if( name == "3bo" ){ index = 78; }
    else if( name == "3bp" ){ index = 79; }
    else { cerr<<"Error:  Task \""<<group_name<<"::"<<name
	       <<"\" not found"<<endl; abort(); }
  }

  // Else, can't find the group... error
  else { cerr<<"Task/Group \""<<group_name<<"\" not found "<<endl; abort(); }

  AddTaskToGroup(task, group_name, type, size, test_fun, num_return_values,
		 index);
}


// Individual Tasks //////////////////////////////////////////////////////////


void cTaskLib::Test_Get(int * done_tasks) const {
  // Called only on an input, so always true
  done_tasks[0] = 0;
}


void cTaskLib::Test_Put(int * done_tasks) const {
  // Called only on an output, so always true
  done_tasks[0] = 0;
}


void cTaskLib::Test_GGP(int * done_tasks) const {
  // Get - Get - Put sequence
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Echo(int * done_tasks) const {
  if( InputCount(0)  > 0   &&
      OutputCount(0) > 0   &&
      Input(0) == Output(0) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Not(int * done_tasks) const {
  if( InputCount(0)  > 0   &&
      OutputCount(0) > 0   &&
      Input(0) == ~Output(0) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Nand(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      Output(0) == (~(Input(0) & Input(1))) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_OrN(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      ( Output(0) == (~Input(0) | Input(1)) ||
	Output(0) == (Input(0) | ~Input(1)) ) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_And(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      Output(0) == (Input(0) & Input(1)) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Or(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      Output(0) == (Input(0) | Input(1)) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_AndN(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      ( Output(0) == (~Input(0) & Input(1)) ||
	Output(0) == (Input(0) & ~Input(1)) ) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Nor(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      Output(0) == ~(Input(0) | Input(1)) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Xor(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      Output(0) == (Input(0) ^ Input(1)) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


void cTaskLib::Test_Equals(int * done_tasks) const {
  if( InputCount(0)  > 1   &&
      OutputCount(0) > 0   &&
      Output(0) == ~(Input(0) ^ Input(1)) ){
    done_tasks[0] = 0;
  }else{
    done_tasks[0] = -1;
  }	
}


// Task Groups ///////////////////////////////////////////////////////////////


void cTaskLib::Test_Logic(int * done_tasks) const {
  const UINT LOGIC_TASK_A  =  0;
  const UINT LOGIC_TASK_B  =  1;
  const UINT LOGIC_TASK_C  =  2;
  const UINT LOGIC_TASK_D  =  3;
  const UINT LOGIC_TASK_E  =  4;
  const UINT LOGIC_TASK_F  =  5;
  const UINT LOGIC_TASK_G  =  6;
  const UINT LOGIC_TASK_H  =  7;
  const UINT LOGIC_TASK_I  =  8;
  const UINT LOGIC_TASK_J  =  9;
  const UINT LOGIC_TASK_K  = 10;
  const UINT LOGIC_TASK_L  = 11;
  const UINT LOGIC_TASK_M  = 12;
  const UINT LOGIC_TASK_N  = 13;
  const UINT LOGIC_TASK_O  = 14;
  const UINT LOGIC_TASK_P  = 15;
  const UINT LOGIC_TASK_Q  = 16;
  const UINT LOGIC_TASK_R  = 17;
  const UINT LOGIC_TASK_S  = 18;
  const UINT LOGIC_TASK_T  = 19;
  const UINT LOGIC_TASK_U  = 20;
  const UINT LOGIC_TASK_V  = 21;
  const UINT LOGIC_TASK_W  = 22;
  const UINT LOGIC_TASK_X  = 23;
  const UINT LOGIC_TASK_Y  = 24;
  const UINT LOGIC_TASK_Z  = 25;
  const UINT LOGIC_TASK_AA = 26;
  const UINT LOGIC_TASK_AB = 27;
  const UINT LOGIC_TASK_AC = 28;
  const UINT LOGIC_TASK_AD = 29;
  const UINT LOGIC_TASK_AE = 30;
  const UINT LOGIC_TASK_AF = 31;
  const UINT LOGIC_TASK_AG = 32;
  const UINT LOGIC_TASK_AH = 33;
  const UINT LOGIC_TASK_AI = 34;
  const UINT LOGIC_TASK_AJ = 35;
  const UINT LOGIC_TASK_AK = 36;
  const UINT LOGIC_TASK_AL = 37;
  const UINT LOGIC_TASK_AM = 38;
  const UINT LOGIC_TASK_AN = 39;
  const UINT LOGIC_TASK_AO = 40;
  const UINT LOGIC_TASK_AP = 41;
  const UINT LOGIC_TASK_AQ = 42;
  const UINT LOGIC_TASK_AR = 43;
  const UINT LOGIC_TASK_AS = 44;
  const UINT LOGIC_TASK_AT = 45;
  const UINT LOGIC_TASK_AU = 46;
  const UINT LOGIC_TASK_AV = 47;
  const UINT LOGIC_TASK_AW = 48;
  const UINT LOGIC_TASK_AX = 49;
  const UINT LOGIC_TASK_AY = 50;
  const UINT LOGIC_TASK_AZ = 51;
  const UINT LOGIC_TASK_BA = 52;
  const UINT LOGIC_TASK_BB = 53;
  const UINT LOGIC_TASK_BC = 54;
  const UINT LOGIC_TASK_BD = 55;
  const UINT LOGIC_TASK_BE = 56;
  const UINT LOGIC_TASK_BF = 57;
  const UINT LOGIC_TASK_BG = 58;
  const UINT LOGIC_TASK_BH = 59;
  const UINT LOGIC_TASK_BI = 60;
  const UINT LOGIC_TASK_BJ = 61;
  const UINT LOGIC_TASK_BK = 62;
  const UINT LOGIC_TASK_BL = 63;
  const UINT LOGIC_TASK_BM = 64;
  const UINT LOGIC_TASK_BN = 65;
  const UINT LOGIC_TASK_BO = 66;
  const UINT LOGIC_TASK_BP = 67;

  UINT get1 = 0;
  UINT get2 = 0;
  UINT get3 = 0;
  UINT put_value = Output(0);

  if (TotalInputCount() >= 3) get3 = Input(2);
  if (TotalInputCount() >= 2) get2 = Input(1);
  if (TotalInputCount() >= 1) get1 = Input(0);

  //cerr<<" Total Input Count = "<<TotalInputCount()<<endl;
  //cerr<<put_value<<" ==? "<<get1<<endl;

  // Setup logic_out to test the output for each logical combination...
  int i;
  int logic_out[8];
  for (i = 0; i < 8; i++) {
    logic_out[i] = -1;
  }


  int func_OK = TRUE;

  // Find the output for each input combo, and make sure they are
  // consistant.
  for (int test_pos = 0; test_pos < 32; test_pos++) {
    int logic_pos = ((get3 & 1) << 2) + ((get2 & 1) << 1) + ((get1 & 1));
    if (logic_out[logic_pos] != -1) {
	if (logic_out[logic_pos] != (int) (put_value & 1)) {
	  func_OK = FALSE;
	  break;
	}
      } else {
	logic_out[logic_pos] = put_value & 1;
      }

    put_value >>= 1;
    get1 >>= 1;
    get2 >>= 1;
    get3 >>= 1;
  }


  // Check number of inputs
  switch(TotalInputCount()) {
  case 0:
    logic_out[1] = logic_out[0];
  case 1:
    logic_out[2] = logic_out[0];
    logic_out[3] = logic_out[1];
  case 2:
    logic_out[4] = logic_out[0];
    logic_out[5] = logic_out[1];
    logic_out[6] = logic_out[2];
    logic_out[7] = logic_out[3];
  }

  // Kill any creature with -1's (its missing some combinations...)
  for (int j = 0; j < 8; j++) {
    if (logic_out[j] == -1) func_OK = FALSE;
  }

  if (func_OK == FALSE){
    done_tasks[0] = -1;
    return;
  }


  int task_id = 0;
  task_id += logic_out[0] << 7;
  task_id += logic_out[1] << 6;
  task_id += logic_out[2] << 5;
  task_id += logic_out[4] << 4;
  task_id += logic_out[6] << 3;
  task_id += logic_out[5] << 2;
  task_id += logic_out[3] << 1;
  task_id += logic_out[7];

  int inputs_used = -1;
  int task_code = -1;

  switch(task_id) {
  case 0:
    inputs_used = 0;
    task_code = LOGIC_TASK_A;
    break;
  case 255:
    inputs_used = 0;
    task_code = LOGIC_TASK_B;
    break;
  case 29:
  case 43:
  case 71:
    inputs_used = 1;
    task_code = LOGIC_TASK_A;
    break;
  case 184:
  case 212:
  case 226:
    inputs_used = 1;
    task_code = LOGIC_TASK_B;
    break;
  case 3:
  case 5:
  case 9:
    inputs_used = 2;
    task_code = LOGIC_TASK_A;
    break;
  case 63:
  case 95:
  case 111:
    inputs_used = 2;
    task_code = LOGIC_TASK_B;
    break;
  case 187:
  case 189:
  case 215:
  case 221:
  case 231:
  case 235:
    inputs_used = 2;
    task_code = LOGIC_TASK_C;
    break;
  case 20:
  case 24:
  case 34:
  case 40:
  case 66:
  case 68:
    inputs_used = 2;
    task_code = LOGIC_TASK_D;
    break;
  case 144:
  case 160:
  case 192:
    inputs_used = 2;
    task_code = LOGIC_TASK_E;
    break;
  case 246:
  case 250:
  case 252:
    inputs_used = 2;
    task_code = LOGIC_TASK_F;
    break;
  case 54:
  case 90:
  case 108:
    inputs_used = 2;
    task_code = LOGIC_TASK_G;
    break;
  case 147:
  case 165:
  case 201:
    inputs_used = 2;
    task_code = LOGIC_TASK_H;
    break;
  case 1:
    inputs_used = 3;
    task_code = LOGIC_TASK_A;
    break;
  case 2:
  case 4:
  case 8:
    inputs_used = 3;
    task_code = LOGIC_TASK_B;
    break;
  case 16:
  case 32:
  case 64:
    inputs_used = 3;
    task_code = LOGIC_TASK_C;
    break;
  case 128:
    inputs_used = 3;
    task_code = LOGIC_TASK_D;
    break;
  case 6:
  case 10:
  case 12:
    inputs_used = 3;
    task_code = LOGIC_TASK_E;
    break;
  case 7:
  case 11:
  case 13:
    inputs_used = 3;
    task_code = LOGIC_TASK_F;
    break;
  case 14:
    inputs_used = 3;
    task_code = LOGIC_TASK_G;
    break;
  case 15:
    inputs_used = 3;
    task_code = LOGIC_TASK_H;
    break;
  case 17:
  case 33:
  case 65:
    inputs_used = 3;
    task_code = LOGIC_TASK_I;
    break;
  case 18:
  case 36:
  case 72:
    inputs_used = 3;
    task_code = LOGIC_TASK_J;
    break;
  case 19:
  case 37:
  case 73:
    inputs_used = 3;
    task_code = LOGIC_TASK_K;
    break;
  case 21:
  case 25:
  case 35:
  case 41:
  case 67:
  case 69:
    inputs_used = 3;
    task_code = LOGIC_TASK_L;
    break;
  case 22:
  case 26:
  case 38:
  case 44:
  case 74:
  case 76:
    inputs_used = 3;
    task_code = LOGIC_TASK_M;
    break;
  case 23:
  case 27:
  case 39:
  case 45:
  case 75:
  case 77:
    inputs_used = 3;
    task_code = LOGIC_TASK_N;
    break;
  case 28:
  case 42:
  case 70:
    inputs_used = 3;
    task_code = LOGIC_TASK_O;
    break;
  case 30:
  case 46:
  case 78:
    inputs_used = 3;
    task_code = LOGIC_TASK_P;
    break;
  case 31:
  case 47:
  case 79:
    inputs_used = 3;
    task_code = LOGIC_TASK_Q;
    break;
  case 48:
  case 80:
  case 96:
    inputs_used = 3;
    task_code = LOGIC_TASK_R;
    break;
  case 49:
  case 81:
  case 97:
    inputs_used = 3;
    task_code = LOGIC_TASK_S;
    break;
  case 50:
  case 52:
  case 82:
  case 88:
  case 100:
  case 104:
    inputs_used = 3;
    task_code = LOGIC_TASK_T;
    break;
  case 51:
  case 53:
  case 83:
  case 89:
  case 101:
  case 105:
    inputs_used = 3;
    task_code = LOGIC_TASK_U;
    break;
  case 55:
  case 91:
  case 109:
    inputs_used = 3;
    task_code = LOGIC_TASK_V;
    break;
  case 56:
  case 84:
  case 98:
    inputs_used = 3;
    task_code = LOGIC_TASK_W;
    break;
  case 57:
  case 85:
  case 99:
    inputs_used = 3;
    task_code = LOGIC_TASK_X;
    break;
  case 58:
  case 60:
  case 86:
  case 92:
  case 102:
  case 106:
    inputs_used = 3;
    task_code = LOGIC_TASK_Y;
    break;
  case 59:
  case 61:
  case 87:
  case 93:
  case 103:
  case 107:
    inputs_used = 3;
    task_code = LOGIC_TASK_Z;
    break;
  case 62:
  case 94:
  case 110:
    inputs_used = 3;
    task_code = LOGIC_TASK_AA;
    break;
  case 112:
    inputs_used = 3;
    task_code = LOGIC_TASK_AB;
    break;
  case 113:
    inputs_used = 3;
    task_code = LOGIC_TASK_AC;
    break;
  case 114:
  case 116:
  case 120:
    inputs_used = 3;
    task_code = LOGIC_TASK_AD;
    break;
  case 115:
  case 117:
  case 121:
    inputs_used = 3;
    task_code = LOGIC_TASK_AE;
    break;
  case 118:
  case 122:
  case 124:
    inputs_used = 3;
    task_code = LOGIC_TASK_AF;
    break;
  case 119:
  case 123:
  case 125:
    inputs_used = 3;
    task_code = LOGIC_TASK_AG;
    break;
  case 126:
    inputs_used = 3;
    task_code = LOGIC_TASK_AH;
    break;
  case 127:
    inputs_used = 3;
    task_code = LOGIC_TASK_AI;
    break;
  case 129:
    inputs_used = 3;
    task_code = LOGIC_TASK_AJ;
    break;
  case 130:
  case 132:
  case 136:
    inputs_used = 3;
    task_code = LOGIC_TASK_AK;
    break;
  case 131:
  case 133:
  case 137:
    inputs_used = 3;
    task_code = LOGIC_TASK_AL;
    break;
  case 134:
  case 138:
  case 140:
    inputs_used = 3;
    task_code = LOGIC_TASK_AM;
    break;
  case 135:
  case 139:
  case 141:
    inputs_used = 3;
    task_code = LOGIC_TASK_AN;
    break;
  case 142:
    inputs_used = 3;
    task_code = LOGIC_TASK_AO;
    break;
  case 143:
    inputs_used = 3;
    task_code = LOGIC_TASK_AP;
    break;
  case 145:
  case 161:
  case 193:
    inputs_used = 3;
    task_code = LOGIC_TASK_AQ;
    break;
  case 146:
  case 164:
  case 200:
    inputs_used = 3;
    task_code = LOGIC_TASK_AR;
    break;
  case 148:
  case 152:
  case 162:
  case 168:
  case 194:
  case 196:
    inputs_used = 3;
    task_code = LOGIC_TASK_AS;
    break;
  case 149:
  case 153:
  case 163:
  case 169:
  case 195:
  case 197:
    inputs_used = 3;
    task_code = LOGIC_TASK_AT;
    break;
  case 150:
  case 154:
  case 166:
  case 172:
  case 202:
  case 204:
    inputs_used = 3;
    task_code = LOGIC_TASK_AU;
    break;
  case 151:
  case 155:
  case 167:
  case 173:
  case 203:
  case 205:
    inputs_used = 3;
    task_code = LOGIC_TASK_AV;
    break;
  case 156:
  case 170:
  case 198:
    inputs_used = 3;
    task_code = LOGIC_TASK_AW;
    break;
  case 157:
  case 171:
  case 199:
    inputs_used = 3;
    task_code = LOGIC_TASK_AX;
    break;
  case 158:
  case 174:
  case 206:
    inputs_used = 3;
    task_code = LOGIC_TASK_AY;
    break;
  case 159:
  case 175:
  case 207:
    inputs_used = 3;
    task_code = LOGIC_TASK_AZ;
    break;
  case 176:
  case 208:
  case 224:
    inputs_used = 3;
    task_code = LOGIC_TASK_BA;
    break;
  case 177:
  case 209:
  case 225:
    inputs_used = 3;
    task_code = LOGIC_TASK_BB;
    break;
  case 178:
  case 180:
  case 210:
  case 216:
  case 228:
  case 232:
    inputs_used = 3;
    task_code = LOGIC_TASK_BC;
    break;
  case 179:
  case 181:
  case 211:
  case 217:
  case 229:
  case 233:
    inputs_used = 3;
    task_code = LOGIC_TASK_BD;
    break;
  case 182:
  case 218:
  case 236:
    inputs_used = 3;
    task_code = LOGIC_TASK_BE;
    break;
  case 183:
  case 219:
  case 237:
    inputs_used = 3;
    task_code = LOGIC_TASK_BF;
    break;
  case 185:
  case 213:
  case 227:
    inputs_used = 3;
    task_code = LOGIC_TASK_BG;
    break;
  case 186:
  case 188:
  case 214:
  case 220:
  case 230:
  case 234:
    inputs_used = 3;
    task_code = LOGIC_TASK_BH;
    break;
  case 190:
  case 222:
  case 238:
    inputs_used = 3;
    task_code = LOGIC_TASK_BI;
    break;
  case 191:
  case 223:
  case 239:
    inputs_used = 3;
    task_code = LOGIC_TASK_BJ;
    break;
  case 240:
    inputs_used = 3;
    task_code = LOGIC_TASK_BK;
    break;
  case 241:
    inputs_used = 3;
    task_code = LOGIC_TASK_BL;
    break;
  case 242:
  case 244:
  case 248:
    inputs_used = 3;
    task_code = LOGIC_TASK_BM;
    break;
  case 243:
  case 245:
  case 249:
    inputs_used = 3;
    task_code = LOGIC_TASK_BN;
    break;
  case 247:
  case 251:
  case 253:
    inputs_used = 3;
    task_code = LOGIC_TASK_BO;
    break;
  case 254:
    inputs_used = 3;
    task_code = LOGIC_TASK_BP;
    break;
  }

  int out_code = -1;
  switch (inputs_used) {
  case 0:
    out_code = task_code;
    break;
  case 1:
    out_code = task_code + 2;
    break;
  case 2:
    out_code = task_code + 4;
    break;
  case 3:
    out_code = task_code + 12;
    break;
  }

  done_tasks[0] = out_code;
  done_tasks[1] = -1;
}
