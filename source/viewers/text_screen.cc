//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "text_screen.hh"

#ifdef VIEW_TEXT

cViewInfo::cViewInfo(cPopulation & in_population) : population(in_population)
{
  active_cpu = NULL;
  pause_level = PAUSE_OFF;
  saved_inst_lib = NULL;
  thread_lock = -1;
  
  // Handle genotype & species managing...
  
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    genotype_chart[i] = NULL;
    species_chart[i] = NULL;
    symbol_chart[i] = (char) (i + 'A');
  }
}

void cViewInfo::UpdateSymbols()
{
  // First, clean up the genotype_chart & species_chart.

  int i, pos;
  for (i = 0; i < NUM_SYMBOLS; i++) {
    if (genotype_chart[i]) {
      pos = GetGenebank().FindPos(*(genotype_chart[i]));
      if (pos < 0) genotype_chart[i] = NULL;
      if (pos >= NUM_SYMBOLS) {
	if (genotype_chart[i]->GetThreshold())
	  genotype_chart[i]->SetSymbol('+');
	else genotype_chart[i]->SetSymbol('.');
	genotype_chart[i] = NULL;
      }
    }
    if (species_chart[i]) {
      pos = GetGenebank().FindPos(*(species_chart[i]));
      if (pos < 0) species_chart[i] = NULL;
      if (pos >= NUM_SYMBOLS) {
	species_chart[i]->SetSymbol('+');
	species_chart[i] = NULL;
      }
    }
  }
  
  // Now, fill in any missing spaces...
  
  cGenotype * temp_gen = GetGenebank().GetBestGenotype();
  cSpecies * temp_species = GetGenebank().GetFirstSpecies();
  for (i = 0; i < SYMBOL_THRESHOLD; i++) {
    if (temp_gen) {
      if (!InGenChart(temp_gen)) AddGenChart(temp_gen);
      temp_gen = temp_gen->GetNext();
    }
    if (temp_species) {
      if (!InSpeciesChart(temp_species)) AddSpeciesChart(temp_species);
      temp_species = temp_species->GetNext();
    }
  }
}


void cViewInfo::EngageStepMode()
{
  if (pause_level == PAUSE_ON && active_cpu) {
    pause_level = PAUSE_ADVANCE_STEP;

    // Save the old instruction library and build a new one for the creature.
    saved_inst_lib = &(active_cpu->GetInstLib());
    alt_inst_lib = active_cpu->GetInstLib();
    alt_inst_lib.ActivateAltFunctions();
    active_cpu->SetInstLib(&alt_inst_lib);
  }
}

void cViewInfo::DisEngageStepMode()
{
  if (saved_inst_lib != NULL) {
    active_cpu->SetInstLib((cInstLib *) saved_inst_lib);
    saved_inst_lib = NULL;
  }
}


#endif
