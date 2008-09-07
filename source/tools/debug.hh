#ifndef DEBUG_HH
#define DEBUG_HH

#include <stdio.h>
#include <stdarg.h>
#include "include.hh"

#define DEBUG_NONE     -1
#define DEBUG_ERROR     0
#define DEBUG_WARNING   1
#define DEBUG_COMMENT   2

class cFunctionStats {
private:
  char * name;
  int times_called;
  cFunctionStats * next;
  cFunctionStats * prev;

  inline void Reset() {
    name = NULL; times_called = 0; next = NULL; prev = NULL;
  }
public:
  inline cFunctionStats() { Reset(); }
  inline cFunctionStats(char * in_name) {
    Reset();
    name = new char[strlen(in_name) + 1];
    strcpy(name, in_name);
  }
  inline ~cFunctionStats() { delete [] name;}

  inline char * GetName() { return name; }
  inline int GetTimesCalled() { return times_called; }
  inline cFunctionStats * GetNext() { return next; }
  inline cFunctionStats * GetPrev() { return prev; }

  inline void SetNext(cFunctionStats * in_next) { next = in_next; }
  inline void SetPrev(cFunctionStats * in_prev) { prev = in_prev; }
  inline void InsertNext(cFunctionStats * in_next) {
    in_next->SetNext(next);
    in_next->SetPrev(this);
    next->SetPrev(in_next);
    next = in_next;
  }
  inline void InsertPrev(cFunctionStats * in_prev) {
    in_prev->SetNext(this);
    in_prev->SetPrev(prev);
    prev->SetNext(in_prev);
    prev = in_prev;
  }
  inline void SwapNext() {
    cFunctionStats * old_next = next;
    prev->SetNext(old_next);
    old_next->GetNext()->SetPrev(this);

    next = old_next->GetNext();
    old_next->SetPrev(prev);
    prev = old_next;
    old_next->SetNext(this);
  }
  inline void SwapPrev() {
    cFunctionStats * old_prev = prev;
    next->SetPrev(old_prev);
    old_prev->GetPrev()->SetNext(this);

    prev = old_prev->GetPrev();
    old_prev->SetNext(next);
    next = old_prev;
    old_prev->SetPrev(this);
  }

  inline int Compare(char * in_name) {
    return !strcmp(name, in_name);
  }

  void Print();
  void Print(FILE * fp);

  inline void Inc() {
    times_called++;

    // Reset position in the queue to keep frequency from most to least...
    while (prev->GetTimesCalled() > 0 &&
	   prev->GetTimesCalled() < times_called) {
      SwapPrev();
    }
  }
};

#define FS_HASH_SIZE 23

class cDebug {
private:
  FILE * fp_debug;
  int debug_level;

  // Should be made faster than list if needed
  cFunctionStats * function_lists[FS_HASH_SIZE];  // lists have fixed heads!
  int num_functions;

  int CalcHash(char * in_name);
  cFunctionStats * FindFS(char * in_name);
public:
  cDebug();
  ~cDebug();
  
  void Error(char * comment, ...);
  void Warning(char * comment, ...);
  void Comment(char * comment, ...);
	void cDebug::DebugOut (char *pszType, char *pszMessage);

  inline int GetLevel();

  inline void SetLevel(int new_level);
  inline void SetFS(char * comment);
  inline void PrintFS();
  inline void PrintFS(FILE * fp);
};

#ifdef DEBUG

inline int cDebug::GetLevel()
{
  return debug_level;
}

inline void cDebug::SetLevel(int new_level)
{
  debug_level = new_level;
}

inline void cDebug::SetFS(char * comment)
{
  cFunctionStats * cur_function = FindFS(comment);
  cur_function->Inc();
}

inline void cDebug::PrintFS()
{
  // Just print them all for now... later make it look nicer (sorted)...
  int i;
  for (i = 0; i < FS_HASH_SIZE; i++) {
    cFunctionStats * cur_function = function_lists[i]->GetNext();
    while (cur_function != function_lists[i]) {
      cur_function->Print();
      cur_function = cur_function->GetNext();
    }
  }
}

inline void cDebug::PrintFS(FILE * fp)
{
  //// Just print them all for now... later make it look nicer (sorted)...
  int i;
  for (i = 0; i < FS_HASH_SIZE; i++) {
    cFunctionStats * cur_function = function_lists[i]->GetNext();
    while (cur_function != function_lists[i]) {
      cur_function->Print(fp);
      cur_function = cur_function->GetNext();
    }
  }
}

#endif   // #ifdef DEBUG

#ifndef DEBUG

inline int cDebug::GetLevel() { return DEBUG_NONE; }
inline void cDebug::SetLevel(int /* new_level */) { ; }
inline void cDebug::SetFS(char * /* comment */) { ; }
inline void cDebug::PrintFS() { ; }
inline void cDebug::PrintFS(FILE * /* fp */) { ; }

#endif // #ifndef DEBUG

#endif
