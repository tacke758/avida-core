//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_MAP_SCREEN_HH
#define VIEW_MAP_SCREEN_HH

#include "text_screen.hh"

#ifdef VIEW_TEXT

#define MAP_BASIC      0
#define MAP_SPECIES    1
#define MAP_BREED_TRUE 2
#define MAP_PARASITE   3
#define MAP_POINT_MUT  4
#define MAP_THREAD     5
#define MAP_INJECT     6
#define NUM_MAPS       7

// Other map modes currently inactive...
#define MAP_COMBO      7
#define MAP_RESOURCE   8
#define MAP_AGE        9

#define	AVIDA_MAP_X_SPACING 2

class cMapScreen : public cScreen {
private:
  int mode;

  int x_size;
  int y_size;
  int corner_id;
  cPopulation & population;

  // Private Methods...
  void CenterActiveCPU();
  void CenterXCoord();
  void CenterYCoord();
  void PlaceCursor();
  
public:
  cMapScreen(int y_size, int x_size, int y_start, int x_start,
		  cViewInfo & in_info, cPopulation & in_pop);
  ~cMapScreen();

  // Virtual in base screen!
  void Draw() { ; }
  void Update();
  void DoInput(int in_char);

  // Virtual in map screen.
  void Navigate();
};


#endif
#endif
