//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "bar_screen.hh"

#ifdef VIEW_TEXT

///////////////////
// The Bar Screen
///////////////////

void cBarScreen::Draw()
{
  SetBoldColor(COLOR_WHITE);

  Box();
  VLine(19);
  VLine(Width() - 14);

  int offset = 7 + (prog_name.GetSize() + 1) / 2;
  Print(1, Width() - offset, "%s", prog_name());

  Print(1, 3, "Update:");
  Print(1, 22, "[M]ap  [S]tats  [O]ptions  [Z]oom  [Q]uit");

  SetBoldColor(COLOR_CYAN);
  Print(1, 23, 'M');
  Print(1, 30, 'S');
  Print(1, 39, 'O');
  Print(1, 50, 'Z');
  Print(1, 58, 'Q');

  Refresh();
}

void cBarScreen::Update()
{
  SetBoldColor(COLOR_WHITE);
  Print(1, 11, "%d", cStats::GetUpdate());
  SetColor(COLOR_WHITE);

  Refresh();
}

#endif
