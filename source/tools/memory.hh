//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include "../main/classes.hh"
#include "../defs.hh"

#include <stdio.h>

#define MEM_HASH_SIZE 2311
#define MAX_BYTES_ALLOCATED 1048576

/**
 * Utility class used by @ref cMemTrack.
 **/

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

// Inline includes
#include "memory.ii"
                    






