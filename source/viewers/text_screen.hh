//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_TEXT_SCREEN_HH
#define VIEW_TEXT_SCREEN_HH

#include "../cpu/cpu.hh"
#include "../main/species.hh"
#include "../main/population.hh"
#include "../defs.hh"

#include "../cpu/cpu.ii"

#include "curses.hh"
#include "ncurses.hh"
#include "ansi.hh"

#ifdef VIEW_TEXT

#define NUM_SYMBOLS 12
#define SYMBOL_THRESHOLD 10

#define PAUSE_OFF               0
#define PAUSE_ON                1
#define PAUSE_ADVANCE_STEP      2
#define PAUSE_ADVANCE_UPDATE    3
#define PAUSE_ADVANCE_GESTATION 4

class cViewInfo {
private:
  cPopulation & population;
  cBaseCPU * active_cpu;

  int pause_level;
  int thread_lock;

  // Instruction Libraries.
  cInstLib const * saved_inst_lib;
  cInstLib alt_inst_lib;

  // Symbol information
  cGenotype * genotype_chart[NUM_SYMBOLS];
  cSpecies * species_chart[NUM_SYMBOLS];
  char symbol_chart[NUM_SYMBOLS];

  inline int InGenChart(cGenotype * in_gen);
  inline int InSpeciesChart(cSpecies * in_species);
  inline void AddGenChart(cGenotype * in_gen);
  inline void AddSpeciesChart(cSpecies * in_species);
public:
  cViewInfo(cPopulation & in_population);
  ~cViewInfo() { ; }

  void UpdateSymbols();

  void EngageStepMode();
  void DisEngageStepMode();

  cGenebank & GetGenebank() { return population.GetGenebank(); }
  cPopulation & GetPopulation() { return population; }

  int GetNumSymbols() { return NUM_SYMBOLS; }
  cGenotype * GetGenotype(int index) { return genotype_chart[index]; }
  cSpecies * GetSpecies(int index) { return species_chart[index]; }

  cBaseCPU * GetActiveCPU() { return active_cpu; }
  cGenotype * GetActiveGenotype()
    { return (active_cpu) ?
	active_cpu->GetActiveGenotype() : (cGenotype *) NULL; }
  cSpecies * GetActiveSpecies()
    { return GetActiveGenotype() ?
	GetActiveGenotype()->GetSpecies() : (cSpecies *) NULL; }
  cEnvironment * GetActiveEnvironment()
    { return active_cpu ?
	active_cpu->GetEnvironment() : (cEnvironment *) NULL;}
  cString GetActiveName()
    { return GetActiveGenotype() ?
	GetActiveGenotype()->GetName() : cString(""); }
  int GetActiveID()
    { return GetActiveEnvironment() ? GetActiveEnvironment()->GetID() : -1; }
  int GetActiveGenotypeID()
    { return GetActiveGenotype() ? GetActiveGenotype()->GetID() : -1; }
  int GetActiveSpeciesID()
    { return GetActiveSpecies() ? GetActiveSpecies()->GetID() : -1; }

  void SetActiveCPU(cBaseCPU * in_cpu) { active_cpu = in_cpu; }

  int GetPauseLevel() { return pause_level; }
  int GetThreadLock() { return thread_lock; }

  void SetPauseLevel(int in_level) { pause_level = in_level; }
  void SetThreadLock(int in_lock) { thread_lock = in_lock; }
};

class cScreen : public cTextWindow {
protected:
  cViewInfo & info;

  inline void SetSymbolColor(char color);
  inline void PrintMerit(int in_y, int in_x, cMerit in_merit);
  inline void PrintFitness(int in_y, int in_x, double in_fitness);
public:
  cScreen(int y_size, int x_size, int y_start, int x_start,
	  cViewInfo & in_info) :
    cTextWindow(y_size, x_size, y_start, x_start), info(in_info) { ; }
  virtual ~cScreen() { ; }

  virtual void Draw() = 0;
  virtual void Update() = 0;
  virtual void AdvanceUpdate() { ; }
  virtual void DoInput(int in_char) = 0;
  virtual void DoMouseClick(int x, int y) { (void) x; (void) y; }
  virtual void Exit() { ; }
};



///////////////
//  cViewInfo
///////////////

inline int cViewInfo::InGenChart(cGenotype * in_gen)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (genotype_chart[i] == in_gen) return TRUE;
  }
  return FALSE;
}

inline int cViewInfo::InSpeciesChart(cSpecies * in_species)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (species_chart[i] == in_species) return TRUE;
  }
  return FALSE;
}

inline void cViewInfo::AddGenChart(cGenotype * in_gen)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (genotype_chart[i] == NULL) {
      genotype_chart[i] = in_gen;
      in_gen->SetSymbol(symbol_chart[i]);
      break;
    }
  }
}

inline void cViewInfo::AddSpeciesChart(cSpecies * in_species)
{
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    if (species_chart[i] == NULL) {
      species_chart[i] = in_species;
      in_species->SetSymbol(symbol_chart[i]);
      break;
    }
  }
}


/////////////
//  cScreen
/////////////

inline void cScreen::SetSymbolColor(char color)
{
  switch (color) {
  case '.':
  case '-':
    SetColor(COLOR_WHITE);
    break;
  case '*':
  case '+':
  case '1':
    SetBoldColor(COLOR_WHITE);
    break;
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    SetBoldColor((int) color - 'A' + 1);
    break;
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
    SetColor((int) color - 'G' + 1);
    break;
  case ' ':
    SetColor(COLOR_OFF);
    break;
  case '0':
  default:
    SetBoldColor(COLOR_OFF);
    break;
  }
}

inline void cScreen::PrintMerit(int in_y, int in_x, cMerit in_merit)
{
  // if we can print the merit normally, do so.
  //if (in_merit.GetDouble() < 1000000) {
    //Print(in_y, in_x, "%7d", in_merit.GetUInt());
  //}

  // otherwise use scientific notation. (or somesuch)
  //else {
    Print(in_y, in_x, "%7.1e", in_merit.GetDouble());
  //}
}

inline void cScreen::PrintFitness(int in_y, int in_x, double in_fitness)
{
  // If we can print the fitness, do so!
  if (in_fitness <= 0.0) {
    Print(in_y, in_x, " 0.0000");
  }
  else if (in_fitness < 10)
    Print(in_y, in_x, "%7.4f", in_fitness);
  //  else if (in_fitness < 100)
  //    Print(in_y, in_x, "%7.3f", in_fitness);
  else if (in_fitness < 1000)
    Print(in_y, in_x, "%7.2f", in_fitness);
  //  else if (in_fitness < 10000)
  //    Print(in_y, in_x, "%7.1f", in_fitness);
  else if (in_fitness < 100000)
    Print(in_y, in_x, "%7.0f", in_fitness);

  // Otherwise use scientific notations.
  else
    Print(in_y, in_x, "%7.1e", in_fitness);
}

#endif

#endif
