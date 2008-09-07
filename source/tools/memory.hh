#include "../main/classes.hh"
#include "../defs.hh"

#include <stdio.h>

#define MEM_HASH_SIZE 2311
#define MAX_BYTES_ALLOCATED 1048576

class cMemElement {
private:
  void * pointer;
  cMemElement * next;
public:
  inline cMemElement(void * in_pointer = NULL, cMemElement * in_next = NULL)
    { pointer = in_pointer; next = in_next; }
  inline ~cMemElement() { ; }

  inline void * GetPointer() { return pointer; }
  inline cMemElement * GetNext() { return next; }
  inline void SetNext(cMemElement * in_next) { next = in_next; }
};

class cMemTrack {
private:
  int bytes_used;
  int blocks_used;
  int mem_used[TOTAL_TYPES];
  cMemElement * check_lists[MEM_HASH_SIZE];
  FILE * fp_mem;
public:
  cMemTrack();
  ~cMemTrack();

  inline int GetRCS(void * in_ptr) { return ((int) in_ptr) % MEM_HASH_SIZE; }

  inline char * Get(int num_bytes);
  inline int * GetInts(int num_ints);
  inline void Free(char * pointer, int num_bytes);
  inline void Free(int * pointer, int num_bytes);

  inline void Add(int type);
  inline void Remove(int type);

  inline int GetBytesUsed() { return bytes_used; }
  inline int GetMemUsed(int in_type) { return mem_used[in_type]; }

  void Print(int update = -1);

  void InsertCheckList(void * in_ptr);
  void RemoveCheckList(void * in_ptr);
};

//////
// Inline methods...
//////

inline char * cMemTrack::Get(int num_bytes)
{
#ifdef DEBUG
  if (num_bytes > MAX_BYTES_ALLOCATED) {
    fprintf(fp_mem, "Allocating too many bytes!\n");
    fflush(fp_mem);
  }
  if (num_bytes < 0) {
    fprintf(fp_mem, "Allocating negative memory!\n");
    fflush(fp_mem);
  }
  bytes_used += num_bytes;
#endif

#ifdef DEBUG_MEM_CHECK
  char * ptr = new char[num_bytes];
  InsertCheckList(ptr);
  return ptr;
#else
  return new char[num_bytes];
#endif
}

inline int * cMemTrack::GetInts(int num_ints)
{
#ifdef DEBUG
  bytes_used += num_ints * 4;
  if (num_ints > MAX_BYTES_ALLOCATED / 4) {
    fprintf(fp_mem, "Allocating too many ints!\n");
    fflush(fp_mem);
  }
  if (num_ints < 0) {
    fprintf(fp_mem, "Allocating negative memory (ints)!\n");
    fflush(fp_mem);
  }
#endif

#ifdef DEBUG_MEM_CHECK
  int * ptr = new int[num_ints];
  InsertCheckList(ptr);
  return ptr;
#else
  return new int[num_ints];
#endif
}

inline void cMemTrack::Free(char * pointer, int num_bytes)
{
  bytes_used -= num_bytes;

#ifdef DEBUG_MEM_CHECK
  if (!pointer)
    fprintf(fp_mem, "Error trying to delete NULL pointer of %d bytes\n",
	    num_bytes);
  RemoveCheckList(pointer);
#endif

  delete [] pointer;
}

inline void cMemTrack::Free(int * pointer, int num_ints)
{
  bytes_used -= num_ints * 4;

#ifdef DEBUG_MEM_CHECK
  if (!pointer)
    fprintf(fp_mem, "Error trying to delete NULL pointer of %d ints\n",
	    num_ints);
  RemoveCheckList(pointer);
#endif

  delete [] pointer;
}

inline void cMemTrack::Add(int type)
{
  mem_used[type]++;
}

inline void cMemTrack::Remove(int type)
{
  mem_used[type]--;

#ifdef DEBUG
  if (mem_used[type] < 0) {
    printf("ERROR: Negative number of objects type %d.\n", type);
  }
#endif
}
