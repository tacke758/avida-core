//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "slice.hh"
#include "../cpu/cpu.ii"

////////////////
//  cSchedule
////////////////

cSchedule::cSchedule()
{
}

cSchedule::~cSchedule()
{
}

void cSchedule::Process(int total_slices)
{
  cBaseCPU * cpu_array = population->GetCPUArray();
  int num_cells = population->GetSize();
  int total_exec;

  for (total_exec = 0;
       total_exec < total_slices;
       total_exec += population->GetNumCreatures()) {
    for (int i = 0; i < num_cells; i++) {
      if (cpu_array[i].GetActiveGenotype()){
	cpu_array[i].SingleProcess();
      }
    }
  }

  cStats::AddExecuted(total_exec);
}


////////////////////
//  cConstSchedule
////////////////////

cConstSchedule::cConstSchedule()
{
}

cConstSchedule::~cConstSchedule()
{
}


////////////////////
//  cBlockSchedule
////////////////////

cBlockSchedule::cBlockSchedule()
{
}

cBlockSchedule::~cBlockSchedule()
{
}

void cBlockSchedule::Process(int total_slices)
{
  total_slices = 0;  // total_slices not needed for Block Scheduling.

  cBaseCPU * cpu_array = population->GetCPUArray();
  cBaseCPU * cur_cpu = NULL;
  int num_cells = population->GetSize();
  for (int i = 0; i < num_cells; i++) {
    cur_cpu = &(cpu_array[i]);

    if (cur_cpu) {
      int time_allocated =
	cur_cpu->GetEnvironment()->ScaleMerit(cur_cpu->GetMerit());
      if (cur_cpu->GetFlag(CPU_FLAG_INJECTED)) time_allocated = 256;

      for (int time_used = 0;
	   time_used < time_allocated && cur_cpu->GetMemorySize();
	   time_used++) {
	cur_cpu->SingleProcess();
      }
    }

  }
}


///////////////////
//  cProbSchedule
///////////////////

// The larger merits cause problems  here; things need to be re-thought out.

cProbSchedule::cProbSchedule(int num_cells)
{
  // Calculate the size of the block structure (leave room for 25% growth
  // before each garbage collection...
  int bs_size = (num_cells * 5) / 4;

  // Construct the merit block strcuture.
  merit_bs = new cIndexedBlockStruct(bs_size, bs_size, num_cells);
}

cProbSchedule::~cProbSchedule()
{
  delete merit_bs;
}

void cProbSchedule::Process(int total_slices)
{
  int next_cpu;
  for (int i = 0; i < total_slices; i++) {
    next_cpu = merit_bs->Find(g_random.GetUInt(merit_bs->GetBlocksUsed()),
			      g_random.GetUInt(merit_bs->GetFixedSize()));
    if (next_cpu >= 0){
      population->GetCPU(next_cpu).SingleProcess();
    }
  }

  cStats::AddExecuted(total_slices);
}


// @CAO NOTE!  This currently only works for values up to max_int.
void cProbSchedule::Adjust(cBaseCPU * in_cpu)
{
  int cpu_id = in_cpu->GetEnvironment()->GetID();
  UINT cpu_merit = in_cpu->GetMerit().GetUInt();

  // Remove the old info about this cpu.
  merit_bs->RemoveRef(cpu_id);

  // Try to add the data (within the if statement).  If this fails, we
  // don't have room for any more scaled objects, and thus must compress the
  // data before we can insert anything new.

  if (merit_bs->AddData(cpu_id, cpu_merit) < 0) {
    merit_bs->Compress();
    merit_bs->AddData(cpu_id, cpu_merit);
    merit_bs->CalcFixedSize();
    merit_bs->Process();
  }

  // Now that we have added the data successfully, we must update the
  // structure.  If  this fails, we must not have enough fixed blocks left
  // and must therefore compress.

  else if (merit_bs->Update() < 0) {
    merit_bs->Compress();
    merit_bs->CalcFixedSize();
    merit_bs->Process();
  }
}


//////////////////////
//  cLogScheduleNode
//////////////////////

int cLogScheduleNode::Process()
{
  process_count++;

  // If this node shouldn't go now, pass execution onto the next.
  if (process_count > process_size) {
    process_count = 0;
    if (next) return next->Process();
    else return 0;
  }

  // Otherwise execute all of the CPUs in this node.
  for (current = first; current != NULL; current = current->GetNext()) {
    current->SingleProcess();
  }
  return size;
}

void cLogScheduleNode::Adjust()
{
  if (next) {
    process_count = 1 << (merit_level - next->GetMeritLevel() - 1);
  }
  else {
    process_count = (1 << 30);
  }
}

//////////////////
//  cLogSchedule
//////////////////

cLogSchedule::cLogSchedule(int num_cells)
{
  node_array = NULL;
  first_node = NULL;
  array_size = 0;
  list_size = 0;
  max_node = 0;
  cpu_chart = g_memory.GetInts(num_cells);
  for (int i = 0; i < num_cells; i++) {
    cpu_chart[i] = -1;
  }
}

cLogSchedule::~cLogSchedule()
{
  for (int i = 0; i < array_size; i++) {
    delete node_array[i];
  }
  delete [] node_array;
  g_memory.Free(cpu_chart, population->GetSize());
}

int cLogSchedule::OK()
{
  int result = TRUE;

#ifdef DEBUG

  int i;
  for (i = 0; i < array_size; i++) {
    if (node_array[i] && !node_array[i]->OK()) {
      result = FALSE;
    }
  }

  for (i = 0; i < population->GetSize(); i++) {
    if (cpu_chart[i] > max_node || cpu_chart[i] < -1) {
      g_debug.Error("Value of %d at cpu_chart[%d]; size = %d",
		    cpu_chart[i], i, array_size);
      result = FALSE;
    }
  }
#endif

  return result;
}

void cLogSchedule::Adjust(cBaseCPU * in_cpu)
{
  int cpu_id = in_cpu->GetEnvironment()->GetID();
  int new_node = in_cpu->GetMerit().GetNumBits();

  if (cpu_chart[cpu_id] == new_node) return;

  // Remove this cpu from its old node (if it is in one).

  if (cpu_chart[cpu_id] >= 0) {
    node_array[cpu_chart[cpu_id]]->Remove(in_cpu);

    // If this old node is now empty, we should destroy it.
    if (!node_array[cpu_chart[cpu_id]]->GetSize()) {
      DestroyNode(cpu_chart[cpu_id]);
    }

    // If this CPU is empty, deal with it...
    if (in_cpu->GetActiveGenotype() == NULL) {
      cpu_chart[cpu_id] = -1;
      return;
    }
  }

  // Make sure these are enough nodes to accomodate this new fitness.

  if (new_node >= array_size) {
    Resize(new_node + 1);
  }

  // If this CPU is injected, place it in the top node.

  if (in_cpu->GetFlag(CPU_FLAG_INJECTED)) new_node = max_node;

  // Make sure this particular new node exists, and inject the CPU into
  // it.

  if (!node_array[new_node]) CreateNode(new_node);
  node_array[new_node]->Insert(in_cpu);

  // Update the cpu chart.

  cpu_chart[cpu_id] = new_node;
}

void cLogSchedule::Process(int total_slices)
{
  int i;

  if (array_size) {
    int count = 0;
    for (i = 0;
	 i < total_slices && population->GetNumCreatures();
	 i += first_node->Process()) {
      count++;
    }

    cStats::AddExecuted(i);
  }
}

void cLogSchedule::Resize(int new_size)
{
  int i;
  // Some compilers like the parents around the class pointer tpe, some don't
  cLogScheduleNode ** new_node_array = new cLogScheduleNode * [new_size];

  for (i = 0; i < array_size; i++) {
    new_node_array[i] = node_array[i];
  }
  for (i = array_size; i < new_size; i++) {
    new_node_array[i] = NULL;
  }

  // delete the old information, and move the new over...
  if (node_array) delete [] node_array;
  node_array = new_node_array;
  array_size = new_size;
}

void cLogSchedule::CreateNode(int node_id)
{
  cLogScheduleNode * new_node = new cLogScheduleNode(node_id);

#ifdef DEBUG
  if (array_size <= node_id) g_debug.Error("LogSchedule array too small!");
  if (node_array[node_id]) g_debug.Error("Creating node which exists!");
#endif

  node_array[node_id] = new_node;

  if (!first_node) {
    first_node = new_node;
    max_node = node_id;
  }
  else if (node_id > max_node) {
    new_node->SetNext(first_node);
    first_node->SetPrev(new_node);
    first_node = new_node;
    max_node = node_id;
  }
  else {
    // Find the place to insert this node...

    cLogScheduleNode * cur_node = first_node;
    while (cur_node->GetNext() &&
	   cur_node->GetNext()->GetMeritLevel() > node_id) {
      cur_node = cur_node->GetNext();
    }
    new_node->SetNext(cur_node->GetNext());
    new_node->SetPrev(cur_node);
    if (cur_node->GetNext()) cur_node->GetNext()->SetPrev(new_node);
    cur_node->SetNext(new_node);
  }

  new_node->Adjust();
  if (new_node->GetNext()) new_node->GetNext()->Adjust();
  if (new_node->GetPrev()) new_node->GetPrev()->Adjust();

  list_size++;
}

void cLogSchedule::DestroyNode(int node_id)
{
  cLogScheduleNode * old_node = node_array[node_id];

  // Remove this node from the node list;

  if (old_node == first_node) {
    first_node = first_node->GetNext();
    if (first_node) {
      max_node = first_node->GetMeritLevel();
      first_node->SetPrev(NULL);
      first_node->Adjust();
    }
    else max_node = -1;
  }
  else {
    if (old_node->GetNext()) old_node->GetNext()->SetPrev(old_node->GetPrev());
    if (old_node->GetPrev()) old_node->GetPrev()->SetNext(old_node->GetNext());
    if (old_node->GetNext()) old_node->GetNext()->Adjust();
    if (old_node->GetPrev()) old_node->GetPrev()->Adjust();
  }

  delete node_array[node_id];
  node_array[node_id] = NULL;

  list_size--;
}


/////////////////////////////
//  cIntegratedScheduleNode
/////////////////////////////

cIntegratedScheduleNode::cIntegratedScheduleNode(cPopulation * in_population, int in_id)
  : active_array(in_population->GetSize())
{
  // Store the input variables.

  population = in_population;
  node_id = in_id;

  // Initialize the remaining variables.

  for (int i = 0; i < population->GetSize(); i++) {
    active_array[i] = 0;
  }
  first_entry = -1;
  size = 0;
  process_size = 1;
  process_count = 0;
  execute = TRUE;
  next = NULL;
  prev = NULL;
}

cIntegratedScheduleNode::~cIntegratedScheduleNode()
{
}


int cIntegratedScheduleNode::OK()
{
  int result = TRUE;

  // Make sure the active_array is setup correctly.

  int size_check = 0;
  int next_check = first_entry;
  for (int i = 0; i < population->GetSize(); i++) {
    if (active_array[i] != 0) {
      size_check++;
      if (next_check != i) {
	// The entries do not mactch.
	g_debug.Error("next_check = %d, but entry found at %d",
		      next_check, i);
	result = FALSE;
	break;
      }
      next_check = active_array[i];
    }
  }

  // Make sure the sizes line up...
  if (size != size_check) {
    g_debug.Error("UD %d: size = %d, but %d active nodes were found!)",
		  cStats::GetUpdate(), size, size_check);
    result = FALSE;
  }

  return result;
}

void cIntegratedScheduleNode::Insert(int cpu_id)
{
  // If this CPU is already active in this node, ignore this call...
  if (active_array[cpu_id]) return;

  // Find the predecessor to this CPU in the list...
  int prev_cpu;
  for (prev_cpu = cpu_id - 1; prev_cpu >= 0; prev_cpu--) {
    if (active_array[prev_cpu]) break;
  }

  // If this is the new first element in the list, set it as such, and link
  // it to the old first entry.
  if (prev_cpu < 0) {
    active_array[cpu_id] = first_entry;
    first_entry = cpu_id;
  }

  // Otherwise make the predecessor point to it, and have it point to the
  // CPU that the old predecessor pointed to.
  else {
    active_array[cpu_id] = active_array[prev_cpu];
    active_array[prev_cpu] = cpu_id;
  }

  size++;
}

void cIntegratedScheduleNode::Remove(int cpu_id)
{
  // If this CPU is already inactive, ignore this call...
  if (!active_array[cpu_id]) return;

  // Find the predecessor to this CPU in the list...
  int prev_cpu;
  for (prev_cpu = cpu_id - 1; prev_cpu >= 0; prev_cpu--) {
    if (active_array[prev_cpu]) break;
  }

  // If this was the first element in the list, re-link the first_entry.
  if (prev_cpu < 0) {
    first_entry = active_array[cpu_id];
    active_array[cpu_id] = 0;
  }

  // Otherwise make the predecessor point to the cpu it used to point to.
  else {
    active_array[prev_cpu] = active_array[cpu_id];
    active_array[cpu_id] = 0;
  }

  size--;
}

int cIntegratedScheduleNode::Process()
{
  int num_executed = 0;

  // Alternate between this node's Process and the next's.
  if (execute == FALSE) {
    execute = TRUE;
    if (process_count >= process_size) {
      process_count = 0;
      if (next) return next->Process();
    }
    return 0;
  }

  //  static FILE * fp = fopen ("slice.out", "w");
  //  fprintf(fp, "%d\n", node_id);

  cBaseCPU * cpu_array = population->GetCPUArray();

  // Loop through the creatures active in this node and execute them.
  for (int cur_cpu = first_entry; cur_cpu != -1;){
    cpu_array[cur_cpu].SingleProcess();

    ++num_executed;
    cStats::IncNumInstExecuted();

    // If the execution of this command has cause this CPU to change its
    // merit such that it is no longer part of this node, we must manually
    // hunt for the next creature.
    if (!(active_array[cur_cpu])) {
      while (cur_cpu < population->GetSize() && !(active_array[cur_cpu]))
	cur_cpu++;
      if (cur_cpu >= population->GetSize()) cur_cpu = -1;
    }

    // Otherwise we just continue to the creature which is pointed to as
    // being next...
    else cur_cpu = active_array[cur_cpu];
  }
  process_count++;
  execute = FALSE;

  return num_executed;
}


/////////////////////////
//  cIntegratedSchedule
/////////////////////////

cIntegratedSchedule::cIntegratedSchedule(cPopulation * in_population)
{
  population = in_population;
  num_active_nodes = 0;

  // Create the merit_chart; they should all init to default value.
  merit_chart = new cMerit[population->GetSize()];
  for (int i = 0; i < population->GetSize(); i++) {
    merit_chart[i] = 0;
  }
}

cIntegratedSchedule::~cIntegratedSchedule()
{
  for (int i = 0; i < node_array.GetSize(); i++) {
    if (node_array[i] != NULL) delete node_array[i];
  }
  delete [] merit_chart;
}

int cIntegratedSchedule::OK()
{
  int result = TRUE;

  // Test to make sure we have an acurate count of the number of active
  // nodes.

  int test_active_nodes = 0;
  int i;
  for (i = 0; i < node_array.GetSize(); i++) {
    if (node_array[i]) test_active_nodes++;
  }

  if (test_active_nodes != num_active_nodes) {
    g_debug.Error("num_active_nodes = %d, but %d were found.",
		  num_active_nodes, test_active_nodes);
    result = FALSE;
  }

  // Test each node to make sure it is OK.

  for (i = 0; i < node_array.GetSize(); i++) {
    if (node_array[i] && !node_array[i]->OK()) {
      result = FALSE;
      g_debug.Warning("node_array[%d] is failing OK()!", i);
    }
  }

  if (!result) g_debug.Warning("cIntegratedSchedule::OK() failed at UD %d",
			       cStats::GetUpdate());
			
  return result;
}


void cIntegratedSchedule::Adjust(cBaseCPU * in_cpu)
{
  // Grab the old_merit, the new merit, and compare them.
  const int cpu_id = in_cpu->GetEnvironment()->GetID();
  cMerit new_merit = in_cpu->GetMerit();         // @CAO add Random factor...
  cMerit old_merit = merit_chart[cpu_id];

  // If this CPU is empty, set new_merit to 0...
  if (in_cpu->GetActiveGenotype() == NULL) new_merit = 0;

  // If the merit is still the same, we're done here.
  if (old_merit == new_merit) return;

  // Save the new merit to the merit_chart.
  merit_chart[cpu_id] = new_merit;

  // Re-adjust the lists.
  int merit_magnitude = Max( old_merit.GetNumBits(), new_merit.GetNumBits() );
  for (int i = 0; i < merit_magnitude; i++) {
    bool old_bit = old_merit.GetBit(i);
    bool new_bit = new_merit.GetBit(i);

    if (old_bit && !new_bit) {
      // Remove the CPU from this node...
      node_array[i]->Remove(cpu_id);
    }

    if (!old_bit && new_bit) {
      // Add the CPU from this node...
      if (i >= node_array.GetSize() || !node_array[i]) InsertNode(i);
      node_array[i]->Insert(cpu_id);
    }
  }
}

void cIntegratedSchedule::Process(int total_slices)
{
  // Make sure there exist nodes to be run...
  if (node_array.GetSize() == 0) return;

  // And run them until for at least total_slices num of instructions.

  int slices_used = 0;
  while (slices_used < total_slices && population->GetNumCreatures()) {
    slices_used += node_array[node_array.GetSize() - 1]->Process();
  }

  cStats::AddExecuted(slices_used);

  // Move through the node array deleteing all nodes who are now empty.

  for (int i = 0; i < node_array.GetSize(); i++) {
    if (node_array[i] && !node_array[i]->GetSize())
      RemoveNode(i);
  }
}

void cIntegratedSchedule::InsertNode(int node_id)
{
#ifdef DEBUG
  if (node_id < node_array.GetSize() && node_array[node_id])
    g_debug.Error("Trying to create node %d which already exists", node_id);
#endif

  cIntegratedScheduleNode * new_node =
    new cIntegratedScheduleNode(population, node_id);

  if (node_id >= node_array.GetSize()) ResizeNodes(node_id);

  node_array[node_id] = new_node;

  // Find the node to mark as the 'prev'.
  for (int prev_id = node_id + 1; prev_id < node_array.GetSize(); prev_id++) {
    cIntegratedScheduleNode * prev_node = node_array[prev_id];
    if (prev_node) {
      new_node->SetPrev(prev_node);
      prev_node->SetNext(new_node);
      prev_node->SetProcessSize(1 << (prev_id - node_id - 1));
      break;
    }
  }

  // And find the node to mark as the 'next'.
  for (int next_id = node_id - 1; next_id >= 0; next_id--) {
    cIntegratedScheduleNode * next_node = node_array[next_id];
    if (next_node) {
      new_node->SetNext(next_node);
      next_node->SetPrev(new_node);
      new_node->SetProcessSize(1 << (node_id - next_id - 1));
      break;
    }
  }

  num_active_nodes++;
}

void cIntegratedSchedule::RemoveNode(int node_id)
{
#ifdef DEBUG
  if (!node_array[node_id])
    g_debug.Error("Trying to remove node %d which does not exist", node_id);
#endif

  cIntegratedScheduleNode * old_node = node_array[node_id];
  cIntegratedScheduleNode * next_node = old_node->GetNext();
  cIntegratedScheduleNode * prev_node = old_node->GetPrev();
  node_array[node_id] = NULL;

  if (next_node) next_node->SetPrev(prev_node);
  if (prev_node) {
    prev_node->SetNext(next_node);
    prev_node->SetProcessSize(old_node->GetProcessSize() *
			      prev_node->GetProcessSize() * 2);
  }

  if (node_id == node_array.GetSize() - 1) {
    if (!old_node->GetNext()) ResizeNodes(0);
    else ResizeNodes(old_node->GetNext()->GetID());
  }

  delete old_node;

  num_active_nodes--;
}

void cIntegratedSchedule::ResizeNodes(int new_max)
{
  int old_size = node_array.GetSize();
  int new_size = new_max + 1;  // 0 to new_max...

  // Clean up tail portions of the array being cut off.
  for (int i = new_size; i < old_size; i++) {
    if (node_array[i]) delete node_array[i];
  }

  node_array.Resize(new_size);

  // Mark as NULL any new cells added to the array.
  for (int i = old_size; i < new_size; i++) {
    node_array[i] = NULL;
  }
}
