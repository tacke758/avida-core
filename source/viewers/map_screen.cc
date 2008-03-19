//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "map_screen.hh"

#ifdef VIEW_TEXT

///////////////////
// The Map Screen
///////////////////

cMapScreen::cMapScreen(int _y_size, int _x_size, int _y_start,
      int _x_start, cViewInfo & in_info, cPopulation & in_pop) :
  cScreen(_y_size, _x_size, _y_start, _x_start, in_info),
  mode(MAP_BASIC),
  x_size(in_pop.GetWorldX()),
  y_size(in_pop.GetWorldY()),
  population(in_pop)
{
  info.SetActiveCPU( &(population.GetCPU(0)) );
  CenterActiveCPU();
}

cMapScreen::~cMapScreen()
{
}

void cMapScreen::Update()
{
  // Get working in multiple modes!!

  char * map = NULL;
  char * color_map = NULL;

  SetBoldColor(COLOR_WHITE);

  switch (mode) {
  case MAP_BASIC:
    if (HasColors()) color_map = population.GetBasicGrid();
    else map = population.GetBasicGrid();
    Print(Height() - 1, Width() - 20, " Genotype View ");
    break;
  case MAP_SPECIES:
    if (HasColors()) color_map = population.GetSpeciesGrid();
    else map = population.GetSpeciesGrid();
    Print(Height() - 1, Width() - 20, " Species View  ");
    break;
  case MAP_COMBO:
    color_map = population.GetBasicGrid();
    map = population.GetSpeciesGrid();
    Print(Height() - 1, Width() - 20, "  Combo View   ");
    break;
  case MAP_INJECT:
    map = population.GetModifiedGrid();
    Print(Height() - 1, Width() - 20, " Modified View ");
    break;
  case MAP_RESOURCE:
    map = population.GetResourceGrid();
    Print(Height() - 1, Width() - 20, " Resource View ");
    break;
  case MAP_AGE:
    map = population.GetAgeGrid();
    Print(Height() - 1, Width() - 20, "   Age View    ");
    break;
  case MAP_BREED_TRUE:
    if (HasColors()) color_map = population.GetBreedGrid();
    else map = population.GetBreedGrid();
    Print(Height() - 1, Width() - 20, "Breed True View");
    break;
  case MAP_PARASITE:
    if (HasColors()) color_map = population.GetParasiteGrid();
    else map = population.GetParasiteGrid();
    Print(Height() - 1, Width() - 20, " Parasite View ");
    break;
  case MAP_POINT_MUT:
    if (HasColors()) color_map = population.GetPointMutGrid();
    else map = population.GetPointMutGrid();
    Print(Height() - 1, Width() - 20, " PointMut View ");
    break;
  case MAP_THREAD:
    if (HasColors()) color_map = population.GetThreadGrid();
    else map = population.GetThreadGrid();
    Print(Height() - 1, Width() - 20, "  Thread View  ");
    break;
  case MAP_LINEAGE:
    if (HasColors()) color_map = population.GetLineageGrid();
    else map = population.GetLineageGrid();
    Print(Height() - 1, Width() - 20, " Lineage View  ");
    break;
  }


  // Draw the [<] and [>] around the map mode....
  Print(Height() - 1, Width() - 24, "[ ]");
  Print(Height() - 1, Width() -  4, "[ ]");
  SetBoldColor(COLOR_CYAN);
  Print(Height() - 1, Width() - 23, '<');
  Print(Height() - 1, Width() -  3, '>');
  SetColor(COLOR_WHITE);

  if (y_size >= Height() && Width() > 60) {
    // Map is too big for the display window (since last line is reserved for
    // the display mode text). Tell users how much is clipped.
    Print(Height() - 1, 0, "* Clipping last %i line(s) *",
		  y_size - Height() + 1);
  }

  int virtual_x = corner_id % x_size;
  int virtual_y = corner_id / x_size;

  for (int y = 0; y < Height() - 1 && y < y_size; y++) {
    Move(y, 0);
    int cur_y = (y + virtual_y) % y_size;
    for (int x = 0;
	 (AVIDA_MAP_X_SPACING * x) < Width() - 1 && x < x_size;
	 x++) {
      int cur_x = (x + virtual_x) % x_size;
      int index = cur_y * x_size + cur_x;
      if (color_map) {
	// If this spot has no color, skip over it!
	if (color_map[index] == ' ') {
	  Print("  ");
	  continue;
	}
	// Otherwise, set the proper color.
	SetSymbolColor(color_map[index]);
      }
      if (map) Print(map[index]);
      else Print(CHAR_BULLET);

      // Skip spaces before the next map symbol
      for (int i = 0; i < AVIDA_MAP_X_SPACING - 1; i++)
	Print(' ');
    }
  }
  SetColor(COLOR_WHITE);

  Refresh();
  if (map) g_memory.Free(map, population.GetSize());
  if (color_map) g_memory.Free(color_map, population.GetSize());
}


void cMapScreen::DoInput(int in_char)
{
  switch(in_char) {
  case '2':
  case KEY_DOWN:
    corner_id += x_size;
    corner_id %= population.GetSize();
    Update();
    break;
  case '8':
  case KEY_UP:
    corner_id -= x_size;
    if (corner_id < 0) corner_id += population.GetSize();
    Update();
    break;
  case '6':
  case KEY_RIGHT:
    corner_id++;
    if (corner_id == population.GetSize()) corner_id = 0;
    Update();
    break;
  case '4':
  case KEY_LEFT:
    corner_id--;
    if (corner_id < 0) corner_id += population.GetSize();
    Update();
    break;
  case '>':
  case '.':
    ++mode %= NUM_MAPS;
    Update();
    break;
  case '<':
  case ',':
    mode += NUM_MAPS;
    --mode %= NUM_MAPS;
    Update();
    break;
  }
}

void cMapScreen::CenterActiveCPU()
{
  assert(x_size > 0 && y_size > 0);

  // first find the map position of the active cpu
  int active_x = info.GetActiveID() % x_size;
  int active_y = info.GetActiveID() / x_size;
  
  // Now find the relative position of the corner to adjust for this
  int corner_x = active_x - Width() / (2 * AVIDA_MAP_X_SPACING);
  int corner_y = active_y - Height() / 2;

  // Make sure the corner co-ordinates are positive
  if (corner_x < 0) corner_x = (corner_x % x_size) + x_size;
  if (corner_y < 0) corner_y = (corner_y % y_size) + y_size;

  // Now, find the ID for these corner co-ordinates
  corner_id = corner_y * x_size + corner_x;
}

void cMapScreen::CenterXCoord()
{
  // first find the map position of the active cpu
  int active_x = info.GetActiveID() % x_size;
  
  // Now find the relative position of the corner to adjust for this
  int corner_x = active_x - Width() / (2 * AVIDA_MAP_X_SPACING);
  int corner_y = corner_id / x_size;
  
  // Make sure the corner co-ordinates are positive
  if (corner_x < 0) corner_x = (corner_x % x_size) + x_size;

  // finally find the ID for these corner co-ordinates
  corner_id = corner_y * x_size + corner_x;
}

void cMapScreen::CenterYCoord()
{
  // first find the map position of the active cpu
  int active_y = info.GetActiveID() / x_size;
  
  // Now find the relative position of the corner to adjust for this
  int corner_x = corner_id % x_size;
  int corner_y = active_y - Height() / 2;

  // Make sure the corner co-ordinates are positive
  if (corner_y < 0) corner_y = (corner_y % y_size) + y_size;

  // finally find the ID for these corner co-ordinates
  corner_id = corner_y * x_size + corner_x;
}

void cMapScreen::PlaceCursor()
{
  int x_offset = info.GetActiveID() - corner_id;
  x_offset %= x_size;
  if (x_offset < 0) x_offset += x_size;

  int y_offset = (info.GetActiveID() / x_size) - (corner_id / x_size);
  if (y_offset < 0) y_offset += y_size;

  cGenotype * cpu_gen = info.GetActiveGenotype();

  if (!cpu_gen) {
    Print(Height() - 1, 33,
	      "(%2d, %2d) - (empty)  ", info.GetActiveID() % x_size,
	      info.GetActiveID() / x_size);
  } else {
    Print(Height() - 1, 33, "(%2d, %2d) - %s",
		       info.GetActiveID() % x_size,
		       info.GetActiveID() / x_size,
		       cpu_gen->GetName()());
  }
  
  if (x_offset == 0 || x_offset == Width()/2 - 1) {
    CenterXCoord();
    Clear();
    Draw();
    PlaceCursor();
  }
  else if (y_offset == 0 || y_offset == Height() - 2) {
    CenterYCoord();
    Clear();
    Draw();
    PlaceCursor();
  }
  else {
    Move(y_offset, x_offset * 2);
    Refresh();
  }
}

void cMapScreen::Navigate()
{
  // Setup for choosing a cpu...

  CenterActiveCPU();
  Clear();
  Update();
  Print(Height() - 1, 0, "Choose a CPU and press ENTER");
  Refresh();

  PlaceCursor();

  cBaseCPU * old_cpu = info.GetActiveCPU();
  int temp_cpu_id;

  int finished = FALSE;
  int cur_char;
  int iXMove, iYMove;

#ifdef VIEW_ANSI
  MOUSE_INFO * pmiData; // For mouse input (see ANSI.HH)
  // Map dim's run from 0 to iXMax and 0 to iYMax; use this to regulate
  // cursor position

  int iXMax = min(x_size * AVIDA_MAP_X_SPACING, Width()) - 1;
  int iYMax = min(y_size, Height()-1) - 1;
      // extra -1 due to text at bottom

  FlushMouseQueue();        // Make sure queue is clear
  EnableMouseEvents(TRUE);  // and enable mouse event tracking
#endif

  while (!finished) {
    iXMove = iYMove = 0;	// nothing moves yet
    cur_char = GetInput();
    switch (cur_char) {
    case 'q':
    case 'Q':
      info.SetActiveCPU(old_cpu);
      finished = TRUE;
      break;
    case '2':
    case KEY_DOWN:
      iYMove = 1;
      break;
    case '8':
    case KEY_UP:
      iYMove = -1;
      break;
    case '4':
    case KEY_LEFT:
      iXMove = -1;
      break;
    case '6':
    case KEY_RIGHT:
      iXMove = 1;
      break;
    case ' ':
    case '\r':
    case '\n':
      finished = TRUE;
      break;

#ifdef VIEW_ANSI
      // Mouse processing code (only works for ANSI right now).  Other view
      //  methods will need a GetMouseInput() stub before this #IFDEF can
      // come out. #defines and MOUSE_INFO struct defined in ansi.hh
    case KEY_MOUSE:
      if (NULL != (pmiData = GetMouseInput()) && 
	  IsCoordInWindow (pmiData->X, pmiData->Y)) {
	// deal with MOUSE_DBLCLICK and MOUSE_CLICK input.  Single click
	// moves cursor.  Dbl click selects CPU and runs '\r' code. 

	// We already know the click occured within our window
	if (pmiData->iEvent & (MOUSE_DBLCLICK || MOUSE_CLICK)) {
	  // Reposition cursor on single or dbl click. Pin the cursor
	  // movement within the map region.
	  iXMove = min (iXMax, ScreenXToWindow(pmiData->X)) - 
	    CursorX();
	  iYMove = min (iYMax, ScreenYToWindow(pmiData->Y)) - 
	    CursorY();
	  iXMove /= AVIDA_MAP_X_SPACING; // Correct for spaces in horizontal
	}
	// On dbl click, we are done (act like Enter was hit)
	finished = pmiData->iEvent & MOUSE_DBLCLICK;
      }
      delete (pmiData);	// free up the storage now
      break;
#endif

    default:
      break;
    }	// End of SWITCH

    // Now deal with the cursor movement
    temp_cpu_id = info.GetActiveID();
    if (iXMove) {
      int i = temp_cpu_id % x_size; // column we are in
      temp_cpu_id -= i; // CPU id, less column
      i += iXMove;
      while (i >= x_size) i -= x_size;
      while (i < 0) i += x_size;
      temp_cpu_id += i; // CPU id readjusted to new column
    }
    if (iYMove) {
      temp_cpu_id += x_size * iYMove; 
      while (temp_cpu_id >= population.GetSize())
	temp_cpu_id -= population.GetSize();
      while (temp_cpu_id < 0) temp_cpu_id += population.GetSize();
    }
    if (iXMove || iYMove) {
      info.SetActiveCPU( &(population.GetCPU(temp_cpu_id)) );
      PlaceCursor();
    }
  } // End of WHILE
  
#ifdef VIEW_ANSI
  // Turn off mouse events, flush the queue and we are finished
  EnableMouseEvents (FALSE);
  FlushMouseQueue();
#endif

  CenterActiveCPU();	// Set map w/ active CPU in the center
  Clear();
}

#endif
