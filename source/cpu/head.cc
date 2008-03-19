//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "cpu.hh"
#include "head.hh"
#include "head.ii"
#include "cpu.ii"

cCPUHead::cCPUHead() {
  main_cpu = NULL;
  cur_cpu = NULL;
  position = 0;
}

cCPUHead::cCPUHead(cBaseCPU * in_cpu, int in_pos) {
  main_cpu = in_cpu;
  cur_cpu  = in_cpu;

  position = in_pos;
  if (in_pos) Adjust();
}

cCPUHead::cCPUHead(const cCPUHead & in_cpu_head) {
  main_cpu = in_cpu_head.main_cpu;
  cur_cpu  = in_cpu_head.cur_cpu;
  position = in_cpu_head.position;
}



void cCPUHead::Adjust() {
  // If we are still in range, stop here!
  if (position >= 0 && position < GetMemory().GetSize()) return;

#ifdef LINEAR_MEMORY
  Set( main_cpu->GetHeadPosition(cur_cpu, position) );
  main_cpu->SetParasite();
#else
  // If the memory is gone, just stick it at the begining of its parent.
  if (GetMemory().GetSize() == 0) {
    cur_cpu = main_cpu;
    position = 0;
  }
  /*
  else {
    // Find out how far past the start of the current memory we are in, and
    // cut that off.
    if (position >= GetMemory().GetSize()) position -= GetMemory().GetSize();

    // Loop back to the main CPU.
    cur_cpu = main_cpu;

    // And bring the head into range there.
    position %= GetMemory().GetSize();
    if (position <= 0) position += GetMemory().GetSize();
  }
  */

  else if (position <= 0) { position = 0; }
  else if (position >= GetMemory().GetSize()) {
    position -= GetMemory().GetSize();
    while (position >= GetMemory().GetSize()) {
      // position back at the begining of the creature.
      position %= GetMemory().GetSize();
      
      // Always loop into the begining of the owner CPU.
      cur_cpu = main_cpu;
    }
  }
#endif
}



/////////////////////////////////////////////////////////////////////////
// Method: cCPUHead::FindLabel(direction)
//
// Search in 'direction' (+ or - 1) from this head for the compliment of
//  the label in 'next_label' and return a head to the resulting pos.
//
/////////////////////////////////////////////////////////////////////////

cCPUHead cCPUHead::FindLabel(const cCodeLabel & label, int direction)
{
  // Make sure the label is of size > 0.
  if (label.GetSize() == 0) {
    return *this;
  }

  int found_pos = -1;

  // Call special functions depending on if jump is forwards or backwards.
  if( direction < 0 ) {
    found_pos =
      FindLabel_Backward(label, GetMemory(), GetPosition() - label.GetSize());
  }

  // Jump forwards.
  else {
    found_pos = FindLabel_Forward(label, GetMemory(), GetPosition());
  }

  if (found_pos >= 0) {
    // Return the last line of the found label, or the starting point.
    cCPUHead search_head(*this);
    search_head.Set(found_pos - 1);
    return search_head;
  }

  // It wasn't found; return the starting position of the instruction pointer.
  return *this;
}

  
// Search forwards for search_label from _after_ position pos in the
// memory.  Return the first line _after_ the the found label.  It is okay
// to find search label's match inside another label.

int cCPUHead::FindLabel_Forward(const cCodeLabel & search_label,
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
  const cInstLib & inst_lib = main_cpu->GetInstLib();

  // Move off the template we are on.
  pos += label_size;
  
  // Search until we find the complement or exit the memory.
  while (pos < search_mem.GetSize()) {

    // If we are within a label, rewind to the beginning of it and see if
    // it has the proper sub-label that we're looking for.

    if (inst_lib.IsNop(search_mem.Get(pos))) {
      // Find the start and end of the label we're in the middle of.

      int start_pos = pos;
      int end_pos = pos + 1;
      while (start_pos > search_start &&
	     inst_lib.IsNop( search_mem.Get(start_pos - 1) )) {
	start_pos--;
      }
      while (end_pos < search_mem.GetSize() &&
	     inst_lib.IsNop( search_mem.Get(end_pos) )) {
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
	      inst_lib.GetNopMod(search_mem.Get(offset + matches))) {
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

int cCPUHead::FindLabel_Backward(const cCodeLabel & search_label,
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
  const cInstLib & inst_lib = main_cpu->GetInstLib();

  // Move off the template we are on.
  pos -= label_size;
  
  // Search until we find the complement or exit the memory.
  while (pos >= 0) {
    // If we are within a label, rewind to the beginning of it and see if
    // it has the proper sub-label that we're looking for.

    if (inst_lib.IsNop(search_mem.Get(pos))) {
      // Find the start and end of the label we're in the middle of.

      int start_pos = pos;
      int end_pos = pos + 1;
      while (start_pos > 0 && inst_lib.IsNop(search_mem.Get(start_pos - 1))) {
	start_pos--;
      }
      while (end_pos < search_start &&
	     inst_lib.IsNop(search_mem.Get(end_pos))) {
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
	      inst_lib.GetNopMod(search_mem.Get(offset + matches))) {
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
