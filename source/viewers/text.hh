//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_TEXT_HH
#define VIEW_TEXT_HH

#ifdef VIEW_CURSES
#define VIEW_TEXT
#endif

#ifdef VIEW_NCURSES
#define VIEW_TEXT
#endif

#ifdef VIEW_ANSI
#define VIEW_TEXT
#endif

#ifdef VIEW_TEXT

#include "curses.hh"
#include "ncurses.hh"
#include "ansi.hh"

#define MODE_BLANK   0
#define MODE_MAP     1
#define MODE_STATS   2
#define MODE_HIST    3
#define MODE_OPTIONS 4
#define MODE_STEP    5

#define MAP_BASIC      0
#define MAP_SPECIES    1
#define MAP_BREED_TRUE 2
#define MAP_PARASITE   3
#define MAP_POINT_MUT  4
#define NUM_MAPS       5

// Other map modes currently inactive...
#define MAP_COMBO      6
#define MAP_INJECT     7
#define MAP_RESOURCE   8
#define MAP_AGE        9

#define HIST_GENOTYPE  0
#define HIST_SPECIES   1
#define NUM_HIST       2

// Unimplemented histogram modes...
#define HIST_TASKS     3
#define HIST_SIZE      4

#define NUM_SYMBOLS 12
#define SYMBOL_THRESHOLD 10

#define PAUSE_OFF               0
#define PAUSE_ON                1
#define PAUSE_ADVANCE_STEP      2
#define PAUSE_ADVANCE_UPDATE    3
#define PAUSE_ADVANCE_GESTATION 4

#define MEMORY_X    32
#define MEMORY_Y     1
#define STACK_X     47
#define STACK_Y      1
#define REG_X       61
#define REG_Y       10
#define INPUT_X     32
#define INPUT_Y     14
#define TASK_X      47
#define TASK_Y      14
#define MINI_MAP_X  61
#define MINI_MAP_Y   1

class cView {
protected:
  cPopulation * population;
  cGenebank * genebank;
  cInstLib * alt_inst_lib;

  // Status Variables
  int view_mode;
  int map_mode;
  int hist_mode;
  char pause_level;
  int memory_offset; // Offset to view the memory at while in Zoom mode.
  int parasite_zoom; // If TRUE, then view original parasite code, not host

  // Window information...
  cTextWindow * bar_window;
  cTextWindow * main_window;

  // Window managing functions...
  virtual void DrawBarWindow();
  virtual void DrawMapWindow();
  virtual void DrawStatsWindow();
  virtual void DrawHistWindow();
  virtual void DrawOptionsWindow();
  virtual void DrawStepWindow();
  void Redraw();
  void ClearAll();

  void UpdateBarWindow();
  virtual void UpdateMapWindow();
  virtual void UpdateStatsWindow();
  virtual void UpdateHistWindow();
  virtual void UpdateOptionsWindow();
  virtual void UpdateStepWindow();

  void ClearBarWindow();
  void ClearMainWindow();

  virtual void UpdateSymbols();

  void UpdateWindows();
  void TogglePause();
  virtual void ExtractCreature();

  // Input functions
  virtual void DoInputs();
  virtual void MapInput(int in_char);
  virtual void HistInput(int in_char);
  void StatsInput(int in_char);
  virtual void OptionsInput(int in_char);
  virtual void StepInput(int in_char);

  // Screen helpers
  void PrintMerit(int in_y, int in_x, cMerit in_merit);
  void PrintFitness(int in_y, int in_x, double in_fitness);
  int Confirm(const cString & message);
  void Notify(const cString & message);
  inline void SetMainColor(char color);

  // Functions for specific modes...
  virtual void EngageStepMode();
  virtual void DisEngageStepMode();
  
  // Map symbol information
  cGenotype * genotype_chart[NUM_SYMBOLS];
  cSpecies * species_chart[NUM_SYMBOLS];
  char symbol_chart[NUM_SYMBOLS];

  // Map functions...
  inline int InGenChart(cGenotype * in_gen);
  inline int InSpeciesChart(cSpecies * in_species);
  inline void AddGenChart(cGenotype * in_gen);
  inline void AddSpeciesChart(cSpecies * in_species);

  // Map navigation
  virtual void NavigateMapWindow();


  // Hist screen functions..
  inline void PrintGenotype(cGenotype * in_gen, int in_pos, int max_num);
  inline void PrintSpecies(cSpecies * in_species, int in_pos, int max_num);
public:
  cView();
  virtual ~cView();

  void NewUpdate();
  void NotifyUpdate();
  void NotifyError(const cString & in_string);
  void NotifyWarning(const cString & in_string);
  void NotifyComment(const cString & in_string);
  inline void Pause() { pause_level = PAUSE_ON; }

  inline void SetViewMode(int in_view_mode) { view_mode = in_view_mode; }
};

class cGAView : public cView {
private:
  // Window managing functions...
  void DrawBarWindow();
  void DrawMapWindow();
  void DrawStatsWindow();
  void DrawHistWindow();
  void DrawOptionsWindow();
  void DrawStepWindow();

  void UpdateMapWindow();
  void UpdateStatsWindow();
  void UpdateHistWindow();
  void UpdateOptionsWindow();
  void UpdateStepWindow();
  void NavigateMapWindow();

  void MapInput(int in_char);
  void OptionsInput(int in_char);
public:
  cGAView(cGAPopulation * in_population);
};

class cAAView : public cView {
protected:
  cAutoAdaptivePopulation * aa_pop;
  cBaseCPU * active_cpu;

  // Window managing functions...
  virtual void DrawBarWindow();
  virtual void DrawMapWindow();
  void DrawStatsWindow();
  void DrawHistWindow();
  virtual void DrawOptionsWindow();
  void DrawStepWindow();

  virtual void UpdateMapWindow();
  void UpdateStatsWindow();
  void UpdateHistWindow();
  virtual void UpdateOptionsWindow();
  virtual void UpdateStepWindow();

  virtual void DoInputs();
  virtual void MapInput(int in_char);
  void HistInput(int in_char);
  void OptionsInput(int in_char);
  virtual void StepInput(int in_char);

  void EngageStepMode();
  void DisEngageStepMode();
  virtual void NavigateMapWindow();
  void ExtractCreature();
public:
  cAAView();
};

class cTierraView : public cAAView {
private:
  // Window managing functions...
  void DrawBarWindow();
  void DrawMapWindow();
  void DrawOptionsWindow();

  void UpdateMapWindow();
  void UpdateOptionsWindow();
  void UpdateStepWindow();

  void MapInput(int in_char);
  void NavigateMapWindow();
public:
  cTierraView(cTierraPopulation * in_population);
};

class cAvidaView : public cAAView {
private:
  cAvidaPopulation * av_pop;
  int corner_id;
  int mini_center_id;
  int map_x_size;
  int map_y_size;

  // Window managing functions...
  void DrawBarWindow();
  void DrawMapWindow();
  void DrawMapWindow(char *szMoreInfo);
  void DrawOptionsWindow();

  void UpdateMapWindow();
  void UpdateMapWindow(char *szMoreInfo);
  void UpdateOptionsWindow();
  void UpdateStepWindow();

  void MapInput(int in_char);
  void StepInput(int in_char);

  void DrawMiniMap();
  void NavigateMapWindow();
  void CenterActiveCPU();
  void CenterXCoord();
  void CenterYCoord();
  void PlaceMapCursor();
public:
  cAvidaView(cAvidaPopulation * in_population);
};

class cPlaybackView : public cView {
private:
  cPlaybackPopulation * population;
  cPlaybackGenebank * pb_genebank;
  int corner_id;
  int map_x_size;
  int map_y_size;

  // Window managing functions...
  void DrawBarWindow();
  void DrawMapWindow();
  void UpdateMapWindow();

  void UpdateSymbols();
public:
  cPlaybackView(cPlaybackPopulation * in_population);
};

//////////////////////////
//  Inline Functions...
//////////////////////////

inline void cView::SetMainColor(char color) {
  switch (color) {
  case '.':
  case '-':
    main_window->SetColor(COLOR_WHITE);
    break;
  case '*':
  case '+':
  case '1':
    main_window->SetBoldColor(COLOR_WHITE);
    break;
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    main_window->SetBoldColor((int) color - 'A' + 1);
    break;
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
    main_window->SetColor((int) color - 'G' + 1);
    break;
  case ' ':
    main_window->SetColor(COLOR_OFF);
    break;
  case '0':
  default:
    main_window->SetBoldColor(COLOR_OFF);
    break;
  }
}

inline int cView::InGenChart(cGenotype * in_gen)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (genotype_chart[i] == in_gen) return TRUE;
  }
  return FALSE;
}

inline int cView::InSpeciesChart(cSpecies * in_species)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (species_chart[i] == in_species) return TRUE;
  }
  return FALSE;
}

inline void cView::AddGenChart(cGenotype * in_gen)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (genotype_chart[i] == NULL) {
      genotype_chart[i] = in_gen;
      in_gen->SetSymbol(symbol_chart[i]);
      break;
    }
  }
}

inline void cView::AddSpeciesChart(cSpecies * in_species)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (species_chart[i] == NULL) {
      species_chart[i] = in_species;
      in_species->SetSymbol(symbol_chart[i]);
      break;
    }
  }
}

inline void cView::PrintGenotype(cGenotype * in_gen, int in_pos, int max_num)
{
  main_window->SetBoldColor(COLOR_CYAN);
  //  if (in_gen->GetGestationTime())
    PrintFitness(in_pos, 0, in_gen->GetFitness());
  //  else main_window->Print(in_pos, 0, "    0.0");
  
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(in_pos, 8, "%s: ", in_gen->GetName()());
  
  int max_stars = main_window->Width() - 28;
  int star_size = (max_num / max_stars);
  if (max_num % max_stars) star_size++;

  int cur_num = in_gen->GetNumCPUs();
  int cur_stars = cur_num / star_size;
  if (cur_num % star_size) cur_stars++;

  // Set the color for this bar.
  SetMainColor(in_gen->GetSymbol());

  // Draw the bar.
  int i;
  for (i = 0; i < cur_stars; i++) {
    main_window->Print(in_gen->GetSpeciesSymbol());
  }

  // Draw the spaces following the bar.
  while (i++ < max_stars) main_window->Print(' ');

  // Display the true length of the bar (highlighted)
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(in_pos, main_window->Width() - 8, " %5d", cur_num);

  // Reset the color to normal
  main_window->SetColor(COLOR_WHITE);
}


inline void cView::PrintSpecies(cSpecies * in_species, int in_pos, int max_num)
{
  //  main_window->SetBoldColor(COLOR_CYAN);
  //  if (in_gen->GetGestationTime())
  //    PrintFitness(in_pos, 0, in_gen->GetFitness());
  //  else main_window->Print(in_pos, 0, "    0.0");
  
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(in_pos, 0, "        sp-%06d: ", in_species->GetID());
  
  int max_stars = main_window->Width() - 28;
  int star_size = (max_num / max_stars);
  if (max_num % max_stars) star_size++;

  int cur_num = in_species->GetNumCreatures();
  int cur_stars = cur_num / star_size;
  if (cur_num % star_size) cur_stars++;

  // Set the color for this bar.
  SetMainColor(in_species->GetSymbol());

  // Draw the bar.
  int i;
  for (i = 0; i < cur_stars; i++) {
    main_window->Print(in_species->GetSymbol());
  }

  // Draw the spaces following the bar.
  while (i++ < max_stars) main_window->Print(' ');

  // Display the true length of the bar (highlighted)
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(in_pos, main_window->Width() - 8, " %5d", cur_num);

  // Reset the color to normal
  main_window->SetColor(COLOR_WHITE);
}


class cMenuWindow {
private:
  cTextWindow * window;
  cString * option_list;
  int num_options;

  // menu status variables
  int active_id;
  int max_width;
  int lines;
  int cols;

  // Private helper functions
  void DrawOption(int option_id);
  void MoveActiveID(int new_id);
public:
  cMenuWindow(int menu_size);
  ~cMenuWindow();

  void AddOption(int option_id, char * in_option);
  int Activate();

  inline void SetActive(int in_id) { active_id = in_id; }
};

#endif

#endif
