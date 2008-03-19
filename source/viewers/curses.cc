//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "view.hh"

#ifdef VIEW_CURSES

////////////////////
//  cTextWindow
////////////////////

cTextWindow::cTextWindow()
{
  win_id = NULL;
}

cTextWindow::cTextWindow(int y_size, int x_size, int y_start, int x_start)
{
  win_id = newwin(y_size, x_size, y_start, x_start);
  keypad(win_id, 1);        // Allows the keypad to be used.
}

cTextWindow::~cTextWindow()
{
  werase(win_id);
  wrefresh(win_id);
  delwin(win_id);
}

void cTextWindow::Construct(int y_size, int x_size, int y_start, int x_start)
{
#ifdef DEBUG
  if (win_id != NULL) g_debug.Error("Trying to re- Construct() a window.");
#endif

  win_id = newwin(y_size, x_size, y_start, x_start);
  keypad(win_id, 1);        // Allows the keypad to be used.
}

void cTextWindow::Box()
{
  box(win_id, 0, 0);
  Print(0,            0,           '+');
  Print(0,            Width() - 1, '+');
  Print(Height() - 1, 0,           '+');
  Print(Height() - 1, Width() - 1, '+');
}

void cTextWindow::Box(int x, int y, int w, int h)
{
  int i;
  for (i = 1; i < h - 1; i++) {
    mvwaddch(win_id, i + y, x, '|');
    mvwaddch(win_id, i + y, x + w - 1, '|');
  }
  for (i = 1; i < w - 1; i++) {
    mvwaddch(win_id, y, i + x, '-');
    mvwaddch(win_id, y + h - 1, i + x, '-');
  }
  mvwaddch(win_id, y, x, '+');
  mvwaddch(win_id, y + h - 1, x, '+');
  mvwaddch(win_id, y, x + w - 1, '+');
  mvwaddch(win_id, y + h - 1, x + w - 1, '+');
}

void cTextWindow::VLine(int in_x)
{
  mvwaddch(win_id, 0, in_x, '+');
  mvwaddch(win_id, Height() - 1, in_x, '+');
  for (int i = 1; i < Height() - 1; i++) {
    mvwaddch(win_id, i, in_x, '|');
  }
}

void cTextWindow::HLine(int in_y)
{
  mvwaddch(win_id, in_y, 0, '+');
  mvwaddch(win_id, in_y, Width() - 1, '+');
  for (int i = 1; i < Width() - 1; i++) {
    mvwaddch(win_id, in_y, i, '-');
  }
}

void cTextWindow::HLine(int in_y, int start_x, int length)
{
  mvwaddch(win_id, in_y, start_x, '+');
  mvwaddch(win_id, in_y, start_x + length - 1, '+');
  for (int i = 1; i < length - 1; i++) {
    mvwaddch(win_id, in_y, start_x + i, '-');
  }
}


/////////////////////////
//  Other functions...
/////////////////////////

void StartProg()
{
  initscr();                // Set up the terminal for curses.
  cbreak();                 // Don't buffer input.
  noecho();                 // Don't echo keypresses to the screen.
  nonl();                   // No new line with CR (when echo is on)

  keypad(stdscr, 1);        // Allows the keypad to be used.
  nodelay(stdscr, 1);       // Don't wait for input if no key is pressed.
}

void EndProg(int ignore)
{
  signal(SIGINT, SIG_IGN);          // Ignore all future interupts.
  mvcur(0, COLS - 1, LINES -1, 0);  // Move curser to the lower left.
  endwin();                         // Restore terminal mode.

  printf ("Exit Code: %d\n", ignore);
  
  exit(0);
}

#endif
