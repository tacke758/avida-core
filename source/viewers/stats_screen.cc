//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "stats_screen.hh"

#ifdef VIEW_TEXT

void cStatsScreen::Draw()
{
  SetBoldColor(COLOR_WHITE);

  // Print(0,  0, "--- Soup Status ---");
  Print(1,  0, "Tot Births.:");
  Print(2,  0, "Breed True.:");
  Print(3,  0, "Parasites..:");
  Print(4,  0, "Energy.....:");
  Print(5,  0, "Max Fitness:");
  Print(6,  0, "Max Merit..:");

  Print(1, 23, "-- Dominant Genotype --");
  Print(2, 23, "Name........:");
  Print(3, 23, "ID..........:");
  Print(4, 23, "Species ID..:");  
  Print(5, 23, "Age.........:");  

  Print(8, 11, "Current    Total  Ave Age  Entropy");
  Print(9,  0, "Creatures:");
  Print(10, 0, "Genotypes:");
  Print(11, 0, "Threshold:");
  Print(12, 0, "Species..:");

  Print(1, 61, "Dominant  Average");
  Print(2, 50, "Fitness..:");
  Print(3, 50, "Merit....:");
  Print(4, 50, "Gestation:");
  Print(5, 50, "Size.....:");
  Print(6, 50, "Copy Size:");
  Print(7, 50, "Exec Size:");
  Print(8, 50, "Abundance:");
  Print(9, 50, "Births...:");
  Print(10, 50, "BirthRate:");
  Print(11, 50, "Num Thrds:");
  Print(12, 50, "Thrd Dist:");


  int task_num = task_offset;
  for (int col_id = 3; task_num < cConfig::GetNumTasks(); col_id += 20) {
    if (col_id + 16 > Width()) break;
    for (int row_id = 15;
	 row_id < 15 + task_rows && task_num < cConfig::GetNumTasks();
	 row_id++) {
      Print(row_id, col_id, ".........:");
      Print(row_id, col_id, "%s",
	    cConfig::GetDefaultTaskLib()->GetTaskName(task_num)());
      task_num++;
    }
  }

  SetColor(COLOR_WHITE);

  Box(0, 14, Width(), task_rows + 2);

  if (task_num < cConfig::GetNumTasks() || task_offset != 0) {
    SetBoldColor(COLOR_WHITE);
    Print(15 + task_rows, Width() - 20, " [<-] More [->] ");
    SetBoldColor(COLOR_CYAN);
    Print(15 + task_rows, Width() - 18, "<-");
    Print(15 + task_rows, Width() - 8, "->");
  }

  Update();
}

void cStatsScreen::Update()
{
  cGenotype * best_gen = population.GetGenebank().GetBestGenotype();

  SetBoldColor(COLOR_CYAN);

  Print(1, 13, "%7d",   cStats::GetNumDeaths());
  Print(2, 13, "%7d",   cStats::GetBreedTrue());
  Print(3, 13, "%7d",   cStats::GetNumParasites());
  Print(4, 13, "%7.2f", cStats::GetEnergy());  
  PrintFitness(5, 13, cStats::GetMaxFitness());  
  // Print(5, 13, "%.1e",  cStats::GetMaxFitness());  
  Print(6, 13, "%.1e",  cStats::GetMaxMerit());  

  Print(9,  13, "%5d", cStats::GetNumCreatures());
  Print(10, 13, "%5d", cStats::GetNumGenotypes());
  Print(11, 13, "%5d", cStats::GetNumThreshold());
  Print(12, 13, "%5d", cStats::GetNumSpecies());
 
  Print(2, 37, "%s",  best_gen->GetName()());
  Print(3, 37, "%9d", best_gen->GetID());
  Print(4, 37, "%9d", (best_gen->GetSpecies()) ?
	(best_gen->GetSpecies()->GetID()) : -1);
  Print(5, 37, "%9d", best_gen->GetAge());

  Print(9,  20, "%.1e", (double) cStats::GetTotCreatures());
  Print(10, 20, "%.1e", (double) cStats::GetTotGenotypes());
  Print(11, 20, "%.1e", (double) cStats::GetTotThreshold());
  Print(12, 20, "%.1e", (double) cStats::GetTotSpecies());

  Print(9,  30, "%6.1f", cStats::GetAveCreatureAge());
  Print(10, 30, "%6.1f", cStats::GetAveGenotypeAge());
  Print(11, 30, "%6.1f", cStats::GetAveThresholdAge());
  Print(12, 30, "%6.1f", cStats::GetAveSpeciesAge());

  Print(9,  39, "%6.2f",
		     log((double) cStats::GetNumCreatures()));
  Print(10, 39, "%6.2f", cStats::GetEntropy());
  Print(12, 39, "%6.2f", cStats::GetSpeciesEntropy());

  PrintFitness(2, 62, best_gen->GetFitness());
  // Print(2, 62, "%.1e", best_gen->GetFitness());
  //PrintMerit(3, 62, best_gen->GetMerit());
  Print(3, 62, "%6.2f", best_gen->GetMerit());
  Print(4, 63, "%6.2f", best_gen->GetGestationTime());
  Print(5, 63, "%6d", best_gen->GetLength());
  Print(6, 63, "%6.2f", best_gen->GetCopiedSize());
  Print(7, 63, "%6.2f", best_gen->GetExecutedSize());
  Print(8, 63, "%6d", best_gen->GetNumCPUs());
  Print(9, 63, "%6d", best_gen->GetThisBirths());
  if (cStats::GetAveMerit() == 0) {
    Print(10, 66, "0.0");
  } else {
    Print(10, 63, "%6.3f", ((double) cConfig::GetAveTimeslice()) * best_gen->GetFitness() / cStats::GetAveMerit());
  }
  //Put Dominant creature threads info @GKH

  PrintFitness(2, 71, cStats::GetAveFitness());
  Print(3, 71, "%.1e", cStats::GetAveMerit());
  //PrintMerit(3, 72, cStats::GetAveMerit());
  Print(4, 72, "%6.1f", cStats::GetAveGestation());
  Print(5, 72, "%6.1f", cStats::GetAveSize());
  Print(6, 72, "%6.1f", cStats::GetAveCopySize());
  Print(7, 72, "%6.1f", cStats::GetAveExeSize());
  Print(8, 71, "%7.2f",
		     ((double) cStats::GetNumCreatures()) /
		     ((double) cStats::GetNumGenotypes()));
  // @CAO this next line should be get num births, which doesn't exist!
  Print(9, 70, "%8.3f",
		     ((double) cStats::GetNumDeaths()) /
		     ((double) cStats::GetNumGenotypes()));
  if (cStats::GetAveMerit() != 0) {
    Print(10, 71, "%7.3f", ((double) cConfig::GetAveTimeslice()) * cStats::GetAveFitness() / cStats::GetAveMerit());
  } else {
    Print(10, 71, "%7.3f", 0.0);
  }

  Print(11, 72, "%6.3f", cStats::GetAveNumThreads());
  Print(12, 72, "%6.3f", cStats::GetAveThreadDist());

  // This section needs to be changed to work with new task_lib @TCC
  int task_num = task_offset;
  for (int col_id = 14; task_num < cConfig::GetNumTasks(); col_id += 20) {
    if (col_id + 5 > Width()) break;
    for (int row_id = 15;
	 row_id < 15 + task_rows && task_num < cConfig::GetNumTasks();
	 row_id++) {
      Print(row_id, col_id, "%4d", cStats::GetTaskCount(task_num));
      task_num++;
    }
  }

  SetColor(COLOR_WHITE);

  Refresh();
}

void cStatsScreen::DoInput(int in_char)
{
  switch (in_char) {
  case '4':
  case KEY_LEFT:
    if (task_offset > 0) {
      task_offset -= 5;
      Draw();
    }
    break;
  case '6':
  case KEY_RIGHT:
    if (task_rows * task_cols + task_offset < cConfig::GetNumTasks()) {
      task_offset += 5;
      Draw();
    }
    break;
  default:
    g_debug.Warning("Unknown Command (#%d): [%c]", in_char, in_char);
    break;
  }
}

#endif
