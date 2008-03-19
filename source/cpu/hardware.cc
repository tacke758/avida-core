//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "hardware.hh"
#include "hardware.ii"
#include "head.ii"

#include <iomanip.h>

////////////////
//  cCPUStack
////////////////

cCPUStack::cCPUStack()
{
  Clear();
}

cCPUStack::cCPUStack(const cCPUStack & in_stack)
{
  for (int i = 0; i < STACK_SIZE; i++) {
    stack[i] = in_stack.stack[i];
  }
  stack_pointer = in_stack.stack_pointer;
}

cCPUStack::~cCPUStack()
{
}

void cCPUStack::operator=(const cCPUStack & in_stack)
{
  for (int i = 0; i < STACK_SIZE; i++) {
    stack[i] = in_stack.stack[i];
  }
  stack_pointer = in_stack.stack_pointer;
}

void cCPUStack::Flip()
{
  int new_stack[STACK_SIZE];
  int i;
  for (i = 0; i < STACK_SIZE; i++) new_stack[i] = Pop();
  for (i = 0; i < STACK_SIZE; i++) Push(new_stack[i]);
}

int cCPUStack::OK()
{
  int result = TRUE;

  // Any checks we can do with the pointers?
  if (stack_pointer >= STACK_SIZE) {
    g_debug.Warning("UD %d: stack_pointer=%d (out of range)",
	    cStats::GetUpdate(), stack_pointer);
    result = FALSE;
  }

  return result;
}


////////////////
//  cCPUThread
////////////////

cCPUThread::cCPUThread(cBaseCPU * in_cpu, int _id) :
 input_buf(cConfig::GetNumTasks(), INPUT_BUF_SIZE),
 output_buf(cConfig::GetNumTasks(), OUTPUT_BUF_SIZE) {
   Reset(in_cpu, _id);
}

cCPUThread::cCPUThread(const cCPUThread & in_thread, int _id) :
 input_buf(cConfig::GetNumTasks(), INPUT_BUF_SIZE),
 output_buf(cConfig::GetNumTasks(), OUTPUT_BUF_SIZE) {
   id = _id;
   if (id == -1) id = in_thread.id;
   for (int i = 0; i < NUM_REGISTERS; i++) {
     reg[i] = in_thread.reg[i];
   }
   for (int i = 0; i < NUM_HEADS; i++) {
     heads[i] = in_thread.heads[i];
   }
   stack = in_thread.stack;
   input_pointer = in_thread.input_pointer;
}

cCPUThread::~cCPUThread() {}

void cCPUThread::operator=(const cCPUThread & in_thread)
{
  id = in_thread.id;
  for (int i = 0; i < NUM_REGISTERS; i++) {
    reg[i] = in_thread.reg[i];
  }
  for (int i = 0; i < NUM_HEADS; i++) {
    heads[i] = in_thread.heads[i];
  }
  stack = in_thread.stack;
  input_pointer = in_thread.input_pointer;
  input_buf = in_thread.input_buf;
  output_buf = in_thread.output_buf;
}

void cCPUThread::Reset(cBaseCPU * in_cpu, int _id)
{
  id = _id;

  for (int i = 0; i < NUM_REGISTERS; i++) reg[i] = 0;
  for (int i = 0; i < NUM_HEADS; i++) heads[i].Reset(in_cpu);

  stack.Clear();
  cur_stack = 0;
  cur_head = HEAD_IP;
  input_pointer = 0;
  input_buf.Clear();
  output_buf.Clear();
  read_label.Clear();
  next_label.Clear();
}


///////////////
//  cHardware
///////////////

cHardware::cHardware(cBaseCPU * owner_cpu)
{
  num_threads = 0;
  cur_thread = 0;
  time_used = 0;
  thread_time_used = 0;

  threads = NULL;
  Reset(owner_cpu);
}

cHardware::~cHardware()
{
  if (threads != NULL) {
    delete [] threads;
  }
}



void cHardware::Reset(cBaseCPU * owner_cpu)
{
  // If more than one thread exists, delete them!
  if (threads != NULL && num_threads != 1) {
    delete [] threads;
    threads = NULL;
  }

  // If we do not have a thread, build a new one.
  if (threads == NULL) {
    threads = new cCPUThread[1];
  }

  // Reset the thread (we know we only have one now.)
  threads[0].Reset(owner_cpu, 0);
  id_chart.Clear();
  id_chart.SetFlag(0);

  num_threads = 1;
  cur_thread = 0;

  time_used = 0;
  thread_time_used = 0;

  input_buffer[0] = 15 << 24; // 00001111
  input_buffer[1] = 51 << 24; // 00110011
  input_buffer[2] = 85 << 24; // 01010101

  global_stack.Clear();
}

int cHardware::OK()
{
  int result = TRUE;

  if (!memory.OK()) result = FALSE;

  for (int i = 0; i < num_threads; i++) {
    if (threads[i].input_pointer >= IO_SIZE) {
      g_debug.Warning("IO pointer is out of range");
      result = FALSE;
    }

    if (!threads[i].stack.OK()) result = FALSE;
    if (!threads[i].next_label.OK()) result = FALSE;
  }

  return result;
}

void cHardware::PrintStatus(ostream & fp)
{
  fp << time_used << " "
     << "IP:" << GetIPPosition() << "   "
     << setbase(16)
     << "AX:" << GetRegister(REG_AX) << "  "
     << "BX:" << GetRegister(REG_BX) << "  "
     << "CX:" << GetRegister(REG_CX) << endl
     << setbase(10);

  fp << "  R-Head:" << GetHeadPosition(HEAD_READ)  << " "
     << "W-Head:" << GetHeadPosition(HEAD_WRITE) << " "
     << "F-Head:" << GetHeadPosition(HEAD_FLOW)  << "  "
     << "RL:" << GetReadLabel().AsString() << "   "
     << "NEXT: >> " << inst_lib->GetName(GetCurInst())() << " <<"
     << endl;
  fp.flush();

    //  << "Mem:" << GetMemorySize() << " "
    // << "RND:" << g_random.GetUseCount()
}

/////////////////////////////////////////////////////////////////////////
// Method: cHardware::FindLabel(direction)
//
// Search in 'direction' (+ or - 1) from the instruction pointer for the
// compliment of the label in 'next_label' and return a pointer to the
// results.  If direction is 0, search from the beginning of the genome.
//
/////////////////////////////////////////////////////////////////////////

cCPUHead cHardware::FindLabel(int direction)
{
  cCPUHead & inst_ptr = threads[CUR_THREAD].heads[HEAD_IP];

  // Start up a search head at the position of the instruction pointer.
  cCPUHead search_head(inst_ptr);
  cCodeLabel & search_label = threads[CUR_THREAD].next_label;

  // Make sure the label is of size > 0.

  if (search_label.GetSize() == 0) {
    return inst_ptr;
  }

  // Call special functions depending on if jump is forwards or backwards.
  int found_pos = 0;
  if( direction < 0 ) {
    found_pos = FindLabel_Backward(search_label, inst_ptr.GetMemory(),
			   inst_ptr.GetPosition() - search_label.GetSize());
  }

  // Jump forward.
  else if (direction > 0) {
    found_pos = FindLabel_Forward(search_label, inst_ptr.GetMemory(),
			   inst_ptr.GetPosition());
  }

  // Jump forward from the very beginning.
  else {
    found_pos = FindLabel_Forward(search_label, inst_ptr.GetMemory(), 0);
  }
  
  // Return the last line of the found label, if it was found.
  if (found_pos >= 0) search_head.Set(found_pos - 1);

  // Return the found position (still at start point if not found).
  return search_head;
}


// Search forwards for search_label from _after_ position pos in the
// memory.  Return the first line _after_ the the found label.  It is okay
// to find search label's match inside another label.

int cHardware::FindLabel_Forward(const cCodeLabel & search_label,
				   const cCodeArray & search_mem, int pos)
{
#ifdef DEBUG
  if (pos >= search_mem.GetSize() || pos < 0) {
    g_debug.Error("Trying to search from outside of memory space");
  }
#endif

  int search_start = pos;
  int label_size = search_label.GetSize();
  int found_label = FALSE;

  // Move off the template we are on.
  pos += label_size;

  // Search until we find the complement or exit the memory.
  while (pos < search_mem.GetSize()) {

    // If we are within a label, rewind to the beginning of it and see if
    // it has the proper sub-label that we're looking for.

    if (inst_lib->IsNop(search_mem.Get(pos))) {
      // Find the start and end of the label we're in the middle of.

      int start_pos = pos;
      int end_pos = pos + 1;
      while (start_pos > search_start &&
	     inst_lib->IsNop( search_mem.Get(start_pos - 1) )) {
	start_pos--;
      }
      while (end_pos < search_mem.GetSize() &&
	     inst_lib->IsNop( search_mem.Get(end_pos) )) {
	end_pos++;
      }
      int test_size = end_pos - start_pos;

      // See if this label has the proper sub-label within it.
      int max_offset = test_size - label_size + 1;
      int offset = start_pos;
      for (offset = start_pos; offset < start_pos + max_offset; offset++) {

	// Test the number of matches for this offset.
	int matches;
	for (matches = 0; matches < label_size; matches++) {
	  if (search_label.GetNop(matches) !=
	      inst_lib->GetNopMod(search_mem.Get(offset + matches))) {
	    break;
	  }
	}

	// If we have found it, break out of this loop!
	if (matches == label_size) {
	  found_label = TRUE;
	  break;
	}
      }

      // If we've found the complement label, set the position to the end of
      // the label we found it in, and break out.

      if (found_label == TRUE) {
	// pos = end_pos;
	pos = label_size + offset;
	break;
      }

      // We haven't found it; jump pos to just after the current label being
      // checked.
      pos = end_pos;
    }

    // Jump up a block to the next possible point to find a label,
    pos += label_size;
  }

  // If the label was not found return a -1.
  if (found_label == FALSE) pos = -1;

  return pos;
}

// Search backwards for search_label from _before_ position pos in the
// memory.  Return the first line _after_ the the found label.  It is okay
// to find search label's match inside another label.

int cHardware::FindLabel_Backward(const cCodeLabel & search_label,
				   const cCodeArray & search_mem, int pos)
{
#ifdef DEBUG
  if (pos >= search_mem.GetSize()) {
    g_debug.Error("Trying to search from outside of memory space");
  }
#endif

  int search_start = pos;
  int label_size = search_label.GetSize();
  int found_label = FALSE;

  // Move off the template we are on.
  pos -= label_size;

  // Search until we find the complement or exit the memory.
  while (pos >= 0) {
    // If we are within a label, rewind to the beginning of it and see if
    // it has the proper sub-label that we're looking for.

    if (inst_lib->IsNop(search_mem.Get(pos))) {
      // Find the start and end of the label we're in the middle of.

      int start_pos = pos;
      int end_pos = pos + 1;
      while (start_pos > 0 && inst_lib->IsNop(search_mem.Get(start_pos - 1))) {
	start_pos--;
      }
      while (end_pos < search_start &&
	     inst_lib->IsNop(search_mem.Get(end_pos))) {
	end_pos++;
      }
      int test_size = end_pos - start_pos;

      // See if this label has the proper sub-label within it.
      int max_offset = test_size - label_size + 1;
      for (int offset = start_pos; offset < start_pos + max_offset; offset++) {

	// Test the number of matches for this offset.
	int matches;
	for (matches = 0; matches < label_size; matches++) {
	  if (search_label.GetNop(matches) !=
	      inst_lib->GetNopMod(search_mem.Get(offset + matches))) {
	    break;
	  }
	}

	// If we have found it, break out of this loop!
	if (matches == label_size) {
	  found_label = TRUE;
	  break;
	}
      }

      // If we've found the complement label, set the position to the end of
      // the label we found it in, and break out.

      if (found_label == TRUE) {
	pos = end_pos;
	break;
      }

      // We haven't found it; jump pos to just before the current label
      // being checked.
      pos = start_pos - 1;
    }

    // Jump up a block to the next possible point to find a label,
    pos -= label_size;
  }

  // If the label was not found return a -1.
  if (found_label == FALSE) pos = -1;

  return pos;
}

// Search for 'in_label' anywhere in the CPU.
cCPUHead cHardware::FindLabel(cCodeLabel * in_label, int direction)
{
#ifdef DEBUG
  if (!in_label->GetSize()) {
    g_debug.Error("Trying to FindLabel() with size 0 label!");
  }
#endif

  // IDEALY:
  // Keep making jumps (in the proper direction) equal to the label
  // length.  If we are inside of a label, check its size, and see if
  // any of the sub-labels match properly.
  // FOR NOW:
  // Get something which works, no matter how inefficient!!!

  cCPUHead temp_head(threads[CUR_THREAD].heads[HEAD_IP].GetMainCPU());

  while (temp_head.InMemory()) {
    // IDEALY: Analyze the label we are in; see if the one we are looking
    // for could be a sub-label of it.  Skip past it if not.

    int i;
    for (i = 0; i < in_label->GetSize(); i++) {
      if (!inst_lib->IsNop(temp_head.GetInst()) ||
	  in_label->GetNop(i) != inst_lib->GetNopMod(temp_head.GetInst())) {
	break;
      }
    }
    if (i == threads[CUR_THREAD].next_label.GetSize()) {
      temp_head.AbsJump(i - 1);
      return temp_head;
    }

    temp_head.AbsJump(direction);     // IDEALY: MAKE LARGER JUMPS
  }

  temp_head.AbsSet(-1);
  return temp_head;
}

// @CAO: direction is not currently used; should be used to indicate the
// direction which the heads[HEAD_IP] should progress through a creature.
cCPUHead cHardware::FindFullLabel(const cCodeLabel & in_label, int direction)
{
#ifdef DEBUG
  if (!in_label.GetSize()) {
    g_debug.Error("Trying to FindFullLabel() with size 0 label!");
  }
#endif

  (void) direction;
  cCPUHead temp_head(threads[CUR_THREAD].heads[HEAD_IP].GetMainCPU());

  while (temp_head.InMemory()) {
    // If we are not in a label, jump to the next checkpoint...
    if (inst_lib->IsNop(temp_head.GetInst())) {
      temp_head.AbsJump(in_label.GetSize());
      continue;
    }

    // Otherwise, rewind to the begining of this label...

    while (!(temp_head.AtFront()) && inst_lib->IsNop(temp_head.GetInst(-1)))
      temp_head.AbsJump(-1);

    // Calculate the size of the label being checked, and make sure they
    // are equal.

    int checked_size = 0;
    while (inst_lib->IsNop(temp_head.GetInst(checked_size))) {
      checked_size++;
    }
    if (checked_size != in_label.GetSize()) {
      temp_head.AbsJump(checked_size + 1);
      continue;
    }

    // ...and do the comparison...

    int j;
    int label_match = TRUE;
    for (j = 0; j < in_label.GetSize(); j++) {
      if (!inst_lib->IsNop(temp_head.GetInst(j)) ||
	  in_label.GetNop(j) != inst_lib->GetNopMod(temp_head.GetInst())) {
	temp_head.AbsJump(in_label.GetSize() + 1);
	label_match = FALSE;
	break;
      }
    }

    if (label_match) {
      // If we have found the label, return the position after it.
      temp_head.AbsJump(j - 1);
      return temp_head;
    }

    // We have not found the label... increment i.

    temp_head.AbsJump(in_label.GetSize() + 1);
  }

  // The label does not exist in this creature.

  temp_head.AbsSet(-1);
  return temp_head;
}


void cHardware::InjectCode(cCodeArray * inject_code, int line_num)
{
  // Inject the new code.

  memory.Inject(inject_code, line_num);

  // Adjust all of the heads to take into account the new mem size.

  for (int i=0; i < NUM_HEADS; i++) {
    if (!threads[CUR_THREAD].heads[i].TestParasite() &&
	threads[CUR_THREAD].heads[i].GetPosition() > line_num)
      threads[CUR_THREAD].heads[i].Jump(inject_code->GetSize());
  }
}


cCPUHead cHardware::FindArgLabel(int in_label)
{
  g_debug.Comment("Running FindArgLabel(%d)", in_label);

  cCPUHead temp_head(threads[CUR_THREAD].heads[HEAD_IP].GetMainCPU());

  while (temp_head.InMemory()) {
    // If we are at a label, check for a match.
    g_debug.Comment("Checking inst: [%s]", inst_lib->GetName(temp_head.GetInst())());

    if (inst_lib->IsLabel(temp_head.GetInst())) {
      int label_value = temp_head.GetInst().GetArg(0).GetValue();
      int label_range = temp_head.GetInst().GetArg(1).GetValue();
      g_debug.Comment("...found LABEL!  value=%d, range=%d", label_value,
		      label_range);
      if (label_range < 0) label_range *= -1;

      if (in_label <= label_value + label_range &&
	  in_label >= label_value - label_range) {
	// We've found a match!
	break;
      }
    }

    // If we have not found the label... increment head.

    temp_head.AbsJump(1);  // Advance, but don't adjust.
  }

  // The label does not exist in this creature.

  if (!temp_head.InMemory()) temp_head.AbsSet(-1);

  return temp_head;
}


int cHardware::ForkThread()
{
#ifdef THREADS
  if (num_threads == cConfig::GetMaxCPUThreads()) return FALSE;

  cCPUThread * new_threads = new cCPUThread[num_threads + 1];
  for (int i = 0; i < num_threads; i++) {
    new_threads[i] = threads[i];
  }
  new_threads[num_threads] = threads[CUR_THREAD];

  int new_id = 0;
  while (id_chart[new_id] == TRUE) new_id++;
  new_threads[num_threads].SetID(new_id);
  id_chart.SetFlag(new_id);

  delete [] threads;
  threads = new_threads;

  num_threads++;
#endif

  return TRUE;
}

int cHardware::KillThread()
{
#ifdef THREADS
  // Make sure that there is always at least one thread...
  if (num_threads == 1) return FALSE;

  // Note the current thread and set the current back one.
  int kill_thread = cur_thread;
  PrevThread();

  id_chart.UnsetFlag(threads[kill_thread].GetID());

  // Kill the thread!
  cCPUThread * new_threads = new cCPUThread[num_threads - 1];

  int i;
  for (i = 0; i < kill_thread; i++) {
    new_threads[i] = threads[i];
  }
  for (i = kill_thread + 1; i < num_threads; i++) {
    new_threads[i - 1] = threads[i];
  }

  delete [] threads;
  threads = new_threads;

  if (cur_thread > kill_thread) cur_thread--;
  num_threads--;
#endif

  return TRUE;
}




void cHardware::SaveState(ostream & fp){
  // note, memory & child_memory handled by cpu
  // id_chart not handled!
  assert(fp.good());

  fp<<"cHardware"<<endl;

  // input_buffer
  for( int i=0; i<IO_SIZE; ++i ){
    fp<<input_buffer[i]<<endl;
  }

  // global_stack (in inverse order so load can just push)
  global_stack.SaveState(fp);

  fp<<time_used<<endl;
  fp<<thread_time_used<<endl;
  fp<<num_threads<<endl;
  fp<<cur_thread<<endl;

  // Threads
  for( int i=0; i<num_threads; ++i ){
    threads[i].SaveState(fp);
  }
}


void cHardware::LoadState(istream & fp){
  // note, memory & child_memory handled by cpu
  assert(fp.good());

  cString foo;
  fp>>foo;
  assert( foo == "cHardware" );

  // input_buffer
  for( int i=0; i<IO_SIZE; ++i ){
    fp>>input_buffer[i];
  }

  // global_stack
  global_stack.LoadState(fp);

  fp>>time_used;
  fp>>thread_time_used;
  fp>>num_threads;
  fp>>cur_thread;

  // Threads
  for( int i=0; i<num_threads; ++i ){
    threads[i].LoadState(fp);
  }
}



void cCPUStack::SaveState(ostream & fp){
  assert(fp.good());
  fp<<"cCPUStack"<<" ";
  // stack (in inverse order so load can just push)
  for( int i=STACK_SIZE-1; i>=0; --i ){
    fp<<Get(i)<<" ";
  }
  fp<<endl;
}

void cCPUStack::LoadState(istream & fp){
  assert(fp.good());
  cString foo;
  fp>>foo;
  assert( foo == "cCPUStack");
  int value;
  for( int i=0; i<STACK_SIZE; ++i ){
    fp>>value;
    Push(value);
  }
}


void cCPUThread::SaveState(ostream & fp){
  assert(fp.good());
  fp<<"cCPUThread"<<endl;

  // registers
  for( int i=0; i<NUM_REGISTERS; ++i ){
    fp<<reg[i]<<endl;
  }

  // heads (@TCC does not handle parasites!!!)
  for( int i=0; i<NUM_HEADS; ++i ){
    fp<<heads[i].GetPosition()<<endl;
  }

  stack.SaveState(fp);

  fp<<"|"; // marker
  fp<<cur_stack;
  fp<<cur_head;
  fp<<input_pointer;
  fp<<endl;

  // IO buffers
  input_buf.SaveState(fp);
  output_buf.SaveState(fp);

  // Code labels
  read_label.SaveState(fp);
  next_label.SaveState(fp);
}



void cCPUThread::LoadState(istream & fp){
  assert(fp.good());
  cString foo;
  fp>>foo;
  assert( foo == "cCPUThread");

  // registers
  for( int i=0; i<NUM_REGISTERS; ++i ){
    fp>>reg[i];
  }

  // heads (@TCC does not handle parasites!!!)
  for( int i=0; i<NUM_HEADS; ++i ){
    int pos;
    fp>>pos;
    heads[i].AbsSet(pos);
  }

  // stack
  stack.LoadState(fp);

  char marker; fp>>marker; assert( marker == '|' );
  fp.get(cur_stack);
  fp.get(cur_head);
  fp.get(input_pointer);

  // IO buffers
  input_buf.LoadState(fp);
  output_buf.LoadState(fp);

  // Code labels
  read_label.LoadState(fp);
  next_label.LoadState(fp);
}
