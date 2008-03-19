//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          // 
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_HIST_SCREEN_HH
#define VIEW_HIST_SCREEN_HH

#include "text_screen.hh"

#ifdef VIEW_TEXT

#define HIST_GENOTYPE  0
#define HIST_SPECIES   1
#define NUM_HIST       2

// Unimplemented histogram modes...
#define HIST_TASKS     3
#define HIST_SIZE      4

class cHistScreen : public cScreen {
protected:
  cPopulation & population;
  int mode;

  inline void PrintGenotype(cGenotype * in_gen, int in_pos, int max_num);
  inline void PrintSpecies(cSpecies * in_species, int in_pos, int max_num);
public:
  cHistScreen(int y_size, int x_size, int y_start, int x_start,
	      cViewInfo & in_info, cPopulation & in_pop)
    : cScreen(y_size, x_size, y_start, x_start, in_info), population(in_pop)
  { mode = HIST_GENOTYPE; }
  virtual ~cHistScreen() { ; }

  // Virtual in base screen...
  void Draw();
  void Update();
  void DoInput(int in_char);
};

inline void cHistScreen::PrintGenotype(cGenotype * in_gen, int in_pos,
				       int max_num)
{
  SetBoldColor(COLOR_CYAN);
  PrintFitness(in_pos, 0, in_gen->GetFitness());
  
  SetBoldColor(COLOR_WHITE);
  Print(in_pos, 8, "%s: ", in_gen->GetName()());
  
  int max_stars = Width() - 28;
  int star_size = (max_num / max_stars);
  if (max_num % max_stars) star_size++;

  int cur_num = in_gen->GetNumCPUs();
  int cur_stars = cur_num / star_size;
  if (cur_num % star_size) cur_stars++;

  // Set the color for this bar.
  SetSymbolColor(in_gen->GetSymbol());

  // Draw the bar.
  int i;
  for (i = 0; i < cur_stars; i++) {
    Print(in_gen->GetSpeciesSymbol());
  }

  // Draw the spaces following the bar.
  while (i++ < max_stars) Print(' ');

  // Display the true length of the bar (highlighted)
  SetBoldColor(COLOR_WHITE);
  Print(in_pos, Width() - 8, " %5d", cur_num);

  // Reset the color to normal
  SetColor(COLOR_WHITE);
}

inline void cHistScreen::PrintSpecies(cSpecies * in_species, int in_pos, int max_num)
{
  //  SetBoldColor(COLOR_CYAN);
  //  if (in_gen->GetGestationTime())
  //    PrintFitness(in_pos, 0, in_gen->GetFitness());
  //  else Print(in_pos, 0, "    0.0");
  
  SetBoldColor(COLOR_WHITE);
  Print(in_pos, 0, "        sp-%06d: ", in_species->GetID());
  
  int max_stars = Width() - 28;
  int star_size = (max_num / max_stars);
  if (max_num % max_stars) star_size++;

  int cur_num = in_species->GetNumCreatures();
  int cur_stars = cur_num / star_size;
  if (cur_num % star_size) cur_stars++;

  // Set the color for this bar.
  SetSymbolColor(in_species->GetSymbol());

  // Draw the bar.
  int i;
  for (i = 0; i < cur_stars; i++) {
    Print(in_species->GetSymbol());
  }

  // Draw the spaces following the bar.
  while (i++ < max_stars) Print(' ');

  // Display the true length of the bar (highlighted)
  SetBoldColor(COLOR_WHITE);
  Print(in_pos, Width() - 8, " %5d", cur_num);

  // Reset the color to normal
  SetColor(COLOR_WHITE);
}

#endif
#endif
