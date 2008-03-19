//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_TEXT_HH
#define VIEW_TEXT_HH

#include "curses.hh"
#include "ncurses.hh"
#include "ansi.hh"

#ifdef VIEW_TEXT

#define MODE_BLANK   0
#define MODE_MAP     1
#define MODE_STATS   2
#define MODE_HIST    3
#define MODE_OPTIONS 4
#define MODE_ZOOM    5

#include "menu.hh"
#include "text_screen.hh"

#include "bar_screen.hh"
#include "map_screen.hh"
#include "stats_screen.hh"
#include "hist_screen.hh"
#include "options_screen.hh"
#include "zoom_screen.hh"

class cView {
private:
  cViewInfo info;

  // Window information...
  static cTextWindow * base_window;
  static cScreen * cur_screen;
  static cBarScreen * bar_screen;

  cMapScreen * map_screen;
  cStatsScreen * stats_screen;
  cHistScreen * hist_screen;
  cOptionsScreen * options_screen;
  cZoomScreen * zoom_screen;

  // Window managing functions...

  void TogglePause();
  void CloneSoup();
  void ExtractCreature();

  // Input function
  void DoInputs();

  // Screen helpers
  void ChangeCurScreen(cScreen * new_screen);
  void PrintMerit(int in_y, int in_x, cMerit in_merit);
  void PrintFitness(int in_y, int in_x, double in_fitness);

  // Map navigation
  void NavigateMapWindow() { map_screen->Navigate(); }
public:
  cView(cPopulation & in_population);
  virtual ~cView();

  void Setup(const cString & in_name);
  void SetViewMode(int in_mode);

  void NewUpdate();
  void NotifyUpdate();
  void NotifyError(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyComment(const cString & in_string);
  inline void Pause() { info.SetPauseLevel(PAUSE_ON); }
  void DoBreakpoint();

  static int Confirm(const cString & message);
  static void Notify(const cString & message);

  void Refresh(){  ChangeCurScreen(cur_screen); bar_screen->Redraw(); }

  // Methods called by sub-windows.
  static void Redraw();
};

#endif

#endif
