//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_TEXT_WINDOW_HH
#define VIEW_TEXT_WINDOW_HH

#include "../tools/debug.hh"
#include "../tools/tools.hh"

// class cTextWindow;

#define MAX_SUB_WINDOWS 4

class cCoords {
private:
  int x;
  int y;
public:
  cCoords(int _x, int _y) { x = _x, y = _y; }
  cCoords(const cCoords & _coords) { x = _coords.x, y = _coords.y; }
  ~cCoords() { ; }

  void Translate(int _x, int _y) { x += _x, y += _y; }

  void operator=(const cCoords & _coords) { x = _coords.x, y = _coords.y; }
  int operator==(const cCoords & _c) { return (x == _c.x && y == _c.y); }
  int operator!=(const cCoords & _c) { return !operator==(_c); }

  int GetX() { return x; }
  int GetY() { return y; }

  void Set(int _x, int _y) { x = _x; y = _y; }
};


class cBaseTextWindow {
protected:
  cBaseTextWindow * sub_windows[MAX_SUB_WINDOWS];
public:
  cBaseTextWindow() {
    for (int i = 0; i < MAX_SUB_WINDOWS; i++) sub_windows[i] = NULL;
  }
  virtual ~cBaseTextWindow() {
    for (int i = 0; i < MAX_SUB_WINDOWS; i++) {
      if (sub_windows[i]) delete sub_windows[i];
    }
  }


  // Dealing with sub-windows...
  virtual void AddSubWindow(cBaseTextWindow * in_win, int id) {
    if (sub_windows[id] != NULL)
      g_debug.Warning("Placing one sub-window over another.");
    sub_windows[id] = in_win;
  }
  virtual void RemoveSubWindow(int id) {
    if (sub_windows[id] == NULL)
      g_debug.Error("Trying to remove a non-existant sub-window.");
    sub_windows[id] = NULL;
  }
  virtual void CloseSubWindow(int id) {
    if (sub_windows[id] == NULL)
      g_debug.Error("Trying to close a non-existant sub-window.");
    delete sub_windows[id];
    sub_windows[id] = NULL;
  }
  virtual cBaseTextWindow * GetSubWindow(int id) { return sub_windows[id]; }
  

  // Interface...
  virtual void Construct(int y_size, int x_size,
			 int y_start=0, int x_start=0) = 0;

  // These function return the number of characters wide or high
  // (respectively) that the screen is.
  virtual int Width() = 0;
  virtual int Height() = 0;

  // Clear the screen and redraw all text on it.
  virtual void Redraw() {
    RedrawMain();
    for (int i = 0; i < MAX_SUB_WINDOWS; i++) {
      if (sub_windows[i]) sub_windows[i]->Redraw();
    }
  }
  virtual void RedrawMain() = 0;

  // Move the active position of the cursor.
  virtual void Move(int new_y, int new_x) = 0;

  // Print all of the changes made onto the screen.
  virtual void Refresh() {
    RefreshMain();
    for (int i = 0; i < MAX_SUB_WINDOWS; i++) {
      if (sub_windows[i]) sub_windows[i]->Refresh();
    }
  }
  virtual void RefreshMain() = 0;

  // These functions clear sections of the screen.
  virtual void Clear() {
    ClearMain();
    for (int i = 0; i < MAX_SUB_WINDOWS; i++) {
      if (sub_windows[i]) sub_windows[i]->Clear();
    }
  }
  virtual void ClearMain() = 0;
  virtual void ClearToBot() = 0;
  virtual void ClearToEOL() = 0;
  
  // Various 'graphic' functions.  Box() draws a box-frame around the window.
  // With arguments, it draws a box starting at x, y, with size w by h.
  // VLine and HLine draw lines across the screen ending in the appropriate
  // facing T's (hence, they are used to cut boxes in half.  With two
  // coords and a length, they only draw the line from the specified start,
  // to the specified distance.
  virtual void Box() = 0;
  virtual void Box(int x, int y, int w, int h) = 0;
  virtual void VLine(int in_x) = 0;
  virtual void HLine(int in_y) = 0;
  virtual void HLine(int in_y, int start_x, int length) = 0;

  // The following functions print characters onto the screen.  If they
  // begin with an y, x then they first move to those coords.

  virtual void Print(chtype ch) = 0;
  virtual void Print(int in_y, int in_x, chtype ch) = 0;
  virtual void Print(const char * fmt, ...) = 0;
  virtual void Print(int in_y, int in_x, const char * fmt, ...) = 0;

  // These functions check or set the screen colors (BG is always black)
  virtual int HasColors() = 0;
  virtual void SetColor(int color) = 0;
  virtual void SetBoldColor(int color) = 0;
};

#endif