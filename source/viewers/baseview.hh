//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef BASEVIEW_HH
#define BASEVIEW_HH

#include "../defs.hh"
#include "../tools/string.hh"

class cBaseView {
protected:
public:
  cBaseView() {;}
  virtual ~cBaseView() {;}

  // Actual Interface -- All Call Backs to Viewer Declared Here!
  virtual void DoUpdate() = 0; 

  virtual void NotifyComment(const cString & in_string) = 0;
  virtual void NotifyWarning(const cString & in_string) = 0;
  virtual void NotifyError(const cString & in_string) = 0;

  virtual void NotifyUpdate() = 0;
  virtual void NotifyBirth(int p_cell, int d_cell) = 0;
  virtual void NotifyPause() = 0;
  virtual void NotifyBreakpoint() = 0;
  virtual void NotifyExit() = 0;

  virtual bool GetEvents() = 0; // adds new events to event_list
};

#endif // ifndef BASEVIEW

