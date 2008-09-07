//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "code_array.hh"
#include "classes.hh"
#include "stats.hh" // for stats

///////////////////////////
//  cCodeArray
///////////////////////////

cCodeArray::cCodeArray()
{
  // Initialize the code array as empty.

  g_memory.Add(C_CODE_ARRAY);
  size = 0;
  data = NULL;
  flags = NULL;
}

cCodeArray::cCodeArray(int in_size)
{
  // Initialize the code array randomly.

  g_memory.Add(C_CODE_ARRAY);
  size = in_size;
  data = g_memory.Get(size);
  flags = g_memory.Get(size);
  
  for (int i = 0; i < size; i++) {
    data[i] = g_random.GetUInt(stats.GetNumInstructions());
    flags[i] = 0;
  }
}

cCodeArray::cCodeArray(const cCodeArray &in_code_array)
{
  size = in_code_array.size;
  data = g_memory.Get(size);
  flags = g_memory.Get(size);

  for (int i = 0; i < size; i++) {
    data[i] = in_code_array.data[i];
    flags[i] = in_code_array.flags[i];
  }
}

cCodeArray::~cCodeArray()
{
  g_memory.Remove(C_CODE_ARRAY);

  if (data) g_memory.Free(data, size);
  if (flags) g_memory.Free(flags, size);
}

int cCodeArray::CountFlag(int flag)
{
  int count = 0;
  for (int i = 0; i < size; i++) {
    if (GetFlag(i, flag)) count++;
  }
  return count;
}

void cCodeArray::Resize(int new_size)
{
  // If we are not changing the size, just return.
  if (new_size == size) return;

  // Otherwise, re-construct the arrays.
  char * new_data = g_memory.Get(new_size);
  char * new_flags = g_memory.Get(new_size);

  int i;
  for (i = 0; i < size && i < new_size; i++) {
    new_data[i] = data[i];
    new_flags[i] = flags[i];
  }
  for (i = size; i < new_size; i++) {
    new_data[i] = g_random.GetUInt(stats.GetNumInstructions());
    new_flags[i] = 0;
  }

  if (data) g_memory.Free(data, size);
  if (flags) g_memory.Free(flags, size);

  size = new_size;
  data = new_data;
  flags = new_flags;
}

void cCodeArray::Insert(int line_num, char new_code)
{
  // Re-construct the arrays.
  char * new_data = g_memory.Get(size + 1);
  char * new_flags = g_memory.Get(size + 1);

  int i;
  for (i = 0; i < line_num; i++) {
    new_data[i] = data[i];
    new_flags[i] = flags[i];
  }
  new_data[line_num] = new_code;
  new_flags[line_num] = 0;
  for (i = line_num; i < size; i++) {
    new_data[i + 1] = data[i];
    new_flags[i + 1] = flags[i];
  }

  if (data) g_memory.Free(data, size);
  if (flags) g_memory.Free(flags, size);

  size++;
  data = new_data;
  flags = new_flags;
}

void cCodeArray::Remove(int line_num)
{
  if (size <= 1) return;

 // Re-construct the arrays.
  char * new_data = g_memory.Get(size - 1);
  char * new_flags = g_memory.Get(size - 1);

  int i;
  for (i = 0; i < line_num; i++) {
    new_data[i] = data[i];
    new_flags[i] = flags[i];
  }
  for (i = line_num + 1; i < size; i++) {
    new_data[i - 1] = data[i];
    new_flags[i - 1] = flags[i];
  }

  if (data) g_memory.Free(data, size);
  if (flags) g_memory.Free(flags, size);

  size--;
  data = new_data;
  flags = new_flags;
}

void cCodeArray::Reset(int new_size)
{
  // If we are clearing it totally, do so.
  if (!new_size) {
    if (data) delete data;
    if (flags) delete flags;
    data = NULL;
    flags = NULL;
    size = 0;
    return;
  }

  // If we are actually changing the size here, re-construct the arrays

  if (new_size != size) {
    char * new_data = g_memory.Get(new_size);
    char * new_flags = g_memory.Get(new_size);

    if (data) g_memory.Free(data, size);
    if (flags) g_memory.Free(flags, size);

    size = new_size;
    data = new_data;
    flags = new_flags;
  }

  // Randomize the initial contents of the new array.
  Randomize();
}

void cCodeArray::Reset(const cCodeArray & in_code, int offset)
{
  int new_size = in_code.GetSize() - offset;

#ifdef DEBUG
  if (new_size <= 0) g_debug.Error("Reseting cCodeArray size to <= 0!");
#endif

  // If we are actually changing the size here, re-construct the arrays

  if (new_size != size) {
    char * new_data = g_memory.Get(new_size);
    char * new_flags = g_memory.Get(new_size);

    if (data) g_memory.Free(data, size);
    if (flags) g_memory.Free(flags, size);

    size = new_size;
    data = new_data;
    flags = new_flags;
  }

  // Initialize the contents of the new array.

  for (int i = 0; i < size; i++) {
    data[i] = in_code.Get(i + offset);
    flags[i] = 0;
  }
}

void cCodeArray::Clear()
{
  int i;

  for (i = 0; i < size; i++) {
    data[i] = 0;
    flags[i] = 0;
  }
}

void cCodeArray::Randomize()
{
  for (int i = 0; i < size; i++) {
    data[i] = g_random.GetUInt(stats.GetNumInstructions());
    flags[i] = 0;
  }
}

// Recombine two other code arrays into this one.  Align them so that
// code2 starts at line "offset" of code1.  Copy code1 until line
// "cross_point" and continue by copying from code2.

void cCodeArray::Copy(cCodeArray * code1, cCodeArray * code2,
		      int offset, int cross_point)
{
  int i;

#ifdef DEBUG
  if (cross_point > code1->GetSize()) {
    g_debug.Warning("cross_point > code1.size");
    cross_point = code1->GetSize();
  }
  if (cross_point < offset) {
    g_debug.Warning("cross_point < offset");
    offset = cross_point - 1;
  }
#endif

  for (i = 0; i < cross_point && i < size; i++) {
    data[i] = code1->data[i];
  }
  for (i = cross_point; i < code2->GetSize() + offset && i < size; i++) {
    data[i] = code2->data[i - offset];
  }
}

void cCodeArray::operator=(const cCodeArray & other_code)
{
  // If we need to resize, do so...

  if (other_code.size != size) {
    if (data) g_memory.Free(data, size);
    if (flags) g_memory.Free(flags, size);
    size = other_code.size;
    data = g_memory.Get(size);
    flags = g_memory.Get(size);
  }

  // Now that both code arrays are the same size, copy the other one over.

  for (int i = 0; i < size; i++) {
    data[i] = other_code.data[i];
    flags[i] = other_code.flags[i];
  }
}


void cCodeArray::CopyData(const cCodeArray & other_code)
{
  // If we need to resize, do so...

  if (other_code.size != size) {
    if (data) g_memory.Free(data, size);
    if (flags) g_memory.Free(flags, size);
    size = other_code.size;
    data = g_memory.Get(size);
    flags = g_memory.Get(size);
  }

  // Now that both code arrays are the same size, copy the other one over.

  for (int i = 0; i < size; i++) {
    data[i] = other_code.data[i];
    flags[i] = 0;
  }
}



void cCodeArray::Inject(cCodeArray * other_code, int inject_line)
{
  if (!other_code->GetSize()) return;

  // Setup the new code arrays.

  int new_size = size + other_code->GetSize();
  char * new_data = g_memory.Get(new_size);
  char * new_flags = g_memory.Get(new_size);

#ifdef DEBUG
  if (inject_line > size) {
    g_debug.Error("UD %d.%d: Injecting at %d in creature of size %d!",
		  stats.GetUpdate(), stats.GetSubUpdate(), inject_line, size);
  }
#endif

  // Copy the first section of code in.

  int i;
  for (i = 0; i < inject_line; i++) {
    new_data[i] = data[i];
    new_flags[i] = flags[i];
  }

  // Copy the injected porition...

  for (i = 0; i < other_code->GetSize(); i++) {
    new_data[i + inject_line] = other_code->data[i];
    new_flags[i + inject_line] = 0; // Clear the flags upon injection...
  }

  // And finally, copy the last section of the original code...

  for (i = inject_line; i < size; i++) {
    new_data[i + other_code->GetSize()] = data[i];
    new_flags[i + other_code->GetSize()] = flags[i];
  }

  // And re-assign all of the new arrays.

  if (data) g_memory.Free(data, size);
  if (flags) g_memory.Free(flags, size);
  data = new_data;
  flags = new_flags;
  size = new_size;
}


int cCodeArray::OK()
{
  int result = TRUE;

  if (size < 0) {
    g_debug.Error("Size of code array is < 0!");
    result = FALSE;
  }

  if (size > MAX_CREATURE_SIZE) {
    g_debug.Warning("Size of code array is > MAX_CREATURE_SIZE");
    result = FALSE;
  }

  if (size > 0 && size < MIN_CREATURE_SIZE) {
    g_debug.Warning("UD %d: Size of code array = %d (min=%d)",
		    stats.GetUpdate(), size, MIN_CREATURE_SIZE);
    result = FALSE;
  }

  for (int i = 0; i < size; i++) {
    if (data[i] < 0) {
      g_debug.Error("Code array has negative data!");
      result = FALSE;
    }
    if (data[i] > MAX_INST_SET_SIZE) {
      g_debug.Error("Code array had data > MAX_SIZE!");
      result = FALSE;
    }
  }

  return result;
}

void cCodeArray::TestPrint()
{
#ifdef TEST
  FILE * fp = stats.GetTestFP();
  for (int i=0; i < GetSize(); i++) {
    if (data[i] < 10) fprintf(fp, "%c", data[i] + '0');
    else fprintf(fp, "%c", data[i] + 'A' - 10);
  }
  fprintf(fp, "\n");
#endif
}

int cCodeArray::FindOverlap(cCodeArray * in_code_array, int offset)
{
  int size1 = GetSize();
  int size2 = in_code_array->GetSize();

  if (offset > 0) size1 -= offset;
  else size2 += offset;

  return (size1 > size2) ? size2 : size1;
}

int cCodeArray::FindBestOffset(cCodeArray * in_code_array)
{
  int i;
  int cur_distance, min_distance, best_distance;
  int best_offset = 0;
  int neg_OK = TRUE, pos_OK = TRUE;

  int size1 = GetSize();
  int size2 = in_code_array->GetSize();

  // Initialize to a 0 offset, then radiate out checking the rest.

  best_distance = cur_distance = FindHammingDistance(in_code_array);
  for (i = 1; i < size1 || i < size2; i++) {
    // Check -i offset...

    if (neg_OK) {
      min_distance = size1 + size2 - 2 * FindOverlap(in_code_array, -i);
      if (min_distance >= best_distance) {
	neg_OK = FALSE;
      } else {
	cur_distance = FindHammingDistance(in_code_array, -i);
	if (cur_distance < best_distance) {
	  best_distance = cur_distance;
	  best_offset = -i;
	}
      }
    }

    // Check +i offset...

    if (pos_OK) {
      min_distance = size1 + size2 - 2 * FindOverlap(in_code_array, i);
      if (min_distance > best_distance) {
	pos_OK = FALSE;
      } else {
	cur_distance = FindHammingDistance(in_code_array, i);
	if (cur_distance < best_distance) {
	  best_distance = cur_distance;
	  best_offset = i;
	}
      }
    }
  }

  return best_offset;
}

int cCodeArray::FindLevenschteinDistance(cCodeArray * in_code_array)
{
  // @CAO write function!
  in_code_array = NULL;
  return 0;
}

int cCodeArray::FindSlidingDistance(cCodeArray * in_code_array)
{
  return FindHammingDistance(in_code_array, FindBestOffset(in_code_array));
}

int cCodeArray::FindHammingDistance(cCodeArray * in_code_array, int offset)
{
  int hamming_distance = 0;
  int start1 = (offset < 0) ? 0 : offset;
  int start2 = (offset > 0) ? 0 : -offset;
  int overlap;
  int i;

  // Find the ammount of overlap at this offset.

  overlap = FindOverlap(in_code_array, offset);
  
  // Automatically add anything protruding past the overlap to the
  // hamming distance.

  hamming_distance = GetSize() + in_code_array->GetSize() - 2 * overlap;

  // Cycle through the overlap adding all differences to the distance.

  for (i = 0; i < overlap; i++) {
    if (data[start1 + i] != in_code_array->data[start2 + i]) {
      hamming_distance++;
    }
  }

  return hamming_distance;
}
