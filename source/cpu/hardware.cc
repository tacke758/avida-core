//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "hardware.hh"
#include "head.ii"

cHardware::cHardware()
{
  Reset();
}

cHardware::~cHardware()
{
}

void cHardware::Reset(cBaseCPU * in_cpu)
{
  int i;
  for (i = 0; i < NUM_REGISTERS; i++) {
    reg[i] = 0;
  }
  for (i = 0; i < STACK_SIZE; i++) {
    stack[i] = 0;
  }
  stack_pointer = 0;
  inst_pointer.Reset(in_cpu);
  input_pointer = 0;

  input_buffer[0] = 15;
  input_buffer[1] = 51;
  input_buffer[2] = 85;
}

int cHardware::OK()
{
  int result = TRUE;

  // Any checks we can do with the pointers?
  if (stack_pointer >= STACK_SIZE) {
    g_debug.Warning("UD %d: stack_pointer=%d (out of range)",
	    stats.GetUpdate(), stack_pointer);
    result = FALSE;
  }
  if (input_pointer >= IO_SIZE) {
    g_debug.Warning("IO pointer is out of range");
    result = FALSE;
  }

  if (!memory.OK()) result = FALSE;
  if (!next_label.OK()) result = FALSE;

  return result;
}

// Search in 'direction' (+ or - 1) from the instruction pointer for the
// compliment of the label in 'next_label' and return a pointer to
// the results.
cCPUHead cHardware::FindLabel(int direction)
{
  // IDEALY:
  // Keep making jumps (in the proper direction) equal to the label
  // length.  If we are inside of a label, check its size, and see if
  // any of the sub-labels match properly.
  // FOR NOW:
  // Get something which works, no matter how inefficient!!!

  cCPUHead temp_head(inst_pointer);
  
  // IDEALY : Re-adjust the start position...
  
  while (temp_head.InMemory()) {
    // IDEALY: Analyze the label we are in; see if the one we are looking
    // for could be a sub-label of it.  Skip past it if not.

    int i;
    for (i = 0; i < next_label.GetSize(); i++) {
      if (next_label.GetNop(i) != temp_head.GetInst(i)) {
	break;
      }
    }
    if (i == next_label.GetSize()) {
      temp_head.AbsJump(i - 1);
      return temp_head;
    }

    temp_head.AbsJump(direction);     // IDEALY: MAKE LARGER JUMPS
  }

  return inst_pointer;
}

// Search for 'in_label' anywhere in the CPU.
cCPUHead cHardware::FindLabel(cCodeLabel * in_label, int direction)
{
  // IDEALY:
  // Keep making jumps (in the proper direction) equal to the label
  // length.  If we are inside of a label, check its size, and see if
  // any of the sub-labels match properly.
  // FOR NOW:
  // Get something which works, no matter how inefficient!!!

  cCPUHead temp_head(inst_pointer.GetMainCPU());
  
  while (temp_head.InMemory()) {
    // IDEALY: Analyze the label we are in; see if the one we are looking
    // for could be a sub-label of it.  Skip past it if not.
 
    int i;
    for (i = 0; i < in_label->GetSize(); i++) {
      if (in_label->GetNop(i) != temp_head.GetInst(i)) {
	break;
      }
    }
    if (i == next_label.GetSize()) {
      temp_head.AbsJump(i - 1);
      return temp_head;
    }

    temp_head.AbsJump(direction);     // IDEALY: MAKE LARGER JUMPS
  }

  temp_head.AbsSet(-1);
  return temp_head;
}

// @CAO: direction is not currently used; should be used to indicate the
// direction which the inst_pointer should progress through a creature.
cCPUHead cHardware::FindFullLabel(const cCodeLabel & in_label, int direction)
{
  (void) direction;
  cCPUHead temp_head(inst_pointer.GetMainCPU());

  while (temp_head.InMemory()) {
    // If we are not in a label, jump to the next checkpoint...
    if (temp_head.GetInst() >= NUM_NOPS) {
      temp_head.AbsJump(in_label.GetSize());
      continue;
    }

    // Otherwise, rewind to the begining of this label...

    while (!(temp_head.AtFront()) && temp_head.GetInst(-1) < NUM_NOPS)
      temp_head.AbsJump(-1);

    // Calculate the size of the label being checked, and make sure they
    // are equal.

    int checked_size = 0;
    while (temp_head.GetInst(checked_size) < NUM_NOPS) {
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
      if (in_label.GetNop(j) != temp_head.GetInst(j)) {
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

  // Adjust the instruction pointer to take into account the new mem size.

  if (!inst_pointer.TestParasite() && inst_pointer.GetPosition() > line_num)
    inst_pointer.Jump(inject_code->GetSize());
}
