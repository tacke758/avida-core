//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef EXTERNAL_HH
#define EXTERNAL_HH

#include <stdlib.h>
#include "../defs.hh"
#include "baseview.hh"
#include "../tools/string.hh"

// Declaring so we can store a pointer

class cExternalView : public cBaseView {
private:
  int event_handler_id;

public:
  cExternalView();
  ~cExternalView();

  // Interface
  inline void DoUpdate() {;} 

  void NotifyComment(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyError(const cString & in_string);

  void NotifyUpdate();
  inline void NotifyBirth(int p_cell, int d_cell){;}
  void NotifyPause();
  void NotifyBreakpoint(){ cerr<<"DoBreakpoint"<<endl; }
  void NotifyExit();

  bool GetEvents();

  void SetEventHandlerID(int in){ event_handler_id = in; }

};

#endif

