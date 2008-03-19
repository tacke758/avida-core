//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VISTA_VIEW_HH
#define VISTA_VIEW_HH

#include <stdlib.h>
#include "../defs.hh"
#include "../tools/string.hh"
#include "../viewers/baseview.hh"

// Declaring so we can store a pointer


class cVistaView : public cBaseView {
private:
  int event_handler_id;

public:
  cVistaView();
  ~cVistaView();

  // Interface
  void DoUpdate();

  void NotifyComment(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyError(const cString & in_string);

  void NotifyUpdate();
  void NotifyBirth(int p_cell, int d_cell);
  void NotifyPause();
  void NotifyExit();

  void NotifyBreakpoint();

  bool GetEvents();

  // Misc
  void SetEventHandlerID(int in){ event_handler_id = in; }
};

#endif

