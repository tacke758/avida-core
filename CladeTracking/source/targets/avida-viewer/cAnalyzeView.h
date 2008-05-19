//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef cAnalyzeView_h
#define cAnalyzeView_h

#ifndef cViewInfo_h
#include "cViewInfo.h"
#endif

#ifndef cView_Base_h
#include "cView_Base.h"
#endif

class cScreen;
class cTextWindow;
class cBarScreen;
class cAnalyzeScreen;
class cWorld;

class cAnalyzeView : public cView_Base {
private:
  cViewInfo info;

  // Window information...
  static cTextWindow * base_window;
  static cBarScreen * bar_screen;
  cAnalyzeScreen * analyze_screen;

  // Input function
  void DoInputs();

public:
  cAnalyzeView(cWorld* world);
  ~cAnalyzeView();

  void Setup(const cString & in_name);
  void SetViewMode(int in_mode) { (void) in_mode; }

  bool ProcessKeypress(int keypress) { return false; }

  void NewUpdate() { ; }
  void NotifyUpdate();
  void NotifyError(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyComment(const cString & in_string);
  void NotifyOutput(const cString & in_string);
  void Pause() { ; }
  void DoBreakpoint() { ; }

  int Confirm(const cString & message);
  void Notify(const cString & message);

  int GetStepOrganism() { return -1; }
  void SetStepOrganism(int in_id) { (void) in_id; }

  void Refresh();

  // Methods called by sub-windows.
  void Redraw();
};


#ifdef ENABLE_UNIT_TESTS
namespace nAnalyzeView {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif
