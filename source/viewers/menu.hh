//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_MENU_HH
#define VIEW_MENU_HH

#include "curses.hh"
#include "ncurses.hh"
#include "ansi.hh"

#ifdef VIEW_TEXT

class cMenuWindow {
private:
  cTextWindow * window;
  cString title;
  cString * option_list;
  char * key_list;
  int num_options;

  // menu status variables
  int active_id;
  int max_width;
  int lines;
  int cols;

  // Private helper functions
  void DrawOption(int option_id, int is_active=FALSE);
  void MoveActiveID(int new_id);
  int FindHotkeyPos(const cString & in_entry);
public:
  cMenuWindow(int menu_size);
  ~cMenuWindow();

  void SetTitle(const char * in_title) { title = in_title; }
  void AddOption(int option_id, const char * in_option);
  int Activate();

  inline void SetActive(int in_id) { active_id = in_id; }
};

#endif

#endif
