//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "hist_screen.hh"

#ifdef VIEW_TEXT

void cHistScreen::Draw()
{
  SetBoldColor(COLOR_WHITE);
  Print(1,  0, "Fitness Name");
  Print(1, 19, "Histogram: [ ]");
  Print(1, 53, "[ ]");
  SetBoldColor(COLOR_CYAN);
  Print(1, 31, '<');
  Print(1, 54, '>');

  Update();
}

void cHistScreen::Update()
{
  int max_num = 0;
  int i;
  
  switch(mode) {
  case HIST_GENOTYPE: 
    max_num = info.GetGenebank().GetBestGenotype()->GetNumCPUs();
    SetBoldColor(COLOR_WHITE);
    Print(1,  34, "Genotype Abundance");
    // Print out top NUM_SYMBOL genotypes in fixed order.
    for (i = 0; i < info.GetNumSymbols(); i++) {
      if (info.GetGenotype(i)) {
	PrintGenotype(info.GetGenotype(i), i + 2, max_num);
      }
      else {
	Move(i + 2, 0);
	ClearToEOL();
      }
    }
    break;
  case HIST_SPECIES:
    max_num = 0;
    for (i = 0; i < NUM_SYMBOLS; i++) {
      if (info.GetSpecies(i) && info.GetSpecies(i)->GetNumCreatures()
	  > max_num)
	max_num = info.GetSpecies(i)->GetNumCreatures();
    }

    SetBoldColor(COLOR_WHITE);
    Print(1,  34, "Species Abundance");

    // Print out top number of symbols species in fixed order.
    for (i = 0; i < info.GetNumSymbols(); i++) {
      if (info.GetSpecies(i)) {
	PrintSpecies(info.GetSpecies(i), i + 2, max_num);
      }
      else {
	Move(i + 2, 0);
	ClearToEOL();
      }
    }
    break;
  }

  ClearToBot();
  Refresh();
}

void cHistScreen::DoInput(int in_char)
{
  switch(in_char) {
  case '<':
  case ',':
    ++mode %= NUM_HIST;
    Update();
    break;
  case '>':
  case '.':
    mode += NUM_HIST;
    --mode %= NUM_HIST;
    Update();
    break;
  }
}

#endif
