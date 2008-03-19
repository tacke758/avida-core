//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "slice.hh"

#include "merit.hh"
#include "tools.hh"
#include "functions.hh"


using namespace std;


////////////////
//  cSchedule
////////////////

cSchedule::cSchedule(int _item_count) : item_count(_item_count)
{
}

cSchedule::~cSchedule()
{
}


////////////////////
//  cConstSchedule
////////////////////

cConstSchedule::cConstSchedule(int _item_count)
  : cSchedule(_item_count), is_active(_item_count)
{
  last_id = 0;
  is_active.SetAll(false);
}

cConstSchedule::~cConstSchedule()
{
}

bool cConstSchedule::OK()
{
  assert(last_id == 0 || last_id < item_count);  //  invalid last_id
  return true;
}

void cConstSchedule::Adjust(int item_id, const cMerit & merit)
{
  if (merit == 0.0) is_active[item_id] = false;
  else is_active[item_id] = true;
}


int cConstSchedule::GetNextID()
{
  // Grab the next ID...
  if (++last_id == item_count) last_id = 0;

  // Make sure we actually have an active ID...
  while (is_active[last_id] == false) {
    if (++last_id == item_count) last_id = 0;
  }
  return last_id;
}


///////////////////
//  cProbSchedule
///////////////////

// The larger merits cause problems here; things need to be re-thought out.

cProbSchedule::cProbSchedule(int _item_count)
  : cSchedule(_item_count)
  , chart(_item_count)
{
}

cProbSchedule::~cProbSchedule()
{
}


int cProbSchedule::GetNextID()
{
  assert(chart.GetTotalWeight() > 0);
  const double position = g_random.GetDouble(chart.GetTotalWeight());
  return chart.FindPosition(position);
}

void cProbSchedule::Adjust(int item_id, const cMerit & item_merit)
{
  chart.SetWeight(item_id, item_merit.GetDouble());
}



/////////////////////////////
//  cIntegratedScheduleNode
/////////////////////////////

cIntegratedScheduleNode::cIntegratedScheduleNode(int _item_count, int in_id)
  : active_array(_item_count)
{
  // Store the input variables.

  node_id = in_id;

  // Initialize the remaining variables.

  for (int i = 0; i < active_array.GetSize(); i++) {
    active_array[i] = 0;
  }
  first_entry = -1;
  active_entry = -1;
  size = 0;
  process_size = 1;
  process_count = 0;
  execute = true;
  next = NULL;
  prev = NULL;
}

cIntegratedScheduleNode::~cIntegratedScheduleNode()
{
}


bool cIntegratedScheduleNode::OK()
{
  bool result = true;

  // Make sure the active_array is setup correctly.

  int size_check = 0;
  int next_check = first_entry;
  for (int i = 0; i < active_array.GetSize(); i++) {
    if (active_array[i] != 0) {
      size_check++;
      assert(next_check == i);  //  Node entries do no match!
      next_check = active_array[i];
    }
  }
  assert(next_check == -1);  // Node array not properly terminated.

  // Make sure the sizes line up...
  assert(size == size_check);  // size and active node count mismatch.

  return result;
}

void cIntegratedScheduleNode::Insert(int item_id)
{
  assert(item_id >= 0 && item_id < active_array.GetSize());  // Illegal ID

  // If this item is already active in this node, ignore this call...
  if (active_array[item_id] != 0) return;

  // See if we're dealing with a new first_entry...
  if (first_entry == -1 || item_id < first_entry) {
    active_array[item_id] = first_entry;
    first_entry = item_id;
  }
  else {
    // Otherwise find the predecessor to this item in the list...
    int prev_item;
    for (prev_item = item_id - 1; prev_item >= 0; prev_item--) {
      if (active_array[prev_item] != 0) break;
    }
    assert(prev_item >= 0);  // prev_item is first, but not identified.
    
    // Make the predecessor point to it, and have it point to the CPU that
    // the old predecessor pointed to.
    active_array[item_id] = active_array[prev_item];
    active_array[prev_item] = item_id;
  }

  size++;
}

void cIntegratedScheduleNode::Remove(int item_id)
{
  assert(item_id >= 0 && item_id < active_array.GetSize()); // Illegal ID

  // If this item is already inactive, ignore this call...
  if (active_array[item_id] == 0) return;

  // If this is the first_entry, adjust it!
  if (first_entry == item_id) {
    first_entry = active_array[item_id];
  }
  else {
    // Find the predecessor to this item in the list...
    int prev_item;
    for (prev_item = item_id - 1; prev_item >= 0; prev_item--) {
      if (active_array[prev_item] != 0) break;
    }
    assert(prev_item >= 0);  // prev_item is first, but not identified.

    // Make the predecessor point to the item removed used to point to.
    active_array[prev_item] = active_array[item_id];
  }

  active_array[item_id] = 0;
  size--;
}


// Execute everything on list, and then shift to calling the next node.
// Wait for the next node to return a -1 before shifting back to this one.

int cIntegratedScheduleNode::GetNextID()
{
  // Alternate between this node's Process and the next's.
  if (execute == false) {
    // If there is a next node, we may be working on it... 
    int next_id = -1;
    if (next != NULL) next_id = next->GetNextID();
   
    // If next_id is a -1, either we don't have a next node, or else it
    // is finished with its execution.

    if (next_id == -1) {
      execute = true;
      process_count = 0;
      active_entry = -1;
    }

    return next_id;
  }

  // Find the next active_entry...
  
  // If we were at the end of the list, start over...
  if (active_entry == -1) active_entry = first_entry;

  // If this entry no longer exists, hunt for the next active entry manually...
  else if (active_array[active_entry] == 0) {
    while (active_entry < active_array.GetSize() &&
	   active_array[active_entry] == 0) {
      active_entry++;
    }
    if (active_entry == active_array.GetSize()) active_entry = -1;
  }
  
  // Otherwise, if the entry does exist, we can just look the next one up.
  else active_entry = active_array[active_entry];


  // If we have now hit the end of this list, move on to the next node.

  if (active_entry == -1) {
    process_count++;
    if (process_count >= process_size) execute = false;
  }

//  cout << "Running " << active_entry << " from node " << node_id
//       << " (size = " << size << ", first = " << first_entry << ")" << endl;

  return active_entry;
}


/////////////////////////
//  cIntegratedSchedule
/////////////////////////

cIntegratedSchedule::cIntegratedSchedule(int _item_count)
  : cSchedule(_item_count)
{
  num_active_nodes = 0;

  // Create the merit_chart; they should all init to default value.
  merit_chart = new cMerit[item_count];
  for (int i = 0; i < item_count; i++) {
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

bool cIntegratedSchedule::OK()
{
  // Test that we have an acurate count of the number of active nodes.

  int test_active_nodes = 0;
  for (int i = 0; i < node_array.GetSize(); i++) {
    if (node_array[i]) test_active_nodes++;
  }

  assert(test_active_nodes == num_active_nodes); // active_node count mismatch.

  // Test each node to make sure it is OK.

  for (int i = 0; i < node_array.GetSize(); i++) {
    if (node_array[i] != NULL) node_array[i]->OK();
  }

  return true;
}


void cIntegratedSchedule::Adjust(int item_id, const cMerit & new_merit)
{
  // Grab the old_merit, the new merit, and compare them.
  const cMerit old_merit = merit_chart[item_id];

  // If the merit is still the same, we're done here.
  if (old_merit == new_merit) return;

  // Save the new merit to the merit_chart.
  merit_chart[item_id] = new_merit;

  // Re-adjust the lists.
  int merit_magnitude = Max( old_merit.GetNumBits(), new_merit.GetNumBits() );
  for (int i = 0; i < merit_magnitude; i++) {
    bool old_bit = old_merit.GetBit(i);
    bool new_bit = new_merit.GetBit(i);

    if (old_bit && !new_bit) {
      // Remove the item from this node...
      node_array[i]->Remove(item_id);
      if (node_array[i]->GetSize() == 0) RemoveNode(i);
    }

    if (!old_bit && new_bit) {
      // Add the item from this node...
      if (i >= node_array.GetSize() || !node_array[i]) InsertNode(i);
      node_array[i]->Insert(item_id);
    }
  }
}


int cIntegratedSchedule::GetNextID()
{
  assert(node_array.GetSize() > 0);  // Running scheduler w/ no entries!

  int next_id = -1;
  while (next_id < 0) {
    next_id = node_array[node_array.GetSize() - 1]->GetNextID();
  }
  
  return next_id;
}

double cIntegratedSchedule::GetStatus(int id)
{
  return merit_chart[id].GetDouble();
}


///////// --- private //////////

void cIntegratedSchedule::InsertNode(int node_id)
{
  // Test if trying to create node that already exists.
  assert(node_id >= node_array.GetSize() || node_array[node_id] == NULL);

  cIntegratedScheduleNode * new_node =
    new cIntegratedScheduleNode(item_count, node_id);

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
  assert(node_array[node_id] != NULL); // Trying to remove non-existant node.

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
