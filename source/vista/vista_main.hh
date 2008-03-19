//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef VISTA_HH
#define VISTA_HH

#include <iostream.h>
#include <fstream.h>

#include "../tools/string.hh"

#include "../main/avida.hh"
#include "../main/stats.hh"

#include "legend.hh"
#include "map_mode_enum.hh"

class cVistaUI;
class cMapWindowUI;
class cGrid;

class cVistaMain;
typedef int (cVistaMain::*tGetMapValueMethod)(int);


class cVistaMain {
private:
  // Event Handler ID's in avida
  int viewer_eh_id;
  int main_eh_id;
  int population_eh_id;

  cVistaUI * ui;
  cMapWindowUI * map_ui;

  bool stats_window_open;
  ifstream stats_in;

  int update;

  cRandom random;
  
  // Map Stuff
  tGetMapValueMethod map_value_method;
  cLegendData * threshold_legend;
  cLegendData * birth_legend;
  cLegendData * genotype_legend;
  cLegendData * genotype_last_legend;

public:
  cVistaMain();
  ~cVistaMain();

  void ShowMainWindow();

  void DoUpdate(double time = 0.025);

  bool GetPauseState();
  void UpdatePauseState();
  void TogglePauseState();

  void Step();

  void Close();
  void ExitEvent();

  void ShowDialog(const cString & title, const cString & mesg);

  // Viewer States
  bool RedrawOnBirth();

  // Update
  void UpdateMainWindow();
  void UpdateStatsWindow();
  void UpdateMapWindow();
  void UpdateMapWindow(int p_cell, int d_cell); // for Update on Births

  // Particular Map Drawing Routines
  void CreateThresholdMap();
  void CreateBirthMap(); 
  void CreateGenotypeMap(); 

  void UpdateGenotypeLegend(); 
  
  int GetThresholdMapValue(int i);
  int GetBirthMapValue(int i);
  int GetGenotypeMapValue(int i);

  // Window Handling
  void OpenMapWindow();
  void CloseMapWindow();
  // Setup for windows (on and after creating)
  void SetupMap(int mode = MAP_MODE_THRESHOLD);
};


extern cVistaMain * vista_main;


#endif // #ifndef VISTA_HH

