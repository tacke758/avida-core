//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "view.hh"

#ifdef VIEW_PRIMITIVE

void EndProg(int ignore)
{
  //  signal(SIGINT, SIG_IGN);          // Ignore all future interupts.
  printf ("Exit Code: %d\n", ignore);
  exit(0);
}

#endif
