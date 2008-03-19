//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
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
  max_size = 0;
  data = NULL;
  flags = NULL;
}

cCodeArray::cCodeArray(int in_size, int in_max_size)
{
#ifdef DEBUG
  if (in_max_size != 0 && in_max_size < in_size) {
    g_debug.Error("Setting a code array of size %d to max size %d\n",
		  in_size, in_max_size);
  }
#endif

  // Initialize the code array randomly.

  g_memory.Add(C_CODE_ARRAY);
  size = in_size;
  if (in_max_size != 0) max_size = in_max_size;
  else max_size = size;

  data = new cInstruction[max_size];
  flags = g_memory.Get(max_size);

  for (int i = 0; i < size; i++) {
    // data[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
    data[i] = cInstruction::GetInstDefault();
    flags[i] = 0;
  }
}

cCodeArray::cCodeArray(const cCodeArray & in_code, int in_max_size)
{
#ifdef DEBUG
  if (in_max_size != 0 && in_max_size < in_code.size) {
    g_debug.Error("Setting a code array of size %d to max size %d\n",
		  in_code.size, in_max_size);
  }
#endif
  g_memory.Add(C_CODE_ARRAY);
  
  size = in_code.size;
  if (in_max_size != 0) max_size = in_max_size;
  else max_size = size;

  data = new cInstruction[max_size];
  flags = g_memory.Get(max_size);

  for (int i = 0; i < size; i++) {
    data[i] = in_code.data[i];
    flags[i] = in_code.flags[i];
  }
}

cCodeArray::cCodeArray(cString in_string)
{
  g_memory.Add(C_CODE_ARRAY);
  
  in_string.RemoveChar('_');  // Remove all blanks from alignments...
  size = in_string.GetSize();
  max_size = size;

  data = new cInstruction[max_size];
  flags = g_memory.Get(max_size);

  for (int i = 0; i < size; i++) {
    data[i].SetSymbol(in_string[i]);
    flags[i] = 0;
  }
}

cCodeArray::~cCodeArray()
{
  g_memory.Remove(C_CODE_ARRAY);

  if (data) delete [] data;
  if (flags) g_memory.Free(flags, max_size);
}

int cCodeArray::CountFlag(int flag) const
{
  int count = 0;
  for (int i = 0; i < size; i++) {
    if (GetFlag(i, flag)) count++;
  }
  return count;
}

void cCodeArray::Resize(int new_size)
{
#ifdef DEBUG
  if (new_size > MAX_CREATURE_SIZE) {
    g_debug.Error("Trying to Resize to %d > MAX_CREATURE_SIZE (%d)",
		  new_size, MAX_CREATURE_SIZE);
  }
#endif

  // If we are not changing the size, just return.
  if (new_size == size) return;

  // Re-construct the arrays only if we have to...
  if (new_size > max_size || new_size * 4 < max_size) {
    cInstruction * new_data = new cInstruction[new_size];
    CA_FLAG_TYPE * new_flags = g_memory.Get(new_size);

    for (int i = 0; i < size && i < new_size; i++) {
      new_data[i] = data[i];
      new_flags[i] = flags[i];
    }

    if (data) delete [] data;
    if (flags) g_memory.Free(flags, max_size);

    data = new_data;
    flags = new_flags;
    max_size = new_size;
  }

  // Fill in the un-filled-in bits...
  for (int i = size; i < new_size; i++) {
    // data[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
    data[i] = cInstruction::GetInstDefault();
    flags[i] = 0;
  }

  size = new_size;
}

void cCodeArray::Insert(int line_num, const cInstruction & new_code)
{
  // If we're at out maximum size, we need to grow a bit...
  if (size == max_size) {
    max_size++;

    cInstruction * new_data = new cInstruction[max_size];
    CA_FLAG_TYPE * new_flags = g_memory.Get(max_size);

    int i;
    for (i = 0; i < line_num; i++) {
      new_data[i] = data[i];
      new_flags[i] = flags[i];
    }
    for (i = line_num; i < size; i++) {
      new_data[i + 1] = data[i];
      new_flags[i + 1] = flags[i];
    }

    if (data) delete [] data;
    if (flags) g_memory.Free(flags, size);

    data = new_data;
    flags = new_flags;
  }

  // Otherwise, just shuffle down a bit.
  else {
    for (int i = size - 1; i >= line_num; i--) {
      data[i + 1] = data[i];
      flags[i + 1] = flags[i];
    }
  }

  // Setup the newline and increase the size.
  data[line_num] = new_code;
  flags[line_num] = 0;

  size++;
}

void cCodeArray::Remove(int line_num)
{
#ifdef DEBUG
  if (size <= 1) {
    g_debug.Error("Trying to remove a line from a code arry of size %d!\n",
		  size);
  }
#endif

  for (int i = line_num + 1; i < size; i++) {
    data[i - 1] = data[i];
    flags[i - 1] = flags[i];
  }

  size--;
}

// Resize the arrays, and leave memeory uninitializes! (private method)
void cCodeArray::ResetSize(int new_size, int new_max)
{
#ifdef DEBUG
  if (new_size < 0) g_debug.Error("Reseting code array size to %d", new_size);
  if (new_max != 0 && new_max < new_size) {
    g_debug.Error("Reseting code array; size=%d, max_size=%d",
		  new_size, new_max);
  }
#endif

  // If we are clearing it totally, do so.
  if (!new_size) {
    if (data) delete [] data;
    if (flags) g_memory.Free(flags, max_size);
    data = NULL;
    flags = NULL;
    size = 0;
    max_size = 0;
    return;
  }

  // If a new max is _not_ being set, see if one needs to be.
  if (new_max == 0 &&
      (max_size < new_size) || (max_size > new_size * 2)) {
    new_max = new_size;
  }

  // If the arrays need to be rebuilt, do so.
  if (new_max != 0 && new_max != max_size) {
    cInstruction * new_data = new cInstruction[new_max];
    CA_FLAG_TYPE * new_flags = g_memory.Get(new_max);

    if (data) delete [] data;
    if (flags) g_memory.Free(flags, max_size);

    data = new_data;
    flags = new_flags;

    max_size = new_max;
  }

  // Reset the array size.
  size = new_size;
}

void cCodeArray::Reset(const int new_size, const int new_max)
{
  ResetSize(new_size, new_max);

  // Initialze the array
  if( cConfig::GetAllocMethod() == ALLOC_METHOD_RANDOM ){
    // Randomize the initial contents of the new array.
    Randomize();
  }else{
    // Assume default instuction initialization
    Clear();
  }
}

void cCodeArray::Reset(const cCodeArray & in_code, const int offset,
		       int new_size)
{
#ifdef DEBUG
  if (offset < 0) g_debug.Warning("Giving a neg offset to codearray reset");
  //if (cutoff < offset) g_debug.Warning("cutoff < offset in codearray reset");
#endif

  if (new_size == 0) new_size = in_code.GetSize() - offset;

  ResetSize(new_size);

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
    data[i] = cInstruction::GetInstDefault();
    flags[i] = 0;
  }
}

void cCodeArray::Randomize()
{
  int i;
  for (i = 0; i < size; i++) {
    data[i].SetOp(g_random.GetUInt(cConfig::GetNumInstructions()));
    flags[i] = 0;
  }
}

// Recombine two other code arrays into this one.  Align them so that
// code2 starts at line "offset" of code1.  Copy code1 until line
// "cross_point" and continue by copying from code2.  Note: This does _not_
// resize the code-array.

void cCodeArray::Copy(const cCodeArray & code1, const cCodeArray & code2,
		      int offset, int cross_point)
{
  int i;

#ifdef DEBUG
  if (cross_point > code1.GetSize()) {
    g_debug.Warning("cross_point > code1.size");
    cross_point = code1.GetSize();
  }
  if (cross_point < offset) {
    g_debug.Warning("cross_point < offset");
    offset = cross_point - 1;
  }
#endif

  for (i = 0; i < cross_point && i < size; i++) {
    data[i] = code1.data[i];
  }
  for (i = cross_point; i < code2.GetSize() + offset && i < size; i++) {
    data[i] = code2.data[i - offset];
  }
}



void cCodeArray::CrossoverInto(const cCodeArray & c1, int start1, int end1,
			       const cCodeArray & c2, int start2, int end2){
  int sz1 = (start1 <= end1) ? end1-start1+1 : c1.GetSize()-start1+end1+1;
  int sz2 = (start2 <= end2) ? end2-start2+1 : c2.GetSize()-start2+end2+1;
  Resize(c1.GetSize() - sz1 + sz2);

  int i = ( start1 <= end1 ) ? 0 : end1+1;
  int j = 0;

  for( ; i < start1; ++i, ++j ){
    Set(j, c1.Get(i));
    SetFlags(j, c1.GetFlags(i));
  }

  int e2 = (start2 > end2) ? end2+c2.GetSize() : end2;
  for( i=start2; i<=e2; ++i, ++j ){
    Set(j, c2.Get(i%c2.GetSize()));
    SetFlags(j, c2.GetFlags(i%c2.GetSize()));
    SetFlag(j, INST_FLAG_CROSS_MUT);
  }

  if( start1 <= end1 ){
    for( i=end1+1; i<c1.GetSize(); ++i, ++j ){
      Set(j, c1.Get(i));
      SetFlags(j, c1.GetFlags(i));
    }
  }
}



void cCodeArray::operator=(const cCodeArray & other_code)
{
  // If we need to resize, do so...

  ResetSize(other_code.size);

  // Now that both code arrays are the same size, copy the other one over.

  for (int i = 0; i < size; i++) {
    data[i] = other_code.data[i];
    flags[i] = other_code.flags[i];
  }
}


void cCodeArray::CopyData(const cCodeArray & other_code)
{
  // If we need to resize, do so...

  ResetSize(other_code.size);

  // Now that both code arrays are the same size, copy the other one over.

  for (int i = 0; i < size; i++) {
    data[i] = other_code.data[i];
    flags[i] = 0;
  }
}



void cCodeArray::Inject(cCodeArray * other_code, int inject_line)
{
#ifdef DEBUG
  if (inject_line > size) {
    g_debug.Error("UD %d.%d: Injecting at %d in creature of size %d!",
	      cStats::GetUpdate(), cStats::GetSubUpdate(), inject_line, size);
  }
#endif

  if (!other_code->GetSize()) return;

  // Setup the new code arrays.

  int new_size = size + other_code->GetSize();

  // If we need to re-build the code array, do so.
  if (new_size > max_size) {
    cInstruction * new_data = new cInstruction[new_size];
    CA_FLAG_TYPE * new_flags = g_memory.Get(new_size);

    // Copy the first section of code in.

    int i;
    for (i = 0; i < inject_line; i++) {
      new_data[i] = data[i];
      new_flags[i] = flags[i];
    }

    // And copy the last section of the original code...

    for (i = inject_line; i < size; i++) {
      new_data[i + other_code->GetSize()]  = data[i];
      new_flags[i + other_code->GetSize()] = flags[i];
    }

    // And re-assign all of the new arrays.
    if (data) delete [] data;
    if (flags) g_memory.Free(flags, max_size);
    data = new_data;
    flags = new_flags;

    max_size = new_size;
  }

  // Otherwise just shift down the portion after the injected area...
  else {
    for (int i = size - 1; i >= inject_line; i--) {
      data[i + other_code->GetSize()]  = data[i];
      flags[i + other_code->GetSize()] = flags[i];
    }
  }

  // Copy the injected porition...

  for (int i = 0; i < other_code->GetSize(); i++) {
    data[i + inject_line]  = other_code->data[i];
    flags[i + inject_line] = 0;  // Clear the flags upon injection...
  }

  // And finally, re-adjust the size.

  size = new_size;
}


int cCodeArray::OK() const
{
  int result = TRUE;

  if (size < 0 || size > max_size || max_size > MAX_CREATURE_SIZE) {
    g_debug.Error("cCodeArray: size=%d, max_size=%d, MAX_SIZE=%d, MIN_SIZE=%d",
		  size, max_size, MAX_CREATURE_SIZE, MIN_CREATURE_SIZE);
    result = FALSE;
  }

  for (int i = 0; i < size; i++) {
    if (!data[i].OK()) {
      g_debug.Warning("cCodeArray => data[%d] => Ok Failed!");
      result = FALSE;
    }
  }

  return result;
}

int cCodeArray::FindOverlap(const cCodeArray & in_code, int offset) const
{
  int size1 = GetSize();
  int size2 = in_code.GetSize();

  if (offset > 0) size1 -= offset;
  else size2 += offset;

  return (size1 < size2) ? size1 : size2;
}

int cCodeArray::FindBestOffset(const cCodeArray & in_code) const
{
  int i;
  int cur_distance, min_distance, best_distance;
  int best_offset = 0;
  int neg_OK = TRUE, pos_OK = TRUE;

  int size1 = GetSize();
  int size2 = in_code.GetSize();

  // Initialize to a 0 offset, then radiate out checking the rest.

  best_distance = cur_distance = FindHammingDistance(in_code);
  for (i = 1; i < size1 || i < size2; i++) {
    // Check -i offset...

    if (neg_OK) {
      min_distance = size1 + size2 - 2 * FindOverlap(in_code, -i);
      if (min_distance >= best_distance) {
	neg_OK = FALSE;
      } else {
	cur_distance = FindHammingDistance(in_code, -i);
	if (cur_distance < best_distance) {
	  best_distance = cur_distance;
	  best_offset = -i;
	}
      }
    }

    // Check +i offset...

    if (pos_OK) {
      min_distance = size1 + size2 - 2 * FindOverlap(in_code, i);
      if (min_distance > best_distance) {
	pos_OK = FALSE;
      } else {
	cur_distance = FindHammingDistance(in_code, i);
	if (cur_distance < best_distance) {
	  best_distance = cur_distance;
	  best_offset = i;
	}
      }
    }
  }

  return best_offset;
}

int cCodeArray::FindLevensteinDistance(const cCodeArray & in_code) const
{
  const int size1 = GetSize();
  const int size2 = in_code.GetSize();

  if (!size1) return size2;
  if (!size2) return size1;

  int * cur_row  = new int[size1];  // The row we are calculating
  int * prev_row = new int[size1];  // The last row we calculater
  int * temp_row = NULL;            // For swapping only...

  // Initialize the previous row to record the differece from nothing.
  for (int i = 0; i < size1; i++)  prev_row[i] = i + 1;

  // Loop through each subsequent character in the test code
  for (int i = 0; i < size2; i++) {
    // Initialize the first entry in cur_row.
    if (Get(0) == in_code.Get(i)) {
      cur_row[0] = i;
    } else {
      cur_row[0] = (i < prev_row[0]) ? (i+1) : (prev_row[0] + 1);
    }

    // Move down the cur_row and fill it out.
    for (int j = 1; j < size1; j++) {
      // If the values are equal, keep the value in the upper left.
      if (Get(j) == in_code.Get(i)) {
	cur_row[j] = prev_row[j-1];
      }

      // Otherwise, set the current position the the minimal of the three
      // numbers above (insertion), to the left (deletion), or upper left
      // (mutation) in the chart, plus one.
      else {
	cur_row[j] =
	  (prev_row[j] < prev_row[j-1]) ? prev_row[j] : prev_row[j-1];
	if (cur_row[j-1] < cur_row[j]) cur_row[j] = cur_row[j-1];
	cur_row[j]++;
      }
    }

    // Swap cur_row and prev_row. (we only really need to move the cur row
    // over to prev, but this saves us from having to keep re-allocating
    // new rows.  We recycle!
    temp_row = cur_row;
    cur_row = prev_row;
    prev_row = temp_row;
  }

  // Now that we are done, return the bottom-right corner of the chart.

  int value = prev_row[size1 - 1];

  delete [] cur_row;
  delete [] prev_row;

  return value;
}

int cCodeArray::FindSlidingDistance(const cCodeArray & in_code) const
{
  return FindHammingDistance(in_code, FindBestOffset(in_code));
}

int cCodeArray::FindHammingDistance(const cCodeArray & in_code, int offset) const
{
  int hamming_distance = 0;
  int start1 = (offset < 0) ? 0 : offset;
  int start2 = (offset > 0) ? 0 : -offset;
  int overlap;
  int i;

  // Find the ammount of overlap at this offset.

  overlap = FindOverlap(in_code, offset);

  // Automatically add anything protruding past the overlap to the
  // hamming distance.

  hamming_distance = GetSize() + in_code.GetSize() - 2 * overlap;

  // Cycle through the overlap adding all differences to the distance.

  for (i = 0; i < overlap; i++) {
    if (data[start1 + i] != in_code.data[start2 + i]) {
      hamming_distance++;
    }
  }

  return hamming_distance;
}


int cCodeArray::FindInstruction(const cInstruction & inst, int start_index){
  assert(start_index < GetSize());
  for(int i=0; i<GetSize(); ++i ){
    if( Get(i) == inst ){
      return i;
    }
  }
  return -1;
}

cString cCodeArray::AsString() const
{
  cString out_string(size);
  for (int i = 0; i < size; i++) {
    out_string[i] = data[i].GetSymbol();
  }

  return out_string;
}
