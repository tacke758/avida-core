#include <stdio.h>
#include "debug.hh"

#ifdef DEBUG




cDebug::cDebug()
{
  fp_debug = fopen("debug.dat", "w");
  debug_level = DEBUG_NONE;

  int i;
  for (i = 0; i < FS_HASH_SIZE; i++) {
    function_lists[i] = new cFunctionStats;
    function_lists[i]->SetNext(function_lists[i]);
    function_lists[i]->SetPrev(function_lists[i]);
  }
  num_functions = 0;
}

cDebug::~cDebug()
{
  fclose(fp_debug);

  cFunctionStats * prev_FS, * last_FS;
  int i;
  for (i = 0; i < FS_HASH_SIZE; i++) {
    last_FS = function_lists[i]->GetPrev();
    while (function_lists[i] != last_FS) {
      prev_FS = function_lists[i];
      function_lists[i] = function_lists[i]->GetNext();
      delete prev_FS;
    }
    delete last_FS;
  }
}

void cDebug::Error(char * fmt, ...)
{
  if (debug_level >= DEBUG_ERROR) {
    va_list argp;
    char buf[1024];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    va_end(argp);

    fprintf(stderr, "ERROR: %s\n", buf);
    DebugOut ("ERROR", buf);
  }
}

void cDebug::Warning(char * fmt, ...)
{
  if (debug_level >= DEBUG_WARNING) {
    va_list argp;
    char buf[1024];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    va_end(argp);

    fprintf(stderr, "WARNING: %s\n", buf);
	DebugOut ("WARNING", buf);
  }
}

void cDebug::Comment(char * fmt, ...)
{
  if (debug_level >= DEBUG_COMMENT) {
    va_list argp;
    char buf[1024];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    va_end(argp);

	DebugOut ("Comment", buf);
  }
}

void cDebug::DebugOut (char *pszType, char *pszMessage)
{

    fprintf(fp_debug, "%s: %s\n", pszType, pszMessage);
    fflush(fp_debug);

#ifdef PLATFORM_WINDOWS
	{
		// Send Debug string to debugger too!
		char	buf[2048];
		wsprintf (buf, "%s: %s\n", pszType, pszMessage);
		OutputDebugString (buf);
	}
#endif
}

#endif

#ifndef DEBUG
cDebug::cDebug() { ; }
cDebug::~cDebug() { ; }
void cDebug::Error(char *   /* comment */, ...) { ; }
void cDebug::Warning(char * /* comment */, ...) { ; }
void cDebug::Comment(char * /* comment */, ...) { ; }
#endif


void cFunctionStats::Print(){
  printf("%d : [%s]\n", times_called, name);
}
void cFunctionStats::Print(FILE * fp){
  fprintf(fp, "%d : [%s]\n", times_called, name);
}

cFunctionStats * cDebug::FindFS(char * in_name)
{
  int hash = CalcHash(in_name);
  cFunctionStats * cur_FS = function_lists[hash]->GetNext();

  while (cur_FS != function_lists[hash]) {
    if (cur_FS->Compare(in_name)) break;
    cur_FS = cur_FS->GetNext();
  }
  if (cur_FS == function_lists[hash]) {
    cur_FS = new cFunctionStats(in_name);
    function_lists[hash]->InsertPrev(cur_FS);
  }

  return cur_FS;
}

int cDebug::CalcHash(char * in_name)
{
  int i, len = strlen(in_name);
  int total = 0;
  for (i = 0; i < len; i++) {
    total += in_name[i];
  }

  return total % FS_HASH_SIZE;
}
