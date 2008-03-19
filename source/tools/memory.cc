#include "memory.hh"

cMemTrack::cMemTrack()
{
  int i;

  bytes_used = 0;
  blocks_used = 0;
  for (i = 0; i < TOTAL_TYPES; i++) {
    mem_used[i] = 0;
  }

  for (i = 0; i < MEM_HASH_SIZE; i++) {
    check_lists[i] = NULL;
  }

#ifdef DEBUG_MEM_CHECK
  fp_mem = fopen("memory.dat", "w");
#endif
}

cMemTrack::~cMemTrack()
{
  // Nothing to do here...
}

void cMemTrack::Print(int update)
{
#ifdef DEBUG_MEM_CHECK
  fprintf(fp_mem, "%3d: Geno Tmpl Bcpu Mcpu Tcpu LgSN Spec Code BLOCK GEN\n",
	  update);
  fprintf(fp_mem, "     %4d %4d %4d %4d %4d %4d %4d %4d %5d %4d\n",
	  mem_used[C_GENOTYPE], mem_used[C_LABEL], mem_used[C_BASE_CPU],
	  mem_used[C_MAIN_CPU], mem_used[C_TEST_CPU],
	  mem_used[C_LOG_SCHEDULE_NODE], mem_used[C_SPECIES],
	  mem_used[C_CODE_ARRAY], blocks_used, bytes_used);

/*
  for (int i = 0; i < TOTAL_TYPES; i++) {
    fprintf (fp_mem, "%d: %d used\n", i, mem_used[i]);
  }
*/

  fflush(fp_mem);
#else
  (void) update;
#endif
}

void cMemTrack::InsertCheckList(void * in_ptr)
{
  int list_num = GetRCS(in_ptr);

  if (!in_ptr) fprintf(fp_mem, "ERROR: Trying to insert a NULL pointer!\n");
  
  cMemElement * new_element = new cMemElement(in_ptr, check_lists[list_num]);
  check_lists[list_num] = new_element;
  blocks_used++;
}

void cMemTrack::RemoveCheckList(void * in_ptr)
{
  int list_num = GetRCS(in_ptr);

  cMemElement * cur_element = check_lists[list_num];
  cMemElement * prev_element = NULL;

  while (cur_element && cur_element->GetPointer() != in_ptr) {
    prev_element = cur_element;
    cur_element = cur_element->GetNext();
  }

  if (!cur_element) {
    fprintf(fp_mem, "ERROR: Trying to re-free ptr. [%p]\n", in_ptr);
    fflush(fp_mem);
    return;
  }

  if (prev_element) {
    prev_element->SetNext(cur_element->GetNext());
    delete cur_element;
  } else {
    check_lists[list_num] = cur_element->GetNext();
    delete cur_element;
  }

  blocks_used--;
}
