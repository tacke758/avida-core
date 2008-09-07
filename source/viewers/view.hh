//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_HH
#define VIEW_HH

// This is a cludge to will be fixed when the viewer is abstracted @TCC --
#include "../main/code_array.hh"
#include "../main/genotype.hh"
#include "../cpu/cpu.hh"
#include "../main/inst_lib.hh"
#include "../main/species.hh"
#include "../main/genebank.hh"
#include "../main/slice.hh"
#include "../main/population.hh"
#include "../main/playback.hh"
#include "../cpu/head.ii"
#include "../cpu/cpu.ii"
// -- @TCC


#include <stdlib.h>
#include "../defs.hh"
#include "../main/stats.hh"

#include "text.hh"

#ifdef VIEW_PRIMITIVE

void EndProg(int ignore);

class cView {
private:
  int view_mode;
public:
  inline cView() { ; }
  inline ~cView() { ; }

  inline void NewUpdate() {
    printf("Update: %d\n", stats.GetUpdate());
  }
  inline void SetViewMode(int in_view_mode) { view_mode = in_view_mode; }
};

#endif // VIEW_PRIMITIVE

#endif

