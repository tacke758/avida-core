//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "view.hh"

#ifdef VIEW_TEXT

// put 1 space between columns
#define	AVIDA_MAP_X_SPACING 2

//////////////////
//  cView
//////////////////

cView::cView()
{
  // Setup variables

  view_mode = MODE_BLANK;
  map_mode = MAP_BASIC;
  hist_mode = HIST_GENOTYPE;
  pause_level = PAUSE_OFF;
  memory_offset = 0;
  parasite_zoom = FALSE;

  // Setup exit

  signal(SIGINT, EndProg);  // Run EndProg() if there is an interupt.

  // Setup text-interface

  StartProg();

  bar_window = new cTextWindow(3, 0, 0, 0);
  main_window = new cTextWindow(0, 0, 3, 0);

  // Handle genotype & species managing...

  for (int i = 0; i < NUM_SYMBOLS; i++) {
    genotype_chart[i] = NULL;
    species_chart[i] = NULL;
    symbol_chart[i] = (char) (i + 'A');
  }
}

cView::~cView()
{
  delete bar_window;
  delete main_window;

  EndProg(0);
}

void cView::DrawBarWindow()
{
  bar_window->Box();

  bar_window->SetBoldColor(COLOR_WHITE);
  bar_window->Print(1, 3, "Update:");
  bar_window->Print(1, 22, "[M]ap  [S]tats  [O]ptions  [Z]oom  [Q]uit");

  bar_window->SetBoldColor(COLOR_CYAN);
  bar_window->Print(1, 23, 'M');
  bar_window->Print(1, 30, 'S');
  bar_window->Print(1, 39, 'O');
  bar_window->Print(1, 50, 'Z');
  bar_window->Print(1, 58, 'Q');
  bar_window->SetColor(COLOR_WHITE);

  bar_window->VLine(19);
  bar_window->VLine(bar_window->Width() - 14);

  bar_window->Refresh();
}

void cView::DrawMapWindow()
{
  // VIRTUAL FUNCTION
}

void cView::DrawStatsWindow()
{
  // VIRTUAL FUNCTION
}

void cView::DrawHistWindow()
{
  // VIRTUAL FUNCTION
}

void cView::DrawOptionsWindow()
{
  // VIRTUAL FUNCTION
}

void cView::DrawStepWindow()
{
  // VIRTUAL FUNCTION
}

void cView::NavigateMapWindow()
{
  // VIRTUAL FUNCTION
}
void cView::Redraw()
{
  bar_window->Redraw();
  main_window->Redraw();
}

void cView::ClearAll()
{
  main_window->Clear();
  main_window->Refresh();
  // Do bar window last (leaves us at top of the screen)
  bar_window->Clear();
  bar_window->Refresh();
}

void cView::UpdateBarWindow()
{
  bar_window->SetBoldColor(COLOR_WHITE);
  bar_window->Print(1, 11, "%d", stats.GetUpdate());
  bar_window->SetColor(COLOR_WHITE);

  bar_window->Refresh();
}

void cView::UpdateMapWindow()
{
  // VIRTUAL FUNCTION
}

void cView::UpdateStatsWindow()
{
  // VIRTUAL FUNCTION
}

void cView::UpdateHistWindow()
{
  // VIRTUAL FUNCTION
}

void cView::UpdateOptionsWindow()
{
  // VIRTUAL FUNCTION
}

void cView::UpdateStepWindow()
{
  // VIRTUAL FUNCTION
}

void cView::ClearBarWindow()
{
  bar_window->Clear();
}

void cView::ClearMainWindow()
{
  main_window->Clear();
  main_window->Refresh();
}

void cView::NewUpdate()
{
  if (pause_level == PAUSE_ADVANCE_STEP) return;

  UpdateBarWindow();
  UpdateSymbols();
  UpdateWindows();
  DoInputs();
}

void cView::NotifyUpdate()
{
  UpdateBarWindow();
  UpdateSymbols();
  UpdateWindows();
  DoInputs();
}

void cView::NotifyError(const cString & in_string)
{
  cString out_string(in_string);
  out_string.Insert("Error: ");
  Notify(out_string);
  EndProg(1);
}
  
void cView::NotifyWarning(const cString & in_string)
{
  cString out_string(in_string);
  out_string.Insert("Warning: ");
  Notify(out_string);
}
  
void cView::NotifyComment(const cString & in_string)
{
  cString out_string(in_string);
  Notify(out_string);
}
  
void cView::UpdateSymbols()
{
  // First, clean up the genotype_chart & species_chart.

  int i, pos;
  for (i = 0; i < NUM_SYMBOLS; i++) {
    if (genotype_chart[i]) {
      pos = genebank->FindPos(genotype_chart[i]);
      if (pos < 0) genotype_chart[i] = NULL;
      if (pos >= NUM_SYMBOLS) {
	if (genotype_chart[i]->GetThreshold())
	  genotype_chart[i]->SetSymbol('+');
	else genotype_chart[i]->SetSymbol('.');
	genotype_chart[i] = NULL;
      }
    }
    if (species_chart[i]) {
      pos = genebank->FindPos(species_chart[i]);
      if (pos < 0) species_chart[i] = NULL;
      if (pos >= NUM_SYMBOLS) {
	species_chart[i]->SetSymbol('+');
	species_chart[i] = NULL;
      }
    }
  }
  
  // Now, fill in any missing spaces...
  
  cGenotype * temp_gen = genebank->GetBestGenotype();
  cSpecies * temp_species = genebank->GetFirstSpecies();
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

void cView::DoInputs()
{
  int cur_char = ERR;

  // If there is any input in the buffer, process all of it.

  while ((cur_char = GetInput()) != ERR || pause_level == PAUSE_ON) {
    switch (cur_char) {
      // Note: Capital 'Q' quits w/o confirming.
    case 'q':
      if (!Confirm("Are you sure you want to quit?")) break;
    case 'Q':
      ClearAll();  // clear the windows before we go
      EndProg(0);  // This implementation calls exit(), blowing us clean away
      break;
    case 'b':
    case 'B':
      view_mode = MODE_BLANK;
      ClearMainWindow();
      break; 
    case 'C':
    case 'c':
      NavigateMapWindow();
      // Now we need to restore the proper window mode
      switch (view_mode) {
      case MODE_STEP:
	DrawStepWindow();
	break;
      case MODE_MAP:
	DrawMapWindow();
	break;
      case MODE_HIST:
	DrawHistWindow();
	break;
      case MODE_OPTIONS:
	DrawOptionsWindow();
	break;
      case MODE_BLANK:	// window is already cleared...
      default:
	break;
      }
      break;
    case 'E':
    case 'e':
      ExtractCreature();
      break;
    case 'm':
    case 'M':
      view_mode = MODE_MAP;
      ClearMainWindow();
      DrawMapWindow();
      break;
    case 's':
    case 'S':
      view_mode = MODE_STATS;
      ClearMainWindow();
      DrawStatsWindow();
      break;
    case 'h':
    case 'H':
      view_mode = MODE_HIST;
      ClearMainWindow();
      DrawHistWindow();
      break;
    case 'o':
    case 'O':
      view_mode = MODE_OPTIONS;
      ClearMainWindow();
      DrawOptionsWindow();
      break;
    case 'r':
    case 'R':
      Redraw();
      break;
    case 'p':
    case 'P':
      TogglePause();
      break;
    case 'n':
    case 'N':
      if (pause_level == PAUSE_ON) {
	pause_level = PAUSE_ADVANCE_UPDATE;
	parasite_zoom = FALSE; // if exec'ing code, show code that is running
      }
      break;
    case 'z':
    case 'Z':
      view_mode = MODE_STEP;
      ClearMainWindow();
      DrawStepWindow();
      break;
    case '*':   // Test Key!!!
      if (TRUE) {
	Confirm("Starting Tests.");
	cMenuWindow menu(50);
	char message[40];
	for (int j = 0; j < 50; j++) {
	  sprintf(message, "Line %d", j);
	  menu.AddOption(j, message);
	}
	menu.SetActive(3);
	menu.Activate();
	Redraw();
      }
      break;
    case ERR:
      break;
    default:
      switch (view_mode) {
      case MODE_MAP:
	MapInput(cur_char);
	break;
      case MODE_STATS:
	StatsInput(cur_char);
	break;
      case MODE_HIST:
	HistInput(cur_char);
	break;
      case MODE_OPTIONS:
	OptionsInput(cur_char);
	break;
      case MODE_STEP: 
	StepInput(cur_char);
	break;
      }
      break;
    }
  }

  if (pause_level == PAUSE_ADVANCE_UPDATE) pause_level = PAUSE_ON;
}

void cView::MapInput(int in_char)
{
  switch (in_char) {
  case 0:  // Keep Visual C compiler from spitting warnings about no cases...
  default:
    break;
  }
}

void cView::HistInput(int in_char)
{
  switch (in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    g_debug.Comment("Unknown Character: %c (#%d)", in_char, (int) in_char);
    break;
  }
}

void cView::StatsInput(int in_char)
{
  switch (in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    g_debug.Comment("Unknown Character: %c (#%d)", in_char, (int) in_char);
    break;
  }
}

void cView::OptionsInput(int in_char)
{
  switch (in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    break;
  }
}

void cView::StepInput(int in_char)
{
  switch (in_char) {
  case ' ':
    memory_offset = 0;
    parasite_zoom = FALSE;
    EngageStepMode();
    break;
  default:
    break;
  }
}

void cView::UpdateWindows()
{
  switch (view_mode) {
  case MODE_MAP:
    UpdateMapWindow();
    break;
  case MODE_STATS:
    UpdateStatsWindow();
    break;
  case MODE_HIST:
    UpdateHistWindow();
    break;
  case MODE_OPTIONS:
    UpdateOptionsWindow();
    break;
  case MODE_STEP:
    UpdateStepWindow();
    break;
  }
}

void cView::TogglePause()
{
  // If the run is already paused, un-pause it!
  if (pause_level) {
    DisEngageStepMode();
    pause_level = PAUSE_OFF;
  }

  // Otherwise, turn on the pause.
  else {
    pause_level = PAUSE_ON;
  }

  // If we are in the options or Zoom window, redraw the screen.
  if (view_mode == MODE_OPTIONS) {
    DrawOptionsWindow();
  }
  else if (view_mode == MODE_STEP) {
    DrawStepWindow();
  }
}

void cView::ExtractCreature()
{
  // Virtual Function...
}

void cView::PrintMerit(int in_y, int in_x, cMerit in_merit)
{
  // if we can print the merit normally, do so.
  if (in_merit.GetUInt() < 100000 && in_merit.GetShift() < 20) {
    main_window->Print(in_y, in_x, "%6d", in_merit.GetUInt());
  }

  // otherwise use scientific notation. (or somesuch)
  else {
    int base = (int) in_merit.GetBase();
    int shift = (int) in_merit.GetShift();
    while (base >= 10) {
      base >>= 1;
      shift++;
    }
    main_window->Print(in_y, in_x, "%1dx2^%2d", base, shift);	      
  }
}

void cView::PrintFitness(int in_y, int in_x, double in_fitness)
{
  // If we can print the fitness, do so!
  if (in_fitness <= 0.0) {
    main_window->Print(in_y, in_x, " 0.0000");
  }
  else if (in_fitness < 10)
    main_window->Print(in_y, in_x, "%7.4f", in_fitness);
  //  else if (in_fitness < 100)
  //    main_window->Print(in_y, in_x, "%7.3f", in_fitness);
  else if (in_fitness < 1000)
    main_window->Print(in_y, in_x, "%7.2f", in_fitness);
  //  else if (in_fitness < 10000)
  //    main_window->Print(in_y, in_x, "%7.1f", in_fitness);
  else if (in_fitness < 100000)
    main_window->Print(in_y, in_x, "%7.0f", in_fitness);

  // Otherwise use scientific notations.
  else
    main_window->Print(in_y, in_x, "%7.1e", in_fitness);
}

int cView::Confirm(const cString & message)
{
  int mess_length = message.GetSize();

  // Create a confirm window, and draw it on the screen.

  cTextWindow * conf_win
    = new cTextWindow(3, mess_length + 10, 10,
		      (main_window->Width() - mess_length - 10) / 2);
  conf_win->Box();
  conf_win->SetBoldColor(COLOR_WHITE);
  conf_win->Print(1, 2, "%s (y/n)", message());
  conf_win->SetBoldColor(COLOR_CYAN);
  conf_win->Print(1, mess_length + 4, 'y');
  conf_win->Print(1, mess_length + 6, 'n');
  conf_win->SetColor(COLOR_WHITE);
  conf_win->Refresh();
  
  // Wait for the results.
  int finished = FALSE;
  int result = FALSE;
  int cur_char;

  while (!finished) {
    cur_char = GetInput();
    switch (cur_char) {
    case 'q':
    case 'Q':
    case 'n':
    case 'N':
    case ' ':
    case '\n':
    case '\r':
      finished = TRUE;
      result = FALSE;
      break;
    case 'y':
    case 'Y':
      finished = TRUE;
      result = TRUE;
      break;
    }
  }

  // Delete the window, redraw the screen, and return the results.
  delete conf_win;
  Redraw();
  return result;
}

void cView::Notify(const cString & message)
{
  cString mess_copy(message);

  // Setup all of the individual lines.
  int num_lines = message.CountNumLines();
  cString * line_array = new cString[num_lines];
  int max_width = 0;
  for (int i = 0; i < num_lines; i++) {
    line_array[i] = mess_copy.PopLine();
    if (line_array[i].GetSize() > max_width)
      max_width = line_array[i].GetSize();
  }

  // Create a window and draw it on the screen.

  cTextWindow * notify_win
    = new cTextWindow(2 + num_lines, max_width + 4, (24 - num_lines - 3) / 2,
		      (main_window->Width() - max_width - 10) / 2);
  notify_win->Box();
  notify_win->SetBoldColor(COLOR_WHITE);
  for (int j = 0; j < num_lines; j++) {
    notify_win->Print(1 + j, 2, "%s", line_array[j]());
  }
  notify_win->Refresh();
  
  // Wait for the results.
  int finished = FALSE;
  int cur_char;

  while (!finished) {
    cur_char = GetInput();
    switch (cur_char) {
    case 'q':
    case 'Q':
    case ' ':
    case '\n':
    case '\r':
      finished = TRUE;
      break;
    }
  }

  // Delete the window and redraw the screen.
  delete notify_win;
  delete [] line_array;
  Redraw();
}

void cView::EngageStepMode()
{
  // VIRTUAL FUNCTION
  g_debug.Comment("in VIRTUAL FUNCTION - cView::EngageStepMode()");
}

void cView::DisEngageStepMode()
{
  // VIRTUAL FUNCTION
  g_debug.Comment("in VIRTUAL FUNCTION - cView::DisEngageStepMode()");
}


////////////////////
//  cGAView
////////////////////

cGAView::cGAView(cGAPopulation * in_population)
{
  population = in_population;
  genebank = population->GetGenebank();
  alt_inst_lib = new cInstLib(population->GetInstLib());
  alt_inst_lib->ActivateAltFunctions();
  DrawBarWindow();
}

void cGAView::DrawBarWindow()
{
  cView::DrawBarWindow();
  bar_window->SetBoldColor(COLOR_WHITE);
  bar_window->Print(1, bar_window->Width() - 8, "GA");
  bar_window->SetColor(COLOR_WHITE);
}

void cGAView::DrawMapWindow()
{
}

void cGAView::DrawStatsWindow()
{
}

void cGAView::DrawHistWindow()
{
}

void cGAView::DrawOptionsWindow()
{
}

void cGAView::DrawStepWindow()
{
}

void cGAView::UpdateMapWindow()
{
}

void cGAView::UpdateStatsWindow()
{
}

void cGAView::UpdateHistWindow()
{
}

void cGAView::UpdateOptionsWindow()
{
}

void cGAView::UpdateStepWindow()
{
}

void cGAView::NavigateMapWindow()
{
}

void cGAView::MapInput(int in_char)
{
  switch(in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    break;
  }
}

void cGAView::OptionsInput(int in_char)
{
  switch(in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    break;
  }
}

////////////////////
//  cAAView
////////////////////

cAAView::cAAView()
{
  active_cpu = NULL;
}

void cAAView::DrawBarWindow()
{
  // VIRTUAL FUNCTION
}

void cAAView::DrawMapWindow()
{
  // VIRTUAL FUNCTION
}

void cAAView::DrawStatsWindow()
{
  main_window->SetBoldColor(COLOR_WHITE);

  // main_window->Print(0,  0, "--- Soup Status ---");
  main_window->Print(1,  0, "Tot Births.:");
  main_window->Print(2,  0, "Breed True.:");
  main_window->Print(3,  0, "Parasites..:");
  main_window->Print(4,  0, "Energy.....:");
  main_window->Print(5,  0, "Max Fitness:");
  main_window->Print(6,  0, "Max Merit..:");

  main_window->Print(1, 23, "-- Dominant Genotype --");
  main_window->Print(2, 23, "Name........:");
  main_window->Print(3, 23, "ID..........:");
  main_window->Print(4, 23, "Species ID..:");  
  main_window->Print(5, 23, "Age.........:");  

  main_window->Print(8, 11, "Current    Total  Ave Age  Entropy");
  main_window->Print(9,  0, "Creatures:");
  main_window->Print(10, 0, "Genotypes:");
  main_window->Print(11, 0, "Threshold:");
  main_window->Print(12, 0, "Species..:");

  main_window->Print(1, 61, "Dominant  Average");
  main_window->Print(2, 50, "Fitness..:");
  main_window->Print(3, 50, "Merit....:");
  main_window->Print(4, 50, "Gestation:");
  main_window->Print(5, 50, "Size.....:");
  main_window->Print(6, 50, "Copy Size:");
  main_window->Print(7, 50, "Exec Size:");
  main_window->Print(8, 50, "Abundance:");
  main_window->Print(9, 50, "Births...:");
  main_window->Print(10, 50, "BirthRate:");

  main_window->Print(15,  2, "Input...:");
  main_window->Print(16,  2, "Output..:");
  main_window->Print(17,  2, "I/O.....:");
  main_window->Print(18,  2, "Echo....:");
  main_window->Print(19,  2, "Nand....:");

  main_window->Print(15, 24, "Not.....:");
  main_window->Print(16, 24, "And.....:");
  main_window->Print(17, 24, "~A Or B.:");
  main_window->Print(18, 24, "~A And B:");
  main_window->Print(19, 24, "Or......:");

  main_window->Print(15, 46, "Nor.....:");
  main_window->Print(16, 46, "Xor.....:");
  main_window->Print(17, 46, "Equals..:");

  main_window->SetColor(COLOR_WHITE);

  main_window->Box(0, 14, main_window->Width(), 7);

  UpdateStatsWindow();
}

void cAAView::DrawHistWindow()
{
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(1,  0, "Fitness Name");
  main_window->Print(1, 19, "Histogram: [ ]");
  main_window->Print(1, 53, "[ ]");
  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(1, 31, '<');
  main_window->Print(1, 54, '>');
  UpdateHistWindow();
}

void cAAView::DrawOptionsWindow()
{
  // Options - Used: ABCEIMOPQRSVWZ

  main_window->SetBoldColor(COLOR_WHITE);
 
  main_window->Print(5,  0, "Max Updates..:");
  main_window->Print(6,  0, "World Size...:");
  main_window->Print(7,  0, "Random Seed..:");
  main_window->Print(8,  0, "Threshold....:");

  main_window->Print(10, 0, "Debug Level..:");
  main_window->Print(11, 0, "Inst Set.....:");
  main_window->Print(12, 0, "Task Set.....:");
  main_window->Print(13, 0, "Events File..:");

  main_window->Print(1, 40, "Time Slicing.:");
  main_window->Print(2, 40, "Task Merit...:");
  main_window->Print(3, 40, "Size Merit...:");
  main_window->Print(4, 40, "Birth Method.:");
  main_window->Print(5, 40, "Ave TimeSlice:");

  main_window->Print(7, 40, "Point  Mut:");
  main_window->Print(8, 40, "Copy   Mut:");
  main_window->Print(9, 40, "Divide Mut:      Ins:      Del:");

  main_window->Print(1, 0, "Current CPU..:");
  main_window->Print(2, 0, "Genotype.....:");
  main_window->Print(3, 0, "ID #.........:");

  // main_window->SetBoldColor(COLOR_WHITE);
  //  main_window->Print(main_window->Height() - 5, 2, "-- Screen --");
  main_window->Print(main_window->Height() - 4, 2, "[H]istogram Screen");
  main_window->Print(main_window->Height() - 3, 2, "[B]lank Screen");
  main_window->Print(main_window->Height() - 2, 2, "[R]edraw Screen");

  main_window->Print(main_window->Height() - 4, 30, "[C]hoose New CPU");
  main_window->Print(main_window->Height() - 3, 30, "[E]xtract Creature");
  //  main_window->Print(main_window->Height() - 3, 30, "[I]nject Creature");
  //  main_window->Print(main_window->Height() - 2, 30, "[W]rite Soup");

  //  main_window->Print(main_window->Height() - 5, 55, "[V]iew Instructions");
  if (pause_level) {
    main_window->Print(main_window->Height() - 4, 55, "Un-[P]ause");
    main_window->Print(main_window->Height() - 3, 55, "[N]ext Update");
  } else {
    main_window->Print(main_window->Height() - 4, 55, "[P]ause   ");
    main_window->Print(main_window->Height() - 3, 55, "             ");
  }

  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(main_window->Height() - 4, 3, 'H');
  main_window->Print(main_window->Height() - 3, 3, 'B');
  main_window->Print(main_window->Height() - 2, 3, 'R');

  main_window->Print(main_window->Height() - 4, 31, 'C');
  main_window->Print(main_window->Height() - 3, 31, 'E');
  if (pause_level) {
    main_window->Print(main_window->Height() - 4, 59, 'P');
    main_window->Print(main_window->Height() - 3, 56, 'N');
  } else {
    main_window->Print(main_window->Height() - 4, 56, 'P');
  }

  main_window->SetColor(COLOR_WHITE);
  main_window->Box(0, main_window->Height() - 5, main_window->Width(), 5);

  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(5, 15, "%d", stats.GetMaxUpdates());
  main_window->Print(7, 15, "%d", g_random.GetOriginalSeed());
  main_window->Print(8, 15, "%d", genebank->GetThreshold());

  switch (g_debug.GetLevel()) {
  case DEBUG_NONE:
    main_window->Print(10, 15, "OFF");
    break;
  case DEBUG_ERROR:
    main_window->Print(10, 15, "ERRORS");
    break;
  case DEBUG_WARNING:
    main_window->Print(10, 15, "WARNINGS");
    break;
  case DEBUG_COMMENT:
    main_window->Print(10, 15, "COMMENTS");
    break;
  default:
    main_window->Print(10, 15, "FULL");
    break;
  }

  main_window->Print(11, 15, "%s", stats.GetInstFilename()());
  main_window->Print(12, 15, "%s", stats.GetTaskFilename()());
  main_window->Print(13, 15, "%s", stats.GetEventFilename()());

  switch(population->GetSlicingMethod()) {
  case SLICE_CONSTANT:
    main_window->Print(1, 55, "Constant");
    break;
  case SLICE_BLOCK_MERIT:
    main_window->Print(1, 55, "Scaled Block");
    break;
  case SLICE_PROB_MERIT:
    main_window->Print(1, 55, "Probablistic");
    break;
  case SLICE_INTEGRATED_MERIT:
    main_window->Print(1, 55, "Integrated");
    break;
  case SLICE_LOGRITHMIC_MERIT:
    main_window->Print(1, 55, "Logrithmic Integrated");
    break;
  }

  switch(stats.GetTaskMeritMethod()) {
  case TASK_MERIT_OFF:
    main_window->Print(2, 55, "Off");
    break;
  case TASK_MERIT_EXPONENTIAL:
    main_window->Print(2, 55, "Exponential");
    break;
  case TASK_MERIT_LINEAR:
    main_window->Print(2, 55, "Linear");
    break;
  }

  switch(stats.GetSizeMeritMethod()) {
  case SIZE_MERIT_OFF:
    main_window->Print(3, 55, "Off");
    break;    
  case SIZE_MERIT_COPIED:
    main_window->Print(3, 55, "Copied Size");
    break;    
  case SIZE_MERIT_EXECUTED:
    main_window->Print(3, 55, "Executed Size");
    break;    
  case SIZE_MERIT_FULL:
    main_window->Print(3, 55, "Full Size");
    break;    
  case SIZE_MERIT_LEAST:
    main_window->Print(3, 55, "Least Size");
    break;    
  }

  switch(stats.GetBirthMethod()) {
  case POSITION_CHILD_AGE:
    main_window->Print(4, 55, "Replace max age");
    break;
  case POSITION_CHILD_MERIT:
    main_window->Print(4, 55, "Replace max age/merit");
    break;
  case POSITION_CHILD_RANDOM:
    main_window->Print(4, 55, "Replace random");
    break;
  case POSITION_CHILD_EMPTY:
    main_window->Print(4, 55, "Place in empty only");
    break;
  }
  
  main_window->Print(5, 55, "%d", population->GetAveTimeslice());

  main_window->Print(7, 52, "%d",
		     population->GetLocalStats()->GetPointMutRate());
  main_window->Print(8, 52, "%d",
		     population->GetLocalStats()->GetCopyMutRate());
  main_window->Print(9, 52, "%d",
		     population->GetLocalStats()->GetDivideMutRate());
  main_window->Print(9, 62, "%d",
		     population->GetLocalStats()->GetDivideInsRate());
  main_window->Print(9, 72, "%d",
		     population->GetLocalStats()->GetDivideDelRate());

  main_window->SetColor(COLOR_WHITE);

  UpdateOptionsWindow();

  main_window->Refresh();
}

void cAAView::DrawStepWindow()
{
  // Creature Status

  main_window->SetBoldColor(COLOR_WHITE);

  main_window->Print(1,  0, "Current CPU.:");
  main_window->Print(2,  0, "Genotype....:");
  main_window->Print(3,  0, "Species.....:");

  main_window->Print(5,  0, "Gestation...:");
  main_window->Print(6,  0, "CurrentMerit:");
  main_window->Print(7, 0, "LastMerit...:");
  main_window->Print(8, 0, "Fitness.....:");
  main_window->Print(9, 0, "Offspring...:");
  main_window->Print(10, 0, "Errors......:");
  main_window->Print(11, 0, "Age.........:");
  main_window->Print(12, 0, "Executed....:");
  main_window->Print(13, 0, "Last Divide.:");
  main_window->Print(14, 0, "Flags.......:");

  if (pause_level) {
    main_window->Print(17, 0, "Un-[P]ause");
    main_window->Print(18, 0, "[N]ext Update");
    main_window->Print(19, 0, "[Space] Next Instruction");
    main_window->Print(20, 0, "[-] and [+] Scroll Memory");
  } else {
    main_window->Print(17, 0, "[P]ause   ");
    main_window->Print(18, 0, "             ");
    main_window->Print(19, 0, "                        ");
    main_window->Print(20, 0, "                         ");
  }

  main_window->SetBoldColor(COLOR_CYAN);

  if (pause_level) {
    main_window->Print(17, 4, 'P');
    if (active_cpu && active_cpu->GetFlag(CPU_FLAG_PARASITE)) {
      main_window->Print(17, 13, 'X');
    }
    main_window->Print(18, 1, 'N');
    main_window->Print( 19, 1, "Space");
    main_window->Print(20, 1, '-');
    main_window->Print(20, 9, '+');
  } else {
    main_window->Print(17, 1, 'P');
  }

  main_window->SetColor(COLOR_WHITE);

  main_window->Box(TASK_X, TASK_Y, 33, 7);
  main_window->Print(TASK_Y + 1, TASK_X + 2, "Get.:");
  main_window->Print(TASK_Y + 2, TASK_X + 2, "Put.:");
  main_window->Print(TASK_Y + 3, TASK_X + 2, "GGP.:");
  main_window->Print(TASK_Y + 4, TASK_X + 2, "Echo:");
  main_window->Print(TASK_Y + 5, TASK_X + 2, "Nand:");

  main_window->Print(TASK_Y + 1, TASK_X + 12, "Not.:");
  main_window->Print(TASK_Y + 2, TASK_X + 12, "And.:");
  main_window->Print(TASK_Y + 3, TASK_X + 12, "~Or.:");
  main_window->Print(TASK_Y + 4, TASK_X + 12, "~And:");
  main_window->Print(TASK_Y + 5, TASK_X + 12, "Or..:");

  main_window->Print(TASK_Y + 1, TASK_X + 22, "Nor.:");
  main_window->Print(TASK_Y + 2, TASK_X + 22, "Xor.:");
  main_window->Print(TASK_Y + 3, TASK_X + 22, "Equ.:");

  main_window->Box(REG_X, REG_Y, 19, 5); 
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(REG_Y + 1, REG_X + 2, "AX:");
  main_window->Print(REG_Y + 2, REG_X + 2, "BX:");
  main_window->Print(REG_Y + 3, REG_X + 2, "CX:");
  main_window->SetColor(COLOR_WHITE);

  main_window->Box(INPUT_X, INPUT_Y, 16, 7);
  main_window->Print(INPUT_Y + 1, INPUT_X + 2, "Inputs");
  main_window->HLine(INPUT_Y + 2, INPUT_X, 16);

  main_window->Box(MINI_MAP_X, MINI_MAP_Y, 19, 10);
  main_window->SetBoldColor(COLOR_WHITE);
  main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 1,  "[<]           [>]");
  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 2,  '<');
  main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 16, '>');
  main_window->SetColor(COLOR_WHITE);

  main_window->Box(MEMORY_X, MEMORY_Y, 16, 14);
  main_window->Print(MEMORY_Y + 1, MEMORY_X + 2,  "Memory:");
  main_window->HLine(MEMORY_Y + 2, MEMORY_X, 16);

  main_window->Box(STACK_X, STACK_Y, 15, 14);
  main_window->Print(STACK_Y + 1, STACK_X + 2, "Stack A");
  main_window->HLine(STACK_Y + 2, STACK_X, 15);

  // Add on a bunch of special characters to smooth the view out...
  main_window->Print(MINI_MAP_Y, MINI_MAP_X, CHAR_TTEE);
  main_window->Print(STACK_Y, STACK_X,       CHAR_TTEE);
  main_window->Print(STACK_Y + 2, STACK_X,   CHAR_PLUS);
  main_window->Print(REG_Y, REG_X,           CHAR_LTEE);
  main_window->Print(REG_Y, REG_X + 18,      CHAR_RTEE);
  main_window->Print(STACK_Y + 13, STACK_X + 14,  CHAR_BTEE);
  main_window->Print(TASK_Y, TASK_X,         CHAR_PLUS);
  main_window->Print(INPUT_Y, INPUT_X,       CHAR_LTEE);
  main_window->Print(TASK_Y, TASK_X + 32,    CHAR_RTEE);
  main_window->Print(TASK_Y + 6, TASK_X,     CHAR_BTEE);

  UpdateStepWindow();

  main_window->Refresh();
}

void cAAView::UpdateMapWindow()
{
  // VIRTUAL FUNCTION
}

void cAAView::UpdateStatsWindow()
{
  cGenotype * best_gen = genebank->GetBestGenotype();
  cLocalStats * local_stats = population->GetLocalStats();

  main_window->SetBoldColor(COLOR_CYAN);

  main_window->Print(1, 13, "%7d",   local_stats->GetNumDeaths());
  main_window->Print(2, 13, "%7d",   local_stats->GetBreedTrue());
  main_window->Print(3, 13, "%7d",   local_stats->GetNumParasites());
  main_window->Print(4, 13, "%7.2f", local_stats->GetEnergy());  
  PrintFitness(5, 13, local_stats->GetMaxFitness());  
  // main_window->Print(5, 13, "%.1e",  local_stats->GetMaxFitness());  
  main_window->Print(6, 13, "%.1e",  local_stats->GetMaxMerit());  

  main_window->Print(9,  13, "%5d", local_stats->GetNumCreatures());
  main_window->Print(10, 13, "%5d", local_stats->GetNumGenotypes());
  main_window->Print(11, 13, "%5d", local_stats->GetNumThreshold());
  main_window->Print(12, 13, "%5d", local_stats->GetNumSpecies());
 
  main_window->Print(2, 37, "%s",  best_gen->GetName()());
  main_window->Print(3, 37, "%9d", best_gen->GetID());
  main_window->Print(4, 37, "%9d", best_gen->GetSpecies()->GetID());
  main_window->Print(5, 37, "%9d", best_gen->GetAge());

  main_window->Print(9,  20, "%.1e", (double) local_stats->GetTotCreatures());
  main_window->Print(10, 20, "%.1e", (double) local_stats->GetTotGenotypes());
  main_window->Print(11, 20, "%.1e", (double) local_stats->GetTotThreshold());
  main_window->Print(12, 20, "%.1e", (double) local_stats->GetTotSpecies());

  main_window->Print(9,  30, "%6.1f", local_stats->GetAveCreatureAge());
  main_window->Print(10, 30, "%6.1f", local_stats->GetAveGenotypeAge());
  main_window->Print(11, 30, "%6.1f", local_stats->GetAveThresholdAge());
  main_window->Print(12, 30, "%6.1f", local_stats->GetAveSpeciesAge());

  main_window->Print(9,  39, "%6.2f",
		     log((double) local_stats->GetNumCreatures()));
  main_window->Print(10, 39, "%6.2f", local_stats->GetEntropy());
  main_window->Print(12, 39, "%6.2f", local_stats->GetSpeciesEntropy());

  PrintFitness(2, 62, best_gen->GetFitness());
  // main_window->Print(2, 62, "%.1e", best_gen->GetFitness());
  PrintMerit(3, 63, best_gen->GetMerit());
  main_window->Print(4, 63, "%6d", best_gen->GetGestationTime());
  main_window->Print(5, 63, "%6d", best_gen->GetLength());
  main_window->Print(6, 63, "%6d", best_gen->GetCopiedSize());
  main_window->Print(7, 63, "%6d", best_gen->GetExecutedSize());
  main_window->Print(8, 63, "%6d", best_gen->GetNumCPUs());
  main_window->Print(9, 63, "%6d", best_gen->GetCreaturesBorn());
  if (!local_stats->GetAveMerit().GetBase()) {
    main_window->Print(10, 66, "0.0");
  } else {
    main_window->Print(10, 63, "%6.3f", ((double) stats.GetAveTimeslice()) * best_gen->GetFitness() / local_stats->GetAveMerit().GetDouble());
  }

  PrintFitness(2, 71, local_stats->GetAveFitness());
  // main_window->Print(2, 71, "%.1e", local_stats->GetAveFitness());
  PrintMerit(3, 72, local_stats->GetAveMerit());
  main_window->Print(4, 72, "%6.1f", local_stats->GetAveGestation());
  main_window->Print(5, 72, "%6.1f", local_stats->GetAveSize());
  main_window->Print(6, 72, "%6.1f", local_stats->GetAveCopySize());
  main_window->Print(7, 72, "%6.1f", local_stats->GetAveExeSize());
  main_window->Print(8, 71, "%7.2f",
		     ((double) local_stats->GetNumCreatures()) /
		     ((double) local_stats->GetNumGenotypes()));
  // @CAO this next line should be get num births, which doesn't exist!
  main_window->Print(9, 70, "%8.3f",
		     ((double) local_stats->GetNumDeaths()) /
		     ((double) local_stats->GetNumGenotypes()));
  if (local_stats->GetAveMerit().GetBase()) {
    main_window->Print(10, 71, "%7.3f", ((double) stats.GetAveTimeslice()) * local_stats->GetAveFitness() / local_stats->GetAveMerit().GetDouble());
  } else {
    main_window->Print(10, 71, "%7.3f", 0.0);
  }

  main_window->Print(15, 13, "%5d", local_stats->GetGetCount());
  main_window->Print(16, 13,"%5d", local_stats->GetPutCount());
  main_window->Print(17, 13, "%5d", local_stats->GetGGPCount());
  main_window->Print(18, 13, "%5d", local_stats->GetTaskCount(TASK_ECHO));
  main_window->Print(19, 13, "%5d", local_stats->GetTaskCount(TASK_NAND));

  main_window->Print(15, 35, "%5d", local_stats->GetTaskCount(TASK_NOT));
  main_window->Print(16, 35, "%5d", local_stats->GetTaskCount(TASK_AND));
  main_window->Print(17, 35, "%5d", local_stats->GetTaskCount(TASK_ORN));
  main_window->Print(18, 35, "%5d", local_stats->GetTaskCount(TASK_ANDN));
  main_window->Print(19, 35, "%5d", local_stats->GetTaskCount(TASK_OR));

  main_window->Print(15, 57, "%5d", local_stats->GetTaskCount(TASK_NOR));
  main_window->Print(16, 57, "%5d", local_stats->GetTaskCount(TASK_XOR));
  main_window->Print(17, 57, "%5d", local_stats->GetTaskCount(TASK_EQU));

  main_window->SetColor(COLOR_WHITE);

  main_window->Refresh();
}

void cAAView::UpdateHistWindow()
{
  int max_num = 0;
  int i;

  
  switch(hist_mode) {
  case HIST_GENOTYPE: 
    max_num = genebank->GetBestGenotype()->GetNumCPUs();
    main_window->SetBoldColor(COLOR_WHITE);
    main_window->Print(1,  34, "Genotype Abundance");
    // Print out top NUM_SYMBOL genotypes in fixed order.
    for (i = 0; i < NUM_SYMBOLS; i++) {
      if (genotype_chart[i]) {
	PrintGenotype(genotype_chart[i], i + 2, max_num);
      }
      else {
	main_window->Move(i + 2, 0);
	main_window->ClearToEOL();
      }
    }
    break;
  case HIST_SPECIES:
    max_num = 0;
    for (i = 0; i < NUM_SYMBOLS; i++) {
      if (species_chart[i] && species_chart[i]->GetNumCreatures() > max_num)
	max_num = species_chart[i]->GetNumCreatures();
    }

    main_window->SetBoldColor(COLOR_WHITE);
    main_window->Print(1,  34, "Species Abundance");
    // Print out top NUM_SYMBOL species in fixed order.
    for (i = 0; i < NUM_SYMBOLS; i++) {
      if (species_chart[i]) {
	PrintSpecies(species_chart[i], i + 2, max_num);
      }
      else {
	main_window->Move(i + 2, 0);
	main_window->ClearToEOL();
      }
    }
    break;
  }

  main_window->ClearToBot();
  main_window->Refresh();
}

void cAAView::UpdateOptionsWindow()
{
  if (!active_cpu) return;

  main_window->SetBoldColor(COLOR_CYAN);

  main_window->Print(2, 15, "%s",active_cpu->GetActiveGenotype()->GetName()());
  main_window->Print(3, 15, "%d",active_cpu->GetEnvironment()->GetID());

  main_window->SetColor(COLOR_WHITE);

  main_window->Refresh();
}

void cAAView::UpdateStepWindow()
{
  if (!active_cpu) return;

  main_window->SetBoldColor(COLOR_CYAN);

  cInstLib * inst_lib = population->GetInstLib();

  // if there is an Active Genotype name AND it is not empty, show it
  main_window->Print(2, 14, "%s",active_cpu->GetActiveGenotype()->GetName()());

  if (active_cpu->GetActiveGenotype()->GetSpecies())
	   main_window->Print(3, 14, "spec-%d", active_cpu->GetActiveGenotype()->GetSpecies()->GetID());
  else
	  main_window->Print(3, 14, "(none)    ");

  main_window->Print(5, 15, "%6d ", active_cpu->GetGestationTime());
  PrintMerit(6, 15, active_cpu->GetCurMerit());
  PrintMerit(7, 15, active_cpu->GetMerit());
  PrintFitness(8, 14, active_cpu->GetFitness());
  main_window->Print(9, 15, "%6d ", active_cpu->GetNumDivides());
  main_window->Print(10, 15, "%6d ", active_cpu->GetNumErrors());
  main_window->Print(11, 15, "%6d ", active_cpu->GetAge());
  main_window->Print(12, 15, "%6d ", active_cpu->GetTotalTimeUsed());
  main_window->Print(13, 15, "%6d ", active_cpu->GetCurGestation());

  // Flags...
  main_window->Print(14, 15, "        ");
  if (active_cpu->GetFlag(CPU_FLAG_MAL_ACTIVE))
    main_window->Print(14, 15, 'A');
  if (active_cpu->GetFlag(CPU_FLAG_PARENT_TRUE))
    main_window->Print(14, 16, 'T');
  if (active_cpu->GetFlag(CPU_FLAG_INJECTED))
    main_window->Print(14, 17, 'I');
  if (active_cpu->GetFlag(CPU_FLAG_PARASITE))
    main_window->Print(14, 18, 'P');
  if (active_cpu->GetFlag(CPU_FLAG_POINT_MUT))
    main_window->Print(14, 19, 'M');
      
  if (pause_level && active_cpu && active_cpu->GetFlag(CPU_FLAG_PARASITE)) {
    if (parasite_zoom) main_window->Print(17, 12, "[X] Host Code    ");
    else main_window->Print(17, 12, "[X] Parasite Code");
  }

  cPhenotype * phenotype = active_cpu->GetPhenotype();
  
  int i;
  static int stack_pointer = 0;

  // Place the task information onto the screen.

  main_window->SetColor(COLOR_CYAN);

  main_window->Print(TASK_Y + 1, TASK_X + 8, "%2d",
	    phenotype->GetGetCount());
  main_window->Print(TASK_Y + 2, TASK_X + 8, "%2d",
	    phenotype->GetPutCount());
  main_window->Print(TASK_Y + 3, TASK_X + 8, "%2d",
	    phenotype->GetGGPCount());
  main_window->Print(TASK_Y + 4, TASK_X + 8, "%2d",
	    phenotype->GetTaskCount(0));
  main_window->Print(TASK_Y + 5, TASK_X + 8, "%2d",
	    phenotype->GetTaskCount(1));

  main_window->Print(TASK_Y + 1, TASK_X + 18, "%2d",
	    phenotype->GetTaskCount(2));
  main_window->Print(TASK_Y + 2, TASK_X + 18, "%2d",
	    phenotype->GetTaskCount(3));
  main_window->Print(TASK_Y + 3, TASK_X + 18, "%2d",
	    phenotype->GetTaskCount(4));
  main_window->Print(TASK_Y + 4, TASK_X + 18, "%2d",
	    phenotype->GetTaskCount(5));
  main_window->Print(TASK_Y + 5, TASK_X + 18, "%2d",
	    phenotype->GetTaskCount(6));

  main_window->Print(TASK_Y + 1, TASK_X + 28, "%2d",
	    phenotype->GetTaskCount(7));
  main_window->Print(TASK_Y + 2, TASK_X + 28, "%2d",
	    phenotype->GetTaskCount(8));
  main_window->Print(TASK_Y + 3, TASK_X + 28, "%2d",
	    phenotype->GetTaskCount(9));

  // Place the registers onto the screen.

  main_window->SetBoldColor(COLOR_CYAN);
  for (i = 0; i < NUM_REGISTERS; i++) {
    main_window->Print(REG_Y+1 + i, REG_X+6, "%11d",
		       active_cpu->GetRegister(i));
  }

  // Place the active stack onto the screen.

  // Stack A
  // main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(STACK_Y+3, STACK_X + 2, "%11d", active_cpu->GetStack(0));
  main_window->SetColor(COLOR_CYAN);
  for (i = 1; i <= 9; i++) {
    main_window->Print(STACK_Y+3 + i, STACK_X + 2,
		       "%11d", active_cpu->GetStack(i));
  }

  // Place the input buffer on the screen.

  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(INPUT_Y+3, INPUT_X+2, "%12d", active_cpu->GetInput(0));

  main_window->SetColor(COLOR_CYAN);
  for (i = 1; i < IO_SIZE; i++) {
    main_window->Print(INPUT_Y+3+i, INPUT_X+2, "%12d",active_cpu->GetInput(i));
  }

  // Place the visible section of the current memory onto the screen.

  main_window->SetBoldColor(COLOR_WHITE);
  if (pause_level == PAUSE_OFF) {
    // If not paused, then set user desired offset to zero AND always show
    // the code that is executing (ie. do not show original Parasite code if
    // host is executing)
    memory_offset = 0;
    parasite_zoom = FALSE;
  }

  // This line gets the creature that is currently executing. Usually the
  // creature we are viewing, but can also be a different one (if this is a
  // parasite).

  const cCodeArray & memory = (parasite_zoom) ?
    active_cpu->GetMemory() : active_cpu->GetInstPointer().GetMemory();

  main_window->SetColor(COLOR_WHITE);
  main_window->Print(MEMORY_Y + 1, MEMORY_X + 9, "%4d", memory.GetSize());

  int adj_inst_ptr = 0;
  int base_inst_ptr = active_cpu->GetInstPointer().GetPosition();
  if (base_inst_ptr < 0 || parasite_zoom) base_inst_ptr = 0;

  for (i = -4; i <= 5; i++) {
    adj_inst_ptr = i + base_inst_ptr + memory_offset;
    while (adj_inst_ptr < 0) adj_inst_ptr += memory.GetSize() + 1;
    adj_inst_ptr %= memory.GetSize() + 1;

    if (adj_inst_ptr == memory.GetSize()) {
      main_window->HLine(MEMORY_Y + 7 + i, MEMORY_X, 16);
    } else {
      main_window->Print(MEMORY_Y + 7 + i, MEMORY_X, CHAR_VLINE);
      main_window->Print(MEMORY_Y + 7 + i, MEMORY_X + 15, CHAR_VLINE);

      // If we are on the instruction about to be executed by the CPU,
      // hilight it...
      if (adj_inst_ptr == base_inst_ptr) {
	main_window->SetBoldColor(COLOR_WHITE);
	main_window->Print(MEMORY_Y + 7 + i, MEMORY_X + 1, "%3d: ",
		  adj_inst_ptr);
	main_window->SetBoldColor(COLOR_CYAN);
      } else {
	main_window->Print(MEMORY_Y + 7 + i, MEMORY_X + 1, "%3d: ",
		  adj_inst_ptr);
	main_window->SetColor(COLOR_CYAN);
      }

      main_window->Print(MEMORY_Y + 7 + i, MEMORY_X + 6, "%8s ",
		inst_lib->name[memory.Get(adj_inst_ptr)]());
      main_window->SetColor(COLOR_WHITE);
    }
  }
  // main_window->Print(MEMORY_Y + 5, MEMORY_X + 15, "<");

  main_window->Refresh();
}

void cAAView::DoInputs()
{
  // See if we are in step-wise mode, and if so, "finish" this step!

  if (pause_level == PAUSE_ADVANCE_STEP) {
    pause_level = PAUSE_ON;
    if (active_cpu) active_cpu->SetInstLib(population->GetInstLib());
  }

  cView::DoInputs();
}


void cAAView::MapInput(int in_char)
{
  switch (in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    break;
  }
}

void cAAView::HistInput(int in_char)
{
  switch(in_char) {
  case '<':
  case ',':
    ++hist_mode %= NUM_HIST;
    UpdateHistWindow();
    break;
  case '>':
  case '.':
    hist_mode += NUM_HIST;
    --hist_mode %= NUM_HIST;
    UpdateHistWindow();
    break;
  default:
    cView::StepInput(in_char);
    break;
  }
}

void cAAView::StepInput(int in_char)
{
  switch(in_char) {
  case '-':
  case '_':
    memory_offset--;
    memory_offset %= active_cpu->GetMemorySize();
    UpdateStepWindow();
    break;
  case '+':
  case '=':
    memory_offset++;
    memory_offset %= active_cpu->GetMemorySize();
    UpdateStepWindow();
    break;
  case 'x':
  case 'X':
    // Toggle the parasite_zoom flag...
    parasite_zoom = !parasite_zoom;
    UpdateStepWindow();
    break;
  default:
    cView::StepInput(in_char);
    break;
  }
}

void cAAView::OptionsInput(int in_char)
{
  switch (in_char) {
  default:
    break;
  }
}

void cAAView::EngageStepMode()
{
  if (pause_level == PAUSE_ON && active_cpu) {
    pause_level = PAUSE_ADVANCE_STEP;
    active_cpu->SetInstLib(alt_inst_lib);
  }
}

void cAAView::DisEngageStepMode()
{
  active_cpu->SetInstLib(population->GetInstLib());
}

void cAAView::NavigateMapWindow()
{
  // VIRTUAL FUNCTION
}

void cAAView::ExtractCreature()
{
  cGenotype * cur_gen = active_cpu->GetActiveGenotype();
  cString gen_name = cur_gen->GetName();

  if (gen_name == "(no name)")
    gen_name.Set("%03d-unnamed", cur_gen->GetLength());

  main_window->Print(20, 0, "Extracting %s...", gen_name());

  cBaseCPU test_cpu(TRUE);
  test_cpu.ChangeGenotype(cur_gen);
  test_cpu.TestProcess();
  test_cpu.Print(gen_name());

  main_window->Print(20, 24, "Done.");
  main_window->Refresh();
}


////////////////////
//  cTierraView
////////////////////

cTierraView::cTierraView(cTierraPopulation * in_population)
{
  population = in_population;
  genebank = population->GetGenebank();
  aa_pop = (cAutoAdaptivePopulation *) in_population;
  active_cpu = aa_pop->GetCPU(0);
  alt_inst_lib = new cInstLib(population->GetInstLib());
  alt_inst_lib->ActivateAltFunctions();
  DrawBarWindow();
}

void cTierraView::DrawBarWindow()
{
  cView::DrawBarWindow();

  bar_window->SetBoldColor(COLOR_WHITE);
  bar_window->Print(1, bar_window->Width() - 10, "Tierra");
  bar_window->SetColor(COLOR_WHITE);
}

void cTierraView::DrawMapWindow()
{
  // @CAO Fill out later...
}

void cTierraView::DrawOptionsWindow()
{
  main_window->Print(6, 15, "%d", population->GetSize());
  cAAView::DrawOptionsWindow();
}

void cTierraView::UpdateMapWindow()
{
  // @CAO Fill out later...
}

void cTierraView::UpdateOptionsWindow()
{
  cAAView::UpdateOptionsWindow();
}

void cTierraView::UpdateStepWindow()
{
  cAAView::UpdateStepWindow();
}

void cTierraView::MapInput(int in_char)
{
  switch (in_char) {
  case 0: // Keep Visual C compiler from spitting warnings about no cases...
  default:
    break;
  }
}

void cTierraView::NavigateMapWindow()
{
}


////////////////////
//  cAvidaView
////////////////////

cAvidaView::cAvidaView(cAvidaPopulation * in_population)
{
  population = in_population;
  genebank = population->GetGenebank();
  av_pop = in_population;
  aa_pop = (cAutoAdaptivePopulation *) in_population;
  active_cpu = aa_pop->GetCPU(0);
  alt_inst_lib = new cInstLib(population->GetInstLib());
  alt_inst_lib->ActivateAltFunctions();
  map_x_size = av_pop->GetWorldX();
  map_y_size = av_pop->GetWorldY();

  CenterActiveCPU();

  DrawBarWindow();
}

void cAvidaView::DrawBarWindow()
{
  cView::DrawBarWindow();

  bar_window->SetBoldColor(COLOR_WHITE);
  bar_window->Print(1, bar_window->Width() - 10, "Avida");
  bar_window->SetColor(COLOR_WHITE);
}

void cAvidaView::DrawMapWindow()
{
  UpdateMapWindow();
}

void cAvidaView::DrawMapWindow(char *szMoreInfo)
{
  UpdateMapWindow(szMoreInfo);
}

void cAvidaView::DrawOptionsWindow()
{
  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(6, 15, "%dx%d",
	    av_pop->GetWorldX(), av_pop->GetWorldY());
  main_window->SetColor(COLOR_WHITE);
  cAAView::DrawOptionsWindow();
}

void cAvidaView::UpdateMapWindow(char *szMoreInfo)
{
  UpdateMapWindow();	// display the standard map window
  if (szMoreInfo) {
    // additional info to explain this map has been passed in... show it
    main_window->Print(main_window->Height() - 1, 0, szMoreInfo);
  }
  main_window->Refresh();
}

void cAvidaView::UpdateMapWindow()
{
  char * map = NULL;
  char * color_map = NULL;
  cAvidaPopulation * av_pop = (cAvidaPopulation *) population;
  int x, y, cur_x, cur_y, virtual_x, virtual_y, index;

  main_window->SetBoldColor(COLOR_WHITE);

  switch (map_mode) {
  case MAP_BASIC:
    if (main_window->HasColors()) color_map = av_pop->GetBasicGrid();
    else map = av_pop->GetBasicGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      " Genotype View ");
    break;
  case MAP_SPECIES:
    if (main_window->HasColors()) color_map = av_pop->GetSpeciesGrid();
    else map = av_pop->GetSpeciesGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      " Species View  ");
    break;
  case MAP_COMBO:
    color_map = av_pop->GetBasicGrid();
    map = av_pop->GetSpeciesGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      "  Combo View   ");
    break;
  case MAP_INJECT:
    map = av_pop->GetInjectGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      "  Inject View  ");
    break;
  case MAP_RESOURCE:
    map = av_pop->GetResourceGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      " Resource View ");
    break;
  case MAP_AGE:
    map = av_pop->GetAgeGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      "   Age View    ");
    break;
  case MAP_BREED_TRUE:
    if (main_window->HasColors()) color_map = av_pop->GetBreedGrid();
    else map = av_pop->GetBreedGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      "Breed True View");
    break;
  case MAP_PARASITE:
    if (main_window->HasColors()) color_map = av_pop->GetParasiteGrid();
    else map = av_pop->GetParasiteGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      " Parasite View ");
    break;
  case MAP_POINT_MUT:
    if (main_window->HasColors()) color_map = av_pop->GetPointMutGrid();
    else map = av_pop->GetPointMutGrid();
    main_window->Print(main_window->Height() - 1, 60,
	      " PointMut View ");
    break;
  }

  main_window->Print(main_window->Height() - 1, 56, "[ ]");
  main_window->Print(main_window->Height() - 1, 76, "[ ]");

  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(main_window->Height() - 1, 57, '<');
  main_window->Print(main_window->Height() - 1, 77, '>');
  main_window->SetColor(COLOR_WHITE);

  if (map_y_size >= main_window->Height()) {
    // Map is too big for the display window (since last line is reserved for
    // the display mode text). Tell users how much is clipped.
    main_window->Print(main_window->Height() - 1, 0,
		       "* Clipping last %i line(s) *",
		       map_y_size - main_window->Height() + 1);
  }

  virtual_x = corner_id % map_x_size;
  virtual_y = corner_id / map_x_size;

  for (y = 0; y < main_window->Height() - 1 && y < map_y_size; y++) {
    main_window->Move(y, 0);
    cur_y = (y + virtual_y) % map_y_size;
    for (x = 0; (AVIDA_MAP_X_SPACING * x) < main_window->Width() - 1 && x < map_x_size; x++) {
      cur_x = (x + virtual_x) % map_x_size;
      index = cur_y * map_x_size + cur_x;
      if (color_map) {
	// If this spot has no color, skip over it!
	if (color_map[index] == ' ') {
	  main_window->Print("  ");
	  continue;
	}
	// Otherwise, set the proper color.
	SetMainColor(color_map[index]);
      }
      if (map) main_window->Print(map[index]);
      else main_window->Print(CHAR_BULLET);

      // Skip spaces before the next map symbol
      for (int i = 0; i < AVIDA_MAP_X_SPACING - 1; i++)
	main_window->Print(' ');
    }
  }
  main_window->SetColor(COLOR_WHITE);

  main_window->Refresh();
  if (map) g_memory.Free(map, av_pop->GetSize());
  if (color_map) g_memory.Free(color_map, av_pop->GetSize());
}

void cAvidaView::UpdateOptionsWindow()
{
  if (!active_cpu) active_cpu = aa_pop->GetCPU(0);
  if (!active_cpu->GetActiveGenotype()) return;

  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(1, 15, "(%d, %d)",
	    active_cpu->GetEnvironment()->GetID() % av_pop->GetWorldX(),
	    active_cpu->GetEnvironment()->GetID() / av_pop->GetWorldY());
  cAAView::UpdateOptionsWindow();
}

void cAvidaView::UpdateStepWindow()
{
  if (!active_cpu) active_cpu = aa_pop->GetCPU(0);
  main_window->SetBoldColor(COLOR_WHITE);

  // Figure out which CPU we're pointing at (useful for watching parasites)
  int id = active_cpu->GetEnvironment()->GetFacing()->GetEnvironment()->GetID();
  if (active_cpu->GetInstPointer().TestParasite()) {
    // We are in a parasite...
    main_window->Print(15, 0, "Executing...:");
    id = active_cpu->GetInstPointer().GetCurID();
  } else {
    // Not parasitic...
    main_window->Print(15, 0, "Facing......:");
    
  }

  // Show the location of the CPU we are looking at.
  main_window->SetBoldColor(COLOR_CYAN);
  main_window->Print(1, 14, "(%d, %d)    ",
		     active_cpu->GetEnvironment()->GetID() % av_pop->GetWorldX(),
		     active_cpu->GetEnvironment()->GetID() / av_pop->GetWorldY());

  // Now show the location of the CPU we are facing/executing.
  main_window->Print(15, 14, "(%2d, %2d)    ", id % av_pop->GetWorldX(),
		id / av_pop->GetWorldY());

  DrawMiniMap();
  if (active_cpu->GetActiveGenotype())
    // Can't update info on the creature's code if this CPU has no creature!
    cAAView::UpdateStepWindow();
  else
    main_window->Refresh();	// UpdateStepWindow normally does this
}

void cAvidaView::MapInput(int in_char)
{
  switch(in_char) {
  case '2':
  case KEY_DOWN:
    corner_id += map_x_size;
    corner_id %= population->GetSize();
    UpdateMapWindow();
    break;
  case '8':
  case KEY_UP:
    corner_id -= map_x_size;
    if (corner_id < 0) corner_id += population->GetSize();
    UpdateMapWindow();
    break;
  case '6':
  case KEY_RIGHT:
    corner_id++;
    if (corner_id == population->GetSize()) corner_id = 0;
    UpdateMapWindow();
    break;
  case '4':
  case KEY_LEFT:
    corner_id--;
    if (corner_id < 0) corner_id += population->GetSize();
    UpdateMapWindow();
    break;
  case '>':
  case '.':
    ++map_mode %= NUM_MAPS;
    UpdateMapWindow();
    break;
  case '<':
  case ',':
    map_mode += NUM_MAPS;
    --map_mode %= NUM_MAPS;
    UpdateMapWindow();
    break;
  }
}

void cAvidaView::StepInput(int in_char)
{
  switch(in_char) {
  case '2':
  case KEY_DOWN:
    mini_center_id += map_x_size;
    mini_center_id %= population->GetSize();
    UpdateStepWindow();
    break;
  case '8':
  case KEY_UP:
    mini_center_id -= map_x_size;
    if (mini_center_id < 0) mini_center_id += population->GetSize();
    UpdateStepWindow();
    break;
  case '6':
  case KEY_RIGHT:
    mini_center_id++;
    if (mini_center_id == population->GetSize()) mini_center_id = 0;
    UpdateStepWindow();
    break;
  case '4':
  case KEY_LEFT:
    mini_center_id--;
    if (mini_center_id < 0) mini_center_id += population->GetSize();
    UpdateStepWindow();
    break;
  case '>':
  case '.':
    ++map_mode %= NUM_MAPS;
    UpdateStepWindow();
    break;
  case '<':
  case ',':
    map_mode += NUM_MAPS;
    --map_mode %= NUM_MAPS;
    UpdateStepWindow();
    break;
  case '\n':
  case '\r':
    active_cpu = av_pop->GetCPU(mini_center_id);
    UpdateStepWindow();
    break;
  default:
    cAAView::StepInput(in_char);
    break;
  }
}


void cAvidaView::DrawMiniMap()
{
  char * map = NULL;
  char * color_map = NULL;
  cAvidaPopulation * av_pop = (cAvidaPopulation *) population;
  int x, y, cur_x, cur_y, virtual_x, virtual_y, index;

  // Setup the start color for the map...
  main_window->SetColor(COLOR_WHITE);

  switch (map_mode) {
  case MAP_BASIC:
    if (main_window->HasColors()) color_map = av_pop->GetBasicGrid();
    else map = av_pop->GetBasicGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "Genotypes");
    break;
  case MAP_SPECIES:
    if (main_window->HasColors()) color_map = av_pop->GetSpeciesGrid();
    else map = av_pop->GetSpeciesGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      " Species ");
    break;
  case MAP_COMBO:
    color_map = av_pop->GetBasicGrid();
    map = av_pop->GetSpeciesGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "  Combo  ");
    break;
  case MAP_INJECT:
    map = av_pop->GetInjectGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      " Inject  ");
    break;
  case MAP_RESOURCE:
    map = av_pop->GetResourceGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "Resources");
    break;
  case MAP_AGE:
    map = av_pop->GetAgeGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "   Age   ");
    break;
  case MAP_BREED_TRUE:
    if (main_window->HasColors()) color_map = av_pop->GetBreedGrid();
    else map = av_pop->GetBreedGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "BreedTrue");
    break;
  case MAP_PARASITE:
    if (main_window->HasColors()) color_map = av_pop->GetParasiteGrid();
    else map = av_pop->GetParasiteGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "Parasite ");
    break;
  case MAP_POINT_MUT:
    if (main_window->HasColors()) color_map = av_pop->GetPointMutGrid();
    else map = av_pop->GetPointMutGrid();
    main_window->Print(MINI_MAP_Y + 8, MINI_MAP_X + 5,
	      "Point Mut");
    break;
  }

  virtual_x = (mini_center_id % map_x_size) + map_x_size;
  virtual_y = (mini_center_id / map_x_size) + map_y_size;

  for (y = -3; y <= 3 && y < map_y_size - 3; y++) {
    main_window->Move(MINI_MAP_Y + 4 + y, MINI_MAP_X + 3);
    cur_y = (y + virtual_y) % map_y_size;
    for (x = -3; x <= 3 && x < map_x_size - 3; x++) {
      cur_x = (x + virtual_x) % map_x_size;
      index = cur_y * map_x_size + cur_x;
      if (color_map) SetMainColor(color_map[index]);
      if (map) main_window->Print(map[index]);
      else main_window->Print(CHAR_BULLET);

      // Space between columns
      for (int i = 0; i < AVIDA_MAP_X_SPACING - 1; i++)
	main_window->Print(' ');
    }
  }
  main_window->SetColor(COLOR_WHITE);
  main_window->Print(MINI_MAP_Y + 4, MINI_MAP_X + 8, '[');
  main_window->Print(MINI_MAP_Y + 4, MINI_MAP_X + 10, ']');

  main_window->Refresh();
  if (map) g_memory.Free(map, av_pop->GetSize());
  if (color_map) g_memory.Free(color_map, av_pop->GetSize());  
}

void cAvidaView::NavigateMapWindow()
{
  // Setup for choosing a cpu...

  CenterActiveCPU();
  ClearMainWindow();
  DrawMapWindow("Choose a CPU and press ENTER");
  PlaceMapCursor();

  cBaseCPU * old_cpu = active_cpu;
  int temp_cpu_id;

  int finished = FALSE;
  int cur_char;
  int iXMove, iYMove;

#ifdef VIEW_ANSI
  MOUSE_INFO * pmiData; // For mouse input (see ANSI.HH)
  // Map dim's run from 0 to iXMax and 0 to iYMax; use this to regulate
  // cursor position

  int iXMax = min(map_x_size*AVIDA_MAP_X_SPACING, main_window->Width()) - 1;
  int iYMax = min(map_y_size, main_window->Height()-1) - 1;
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
      active_cpu = old_cpu;
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
	  main_window->IsCoordInWindow (pmiData->X, pmiData->Y)) {
	// deal with MOUSE_DBLCLICK and MOUSE_CLICK input.  Single click
	// moves cursor.  Dbl click selects CPU and runs '\r' code. 

	// We already know the click occured within our window
	if (pmiData->iEvent & (MOUSE_DBLCLICK || MOUSE_CLICK)) {
	  // Reposition cursor on single or dbl click. Pin the cursor
	  // movement within the map region.
	  iXMove = min (iXMax, main_window->ScreenXToWindow(pmiData->X)) - 
	    main_window->CursorX();
	  iYMove = min (iYMax, main_window->ScreenYToWindow(pmiData->Y)) - 
	    main_window->CursorY();
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
    temp_cpu_id = active_cpu->GetEnvironment()->GetID();
    if (iXMove) {
      int i = temp_cpu_id % map_x_size; // column we are in
      temp_cpu_id -= i; // CPU id, less column
      i += iXMove;
      while (i >= map_x_size) i -= map_x_size;
      while (i < 0) i += map_x_size;
      temp_cpu_id += i; // CPU id readjusted to new column
    }
    if (iYMove) {
      temp_cpu_id += map_x_size * iYMove; 
      while (temp_cpu_id >= aa_pop->GetSize())
	temp_cpu_id -= aa_pop->GetSize();
      while (temp_cpu_id < 0) temp_cpu_id += aa_pop->GetSize();
    }
    if (iXMove || iYMove) {
      active_cpu = aa_pop->GetCPU(temp_cpu_id);
      PlaceMapCursor();
    }
  } // End of WHILE
  
#ifdef VIEW_ANSI
  // Turn off mouse events, flush the queue and we are finished
  EnableMouseEvents (FALSE);
  FlushMouseQueue();
#endif

  CenterActiveCPU();	// Set map w/ active CPU in the center
  ClearMainWindow();
}

void cAvidaView::CenterActiveCPU()
{
  // first find the map position of the active cpu
  int active_x = active_cpu->GetEnvironment()->GetID() % map_x_size;
  int active_y = active_cpu->GetEnvironment()->GetID() / map_x_size;
  
  // Now find the relative position of the corner to adjust for this
  int corner_x = active_x - main_window->Width() / (2 * AVIDA_MAP_X_SPACING);
  int corner_y = active_y - main_window->Height() / 2;

  // Make sure the corner co-ordinates are positive
  if (corner_x < 0) corner_x = (corner_x % map_x_size) + map_x_size;
  if (corner_y < 0) corner_y = (corner_y % map_y_size) + map_y_size;

  // Now, find the ID for these corner co-ordinates
  corner_id = corner_y * map_x_size + corner_x;

  // Finally record for the center ID.
  mini_center_id = active_cpu->GetEnvironment()->GetID();
}

void cAvidaView::CenterXCoord()
{
  // first find the map position of the active cpu
  int active_x = active_cpu->GetEnvironment()->GetID() % map_x_size;
  
  // Now find the relative position of the corner to adjust for this
  int corner_x = active_x - main_window->Width() / (2 * AVIDA_MAP_X_SPACING);
  int corner_y = corner_id / map_x_size;
  
  // Make sure the corner co-ordinates are positive
  if (corner_x < 0) corner_x = (corner_x % map_x_size) + map_x_size;

  // finally find the ID for these corner co-ordinates
  corner_id = corner_y * map_x_size + corner_x;
}

void cAvidaView::CenterYCoord()
{
  // first find the map position of the active cpu
  int active_y = active_cpu->GetEnvironment()->GetID() / map_x_size;
  
  // Now find the relative position of the corner to adjust for this
  int corner_x = corner_id % map_x_size;
  int corner_y = active_y - main_window->Height() / 2;

  // Make sure the corner co-ordinates are positive
  if (corner_y < 0) corner_y = (corner_y % map_y_size) + map_y_size;

  // finally find the ID for these corner co-ordinates
  corner_id = corner_y * map_x_size + corner_x;
}

void cAvidaView::PlaceMapCursor()
{
  int x_offset = active_cpu->GetEnvironment()->GetID() - corner_id;
  x_offset %= map_x_size;
  if (x_offset < 0) x_offset += map_x_size;

  int y_offset = (active_cpu->GetEnvironment()->GetID() / map_x_size) - (corner_id / map_x_size);
  if (y_offset < 0) y_offset += map_y_size;

  cGenotype * cpu_gen = active_cpu->GetActiveGenotype();

  if (!cpu_gen) {
    main_window->Print(main_window->Height() - 1, 33,
	      "(%2d, %2d) - (empty)  ", active_cpu->GetEnvironment()->GetID() % map_x_size,
	      active_cpu->GetEnvironment()->GetID() / map_x_size);
  } else {
    main_window->Print(main_window->Height() - 1, 33, "(%2d, %2d) - %s",
		       active_cpu->GetEnvironment()->GetID() % map_x_size,
		       active_cpu->GetEnvironment()->GetID() / map_x_size,
		       cpu_gen->GetName()());
  }
  
  if (x_offset == 0 || x_offset == main_window->Width()/2 - 1) {
    CenterXCoord();
    ClearMainWindow();
    DrawMapWindow();
    PlaceMapCursor();
  }
  else if (y_offset == 0 || y_offset == main_window->Height() - 2) {
    CenterYCoord();
    ClearMainWindow();
    DrawMapWindow();
    PlaceMapCursor();
  }
  else {
    main_window->Move(y_offset, x_offset * 2);
    main_window->Refresh();
  }
}


////////////////////
//  cPlaybackView
////////////////////

cPlaybackView::cPlaybackView(cPlaybackPopulation * in_population)
{
  view_mode = MODE_MAP;
  population = in_population;
  genebank = population->GetGenebank();
  pb_genebank = population->GetPBGenebank();

  map_x_size = ((cPlaybackPopulation *) population)->GetWorldX();
  map_y_size = ((cPlaybackPopulation *) population)->GetWorldY();

  // Now find the relative position of the corner to adjust for this
  int corner_x = - main_window->Width() / (2 * AVIDA_MAP_X_SPACING);
  int corner_y = - main_window->Height() / 2;

  // Make sure the corner co-ordinates are positive
  if (corner_x < 0) corner_x = (corner_x % map_x_size) + map_x_size;
  if (corner_y < 0) corner_y = (corner_y % map_y_size) + map_y_size;

  // finally find the ID for these corner co-ordinates
  corner_id = corner_y * map_x_size + corner_x;

  DrawBarWindow();
}

void cPlaybackView::DrawBarWindow()
{
  cView::DrawBarWindow();
  bar_window->Print(1, bar_window->Width() - 11, "Playback");
}

void cPlaybackView::DrawMapWindow()
{
  // Not much to do here...
  UpdateMapWindow();
}

void cPlaybackView::UpdateMapWindow()
{
  char * map = NULL;
  cPlaybackPopulation * pb_pop = (cPlaybackPopulation *) population;

  int x, y, cur_x, cur_y, virtual_x, virtual_y, index;

  virtual_x = corner_id % map_x_size;
  virtual_y = corner_id / map_x_size;

  map = pb_pop->GetBasicGrid();

  for (y = 0; y < main_window->Height() - 1 && y < map_y_size; y++) {
    main_window->Move(y, 0);
    cur_y = (y + virtual_y) % map_y_size;
    for (x = 0; (AVIDA_MAP_X_SPACING * x) < main_window->Width() - 1 &&
	 x < map_x_size; x++) {
      cur_x = (x + virtual_x) % map_x_size;
      index = cur_y * map_x_size + cur_x;
      main_window->Print(map[index]);
      main_window->Print(' ');
    }
  }

  main_window->Refresh();
  g_memory.Free(map, map_x_size * map_y_size);
}

void cPlaybackView::UpdateSymbols()
{
  pb_genebank->AssignSymbols();
}

//////////////////
//  cMenuWindow
//////////////////

cMenuWindow::cMenuWindow(int menu_size)
{
  window = NULL;
  option_list = new cString[menu_size];
  num_options = menu_size;
  active_id = 0;
}

cMenuWindow::~cMenuWindow()
{
  if (window) delete window;
  delete [] option_list;
}

void cMenuWindow::AddOption(int option_id, char * in_option)
{
  option_list[option_id] = in_option;
}

int cMenuWindow::Activate()
{
  int i;

  // Calculate number of columns needed and the width of each

  max_width = 0;
  for (i = 0; i < num_options; i++) {
    if (max_width < option_list[i].GetSize())
      max_width = option_list[i].GetSize();
  }

  // @CAO should make these more flexible.
  int max_lines = 20;
  // int max_cols = 80 / (max_width + 3);
  
  cols = num_options / max_lines + 1;
  lines = num_options / cols;
  if (lines * cols < num_options) lines++;  // In case of roundoff error.

  // Create and display the window.
  int win_height = lines + 4;
  int win_width = (max_width + 2) * cols + 2;
  window = new cTextWindow(win_height, win_width,
			   (23 - win_height) / 2, (80 - win_width) / 2);
  window->Box();

  window->SetColor(COLOR_CYAN);
  for (i = 0; i < num_options; i++) {
    DrawOption(i);
  }
  window->SetBoldColor(COLOR_CYAN);
  DrawOption(active_id);
  window->Refresh();
  
  // Wait for the results.
  int finished = FALSE;
  int cur_char;

  while (!finished) {
    cur_char = GetInput();
    switch (cur_char) {
    case 'q':
    case 'Q':
      // Abort!
      finished = TRUE;
      active_id = -1;
      break;
    case ' ':
    case '\n':
    case '\r':
      // Select current entry.
      finished = TRUE;
      break;
    case KEY_UP:
      MoveActiveID(active_id - 1);
      break;
    case KEY_DOWN:
      MoveActiveID(active_id + 1);
      break;
    case KEY_LEFT:
      MoveActiveID(active_id - lines);
      break;
    case KEY_RIGHT:
      MoveActiveID(active_id + lines);
      break;
    case KEY_HOME:
      MoveActiveID(0);
      break;
#ifdef KEY_END
    case KEY_END:
      MoveActiveID(num_options - 1);
      break;
#endif
    }
  }

  // Delete the window and return the results.
  delete window;
  window = NULL;

  return active_id;
}

void cMenuWindow::DrawOption(int option_id)
{
  int line_id = option_id % lines;
  int col_id  = option_id / lines;
  window->Print(line_id + 2, col_id * (max_width + 2) + 2,
	    option_list[option_id]());
}

void cMenuWindow::MoveActiveID(int new_id)
{
  // Don't do anything if we are not really moving, or else trying to
  // move out of range.
  if (active_id == new_id || new_id < 0 || new_id >= num_options) return;

  // Dull out old active ID.
  window->SetColor(COLOR_CYAN);
  DrawOption(active_id);

  // Reset the active ID.
  active_id = new_id;
  // while (active_id < 0) active_id += num_options;
  // active_id %= num_options;

  // Hi-lite the new ID.
  window->SetBoldColor(COLOR_CYAN);
  DrawOption(active_id);

  // Refresh the screen.
  window->Refresh();
}

#endif
