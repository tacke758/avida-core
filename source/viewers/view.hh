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
#include "../cpu/head.ii"
#include "../cpu/cpu.ii"

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
    cout<<"Update: "<<cStats::GetUpdate()<<"\t"
	<<"Generation: "<<cStats::GetAveGeneration()<<endl;
  }

  /*
  inline void NotifyComment(const cString & in_string) {
    fprintf(stderr,"%s\n", in_string());
  }

  inline void NotifyWarning(const cString & in_string) {
    fprintf(stderr,"Warning: %s\n", in_string());
  }

  inline void NotifyError(const cString & in_string) {
    fprintf(stderr,"ERROR: %s\n", in_string());
  }
  */  
  inline void SetViewMode(int in_view_mode) { view_mode = in_view_mode; }
};

#endif // VIEW_PRIMITIVE

#endif

