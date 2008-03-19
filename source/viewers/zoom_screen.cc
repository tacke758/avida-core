//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "zoom_screen.hh"
#include "text.hh"
#include "menu.hh"

#ifdef VIEW_TEXT

/////////////////////
//  The Zoom Screen
/////////////////////

cZoomScreen::cZoomScreen(int y_size, int x_size, int y_start, int x_start,
			 cViewInfo & in_info, cPopulation & in_pop)
  : cScreen(y_size, x_size, y_start, x_start, in_info), population(in_pop)
{
  memory_offset = 0;
  parasite_zoom = FALSE;
  mode = ZOOM_MODE_STATS;
  map_mode = MAP_BASIC;
  inst_view_mode = TRUE;
  active_section = ZOOM_SECTION_MEMORY;
  task_offset = 0;

  map_mode = MAP_BASIC;
  mini_center_id = 0;
  map_x_size = population.GetWorldX();
  map_y_size = population.GetWorldY();
}




void cZoomScreen::Draw()
{
  // Draw the options block which is on all screens.

  SetBoldColor(COLOR_WHITE);
  Print(OPTIONS_Y, OPTIONS_X, "[<]                [>]");
  if (info.GetPauseLevel()) {
    Print(OPTIONS_Y+2, OPTIONS_X+2, "Un-[P]ause");
    Print(OPTIONS_Y+3, OPTIONS_X+2, "[N]ext Update");
    Print(OPTIONS_Y+4, OPTIONS_X+2, "[Space] Next Inst");
  } else {
    Print(OPTIONS_Y+2, OPTIONS_X+2, "[P]ause   ");
    Print(OPTIONS_Y+3, OPTIONS_X+2, "             ");
    Print(OPTIONS_Y+4, OPTIONS_X+2, "                 ");
  }

  SetBoldColor(COLOR_CYAN);
  Print(OPTIONS_Y, OPTIONS_X+1, "<");
  Print(OPTIONS_Y, OPTIONS_X+20, ">");
  if (info.GetPauseLevel()) {
    Print(OPTIONS_Y+2, OPTIONS_X+6, "P");
    Print(OPTIONS_Y+3, OPTIONS_X+3, "N");
    Print(OPTIONS_Y+4, OPTIONS_X+3, "Space");
  } else {
    Print(OPTIONS_Y+2, OPTIONS_X+3, "P");
  }

  // Redirect to the proper Draw() method.

  if (mode == ZOOM_MODE_CPU) DrawCPU();
  else if (mode == ZOOM_MODE_STATS) DrawStats();
  else if (mode == ZOOM_MODE_GENOTYPE) DrawGenotype();

  Update();
  Refresh();
}

void cZoomScreen::DrawStats()
{
  // Creature Status

  SetBoldColor(COLOR_WHITE);

  Print(1, 0, "GenotypeID:");
  Print(2, 0, "Geno Name.:");
  Print(3, 0, "Species.ID:");

  Print(5, 0, "Fitness...:");
  Print(6, 0, "Gestation.:");
  Print(7, 0, "CPU Speed.:");
  Print(8, 0, "Cur Merit.:");
  Print(9, 0, "GenomeSize:");
  Print(10, 0, "Mem Size..:");
  Print(11, 0, "Faults....:");

  Print(1,  27, "Location..:");

  Print(4,  27, "Generation:");
  Print(5,  27, "Age.......:");
  Print(6,  27, "Executed..:");
  Print(7,  27, "LastDivide:");
  Print(8,  27, "Offspring.:");

  Print(10, 27, "Thread:");
  Print(11, 27, "IP....:");
  Print(12, 27, "AX....:");
  Print(13, 27, "BX....:");
  Print(14, 27, "CX....:");
  Print(15, 27, "Stack.:");
  Print(16, 27, "---- Memory ----");

  Print(CPU_FLAGS_Y, CPU_FLAGS_X, "---- Flags ----");

  // --== Options ==--
  SetBoldColor(COLOR_YELLOW);
  Print(OPTIONS_Y, OPTIONS_X+4, "CPU Stats Zoom");

  // --== Tasks ==--
  SetColor(COLOR_WHITE); 
  Box(TASK_X, TASK_Y, 30, Height()-TASK_Y);
  

  int task_num = task_offset;
  int col_num = 0;
  for (int cur_col = TASK_X + 2;
       task_num < cConfig::GetNumTasks();
       cur_col += 14) {
    for (int cur_row = TASK_Y + 1;
	 cur_row < Height() - 1 && task_num < cConfig::GetNumTasks();
	 cur_row++) {
      Print(cur_row, cur_col, "........:");
      Print(cur_row, cur_col, "%s", 
	    cConfig::GetDefaultTaskLib()->GetTaskName(task_num)());
      task_num++;
    }
    col_num++;
    if (col_num == 2) break;
  }

  if (task_num < cConfig::GetNumTasks() || task_offset != 0) {
    SetBoldColor(COLOR_WHITE);
    Print(Height()-1, Width() - 23, " [<-] More [->] ");
    SetBoldColor(COLOR_CYAN);
    Print(Height()-1, Width() - 21, "<-");
    Print(Height()-1, Width() - 11, "->");
  }


}

void cZoomScreen::DrawCPU()
{
  SetColor(COLOR_WHITE);

  // --== Registers ==--
  Box(REG_X, REG_Y, 19, 7); 
  Print(REG_Y + 1, REG_X + 2, "Registers");
  HLine(REG_Y + 2, REG_X, 19);

  SetBoldColor(COLOR_WHITE);
  Print(REG_Y + 3, REG_X + 2, "AX:");
  Print(REG_Y + 4, REG_X + 2, "BX:");
  Print(REG_Y + 5, REG_X + 2, "CX:");
  SetColor(COLOR_WHITE);

  // --== Inputs ==--
  Box(INPUT_X, INPUT_Y, 16, 7);
  Print(INPUT_Y + 1, INPUT_X + 2, "Inputs");
  HLine(INPUT_Y + 2, INPUT_X, 16);

  // --== Mini-Map ==--
  Box(MINI_MAP_X, MINI_MAP_Y, 17, 3);
  Print(MINI_MAP_Y + 1, MINI_MAP_X + 2, "Mini-Map");
  // HLine(MINI_MAP_Y + 2, MINI_MAP_X, 19);

  SetBoldColor(COLOR_WHITE);
  Print(MINI_MAP_Y + 11, MINI_MAP_X,  "[-]           [+]");
  SetBoldColor(COLOR_CYAN);
  Print(MINI_MAP_Y + 11, MINI_MAP_X + 1,  '-');
  Print(MINI_MAP_Y + 11, MINI_MAP_X + 15, '+');
  SetColor(COLOR_WHITE);

  // --== Memory ==--
  Box(MEMORY_X, MEMORY_Y, 36, 5 + MEMORY_PRE_SIZE + MEMORY_POST_SIZE);
  Print(MEMORY_Y + 1, MEMORY_X + 2,  "Memory:");
  HLine(MEMORY_Y + 2, MEMORY_X, 36);

  // --== Stack ==--
  Box(STACK_X, STACK_Y, 15, 7);
  HLine(STACK_Y + 2, STACK_X, 15);

  // --== Options ==--
  SetBoldColor(COLOR_YELLOW);
  Print(OPTIONS_Y, OPTIONS_X+4,    "Component Zoom");
  SetBoldColor(COLOR_WHITE);

  Print(OPTIONS_Y+6, OPTIONS_X+2, "[E]dit Component");
  Print(OPTIONS_Y+7, OPTIONS_X+2, "[V]iew Component");
#ifdef THREADS
  Print(OPTIONS_Y+8, OPTIONS_X+2, "[T]hread Options");
#endif
  Print(OPTIONS_Y+9, OPTIONS_X+2, "[TAB] Shift Active");

  if (info.GetPauseLevel()) {
    Print(OPTIONS_Y+10, OPTIONS_X+2, "[Arrows] Scroll");
  } else {
    Print(OPTIONS_Y+10, OPTIONS_X+2, "               ");
  }


  SetBoldColor(COLOR_CYAN);
  Print(OPTIONS_Y+6, OPTIONS_X+3, "E");
  Print(OPTIONS_Y+7, OPTIONS_X+3, "V");
#ifdef THREADS
  Print(OPTIONS_Y+8, OPTIONS_X+3, "T");
#endif
  Print(OPTIONS_Y+9, OPTIONS_X+3, "TAB");

  if (info.GetPauseLevel()) {
    Print(OPTIONS_Y+10, OPTIONS_X+3, "Arrows");
  }

  // Highlight the active section...
  SetActiveSection(active_section);

  // Add on a bunch of special characters to smooth the view out... 
  Print(INPUT_Y,     INPUT_X, CHAR_TTEE);
  Print(INPUT_Y + 2, INPUT_X, CHAR_PLUS);
  Print(INPUT_Y + 6, INPUT_X, CHAR_BTEE);

  Print(STACK_Y,     STACK_X, CHAR_TTEE);
  Print(STACK_Y + 2, STACK_X, CHAR_PLUS);
  Print(STACK_Y + 6, STACK_X, CHAR_BTEE);


  // A few stats on this screen...

  SetBoldColor(COLOR_WHITE);

  Print(13, 52, "Location.....:");
  Print(14, 52, "Genotype ID..:");
  Print(15, 52, "Genotype Name:");

  Print(17, 52, "Faults.......:");
  Print(18, 52, "Offspring....:");
  Print(19, 52, "Thread.......:");
}

void cZoomScreen::DrawGenotype()
{
  SetBoldColor(COLOR_YELLOW);
  Print(OPTIONS_Y, OPTIONS_X + 4, "Genotype Zoom");

  // Genotype status.

  SetBoldColor(COLOR_WHITE);

  Print(1, 0, "GenotypeID:");
  Print(2, 0, "Geno Name.:");
  Print(3, 0, "Species ID:");

  Print(5, 0, "Abundance.:");
  Print(6, 0, "Length....:");
  Print(7, 0, "CopyLength:");
  Print(8, 0, "Exe Length:");

  Print(10, 0, "Fitness...:");
  Print(11, 0, "Gestation.:");
  Print(12, 0, "Merit.....:");
  Print(13, 0, "ReproRate.:");

  Print(1, 27, "Update Born:");
  Print(2, 27, "Parent ID..:");
  Print(3, 27, "Parent Dist:");
  Print(4, 27, "Gene Depth.:");  

  Print(6, 27,  "-- This Update --");
  Print(7, 27,  "Deaths.....:");
  Print(8, 27,  "Divides....:");
  Print(9, 27,  "Breed True.:");
  Print(10, 27, "Breed In...:");
  Print(11, 27, "Breed Out..:");

  Print(13, 27, "-- Totals --");
  Print(14, 27, "Abundance..:");
  Print(15, 27, "Divides....:");
  Print(16, 27, "Breed True.:");
  Print(17, 27, "Breed In...:");
  Print(18, 27, "Breed Out..:");
}

void cZoomScreen::Update()
{
  if (mode == ZOOM_MODE_CPU) UpdateCPU();
  else if (mode == ZOOM_MODE_STATS) UpdateStats();
  else if (mode == ZOOM_MODE_GENOTYPE) UpdateGenotype();

  Refresh();
}

void cZoomScreen::UpdateStats()
{
  if (!info.GetActiveCPU()) return;
  cGenotype * genotype = info.GetActiveGenotype();

  SetBoldColor(COLOR_CYAN);

  // if there is an Active Genotype name AND it is not empty, show it
  Print(1, 12, "%9d", info.GetActiveGenotypeID());
  Print(2, 12, "%s", info.GetActiveName()());

  if (info.GetActiveSpecies())
    Print(3, 12, "%9d", info.GetActiveSpeciesID());
  else
    Print(3, 15, "(none) ");

  PrintFitness(5, 14, info.GetActiveCPU()->GetFitness());
  Print(6, 15, "%6d ", info.GetActiveCPU()->GetGestationTime());
  PrintMerit(7, 14, info.GetActiveCPU()->GetMerit());
  PrintMerit(8, 14, info.GetActiveCPU()->GetCurMerit());
  Print(9, 15, "%6d ", genotype ? genotype->GetLength() : 0);
  Print(10, 15, "%6d ", info.GetActiveCPU()->GetMemorySize());

  Print(11, 15, "%6d ", info.GetActiveCPU()->GetNumErrors());

  Print(4, 39, "%9d ", info.GetActiveCPU()->GetEnvironment()->GetGeneration());
  Print(5, 39, "%9d ", info.GetActiveCPU()->GetAge());
  Print(6, 39, "%9d ", info.GetActiveCPU()->GetTotalTimeUsed());
  Print(7, 39, "%9d ", info.GetActiveCPU()->GetGestationStart());
  Print(8, 39, "%9d ", info.GetActiveCPU()->GetNumDivides());


  if (info.GetThreadLock() != -1) Print(10, 36, "LOCKED");
  else Print(10, 36, "      ");
  Print(10, 43, "%2d/%2d",
	info.GetActiveCPU()->GetActiveThreadID() + 1,
	info.GetActiveCPU()->GetNumThreads());
  Print(12, 34, "%14d", info.GetActiveCPU()->GetRegister(0));
  Print(13, 34, "%14d", info.GetActiveCPU()->GetRegister(1));
  Print(14, 34, "%14d", info.GetActiveCPU()->GetRegister(2));
  Print(15, 34, "%14d", info.GetActiveCPU()->GetStack(0));

  cCPUHead inst_ptr(info.GetActiveCPU()->GetInstPointer());
  cInstLib * inst_lib = population.GetInstLib();
  
  for (int pos = 0; pos < 3; pos++) {
    // Clear the line
    Print(17+pos, 29, "                    ");
    if (inst_ptr.InMemory() == FALSE) continue;
    if (pos == 1) SetColor(COLOR_CYAN);

    // Draw the current instruction.
#ifdef INST_ARGS
    int num_args = inst_lib->GetNumArgs();

    if (num_args >= 2) {
      Print(17+pos, 29, "%s %s %s", inst_lib->GetName(inst_ptr.GetInst())(),
	    inst_ptr.GetInst().GetArg(0).AsString(inst_lib)(),
	    inst_ptr.GetInst().GetArg(1).AsString(inst_lib)() );
    } else if (num_args == 1) {
      Print(17+pos, 29, "%s %s", inst_lib->GetName(inst_ptr.GetInst())(),
	    inst_ptr.GetInst().GetArg(1).AsString(inst_lib)() );
    } else
#endif
      Print(17+pos, 29, "%s",	inst_lib->GetName(inst_ptr.GetInst())());
    inst_ptr.Advance();
  }

  // Flags...
  if (info.GetActiveCPU()->GetMalActive())
    SetBoldColor(COLOR_CYAN);
  else SetColor(COLOR_CYAN);
  Print(CPU_FLAGS_Y + 1, CPU_FLAGS_X + 1, "Mem Allocated");

  if (info.GetActiveCPU()->GetParentTrue())
    SetBoldColor(COLOR_CYAN);
  else SetColor(COLOR_CYAN);
  Print(CPU_FLAGS_Y + 2, CPU_FLAGS_X + 1, "Parent True");

  if (info.GetActiveCPU()->IsInjected())
    SetBoldColor(COLOR_CYAN);
  else SetColor(COLOR_CYAN);
  Print(CPU_FLAGS_Y + 3, CPU_FLAGS_X + 1, "Injected");

  if (info.GetActiveCPU()->IsParasite())
    SetBoldColor(COLOR_CYAN);
  else SetColor(COLOR_CYAN);
  Print(CPU_FLAGS_Y + 4, CPU_FLAGS_X + 1, "Parasite");

  if (info.GetActiveCPU()->HasPointMut())
    SetBoldColor(COLOR_CYAN);
  else SetColor(COLOR_CYAN);
  Print(CPU_FLAGS_Y + 5, CPU_FLAGS_X + 1, "Point Mutated");
      
  if (info.GetActiveCPU()->IsModified())
    SetBoldColor(COLOR_CYAN);
  else SetColor(COLOR_CYAN);
  Print(CPU_FLAGS_Y + 6, CPU_FLAGS_X + 1, "Modified");
      
  SetColor(COLOR_WHITE);

  if (info.GetPauseLevel() && info.GetActiveCPU() &&
      info.GetActiveCPU()->IsParasite()) {
    if (parasite_zoom) Print(17, 12, "[X] Host Code    ");
    else Print(17, 12, "[X] Parasite Code");
  }

  // Place the task information onto the screen.

  cPhenotype * phenotype = info.GetActiveCPU()->GetPhenotype();
  SetColor(COLOR_CYAN);

  int task_num = task_offset;
  int col_num = 0;
  for (int cur_col = TASK_X + 12;
       task_num < cConfig::GetNumTasks();
       cur_col += 14) {
    for (int cur_row = TASK_Y + 1;
	 cur_row <= Height() - 2 && task_num < cConfig::GetNumTasks();
	 cur_row++) {
      if (col_num < 2) {
	Print(cur_row, cur_col, "%2d", phenotype->GetTaskCount(task_num));
      }
      task_num++;
    }
    col_num++;
  }



  if (!info.GetActiveCPU()) info.SetActiveCPU( &(population.GetCPU(0)) );
  SetBoldColor(COLOR_WHITE);

  // Figure out which CPU we're pointing at (useful for watching parasites)
  int id = info.GetActiveEnvironment()->GetFacing()->GetEnvironment()->GetID();
  if (info.GetActiveCPU()->GetInstPointer().TestParasite()) {
    // We are in a parasite...
    Print(2, 27, "Executing.:");
    id = info.GetActiveCPU()->GetInstPointer().GetCurID();
  } else {
    // Not parasitic...
    Print(2, 27, "Facing....:");
    
  }

  // Show the location of the CPU we are looking at.
  SetBoldColor(COLOR_CYAN);
  Print(1, 40, "[%2d, %2d] ",
	info.GetActiveID() % population.GetWorldX(),
	info.GetActiveID() / population.GetWorldY());

  // Now show the location of the CPU we are facing/executing.
  Print(2, 40, "[%2d, %2d] ",
	id % population.GetWorldX(), id / population.GetWorldY());

  // And print the IP.

  const cCPUHead & active_inst_ptr = info.GetActiveCPU()->GetInstPointer();
  int cur_id = active_inst_ptr.GetCurCPU()->GetEnvironment()->GetID();
  Print(11, 36, "%12s", cString::Stringf("[%2d,%2d] : %2d",
	cur_id % population.GetWorldX(), cur_id / population.GetWorldX(),
	active_inst_ptr.GetPosition())() );

}

void cZoomScreen::UpdateCPU()
{
  int i;

  // Place the registers onto the screen.

  SetBoldColor(COLOR_CYAN);
  for (i = 0; i < NUM_REGISTERS; i++) {
    Print(REG_Y+3 + i, REG_X+6, "%11d", info.GetActiveCPU()->GetRegister(i));
  }

  // Place the active stack onto the screen.

  // Stack A
  // SetBoldColor(COLOR_CYAN);   // -Redundant
  SetColor(COLOR_WHITE);
  char stack_letter = 'A' + info.GetActiveCPU()->GetActiveStack();
  Print(STACK_Y + 1, STACK_X + 2, "Stack %c", stack_letter);

  SetBoldColor(COLOR_CYAN);
  Print(STACK_Y+3, STACK_X + 2, "%11d", info.GetActiveCPU()->GetStack(0));
  SetColor(COLOR_CYAN);
  for (i = 1; i <= 2; i++) {
    Print(STACK_Y+3 + i, STACK_X+2, "%11d", info.GetActiveCPU()->GetStack(i));
  }

  // Place the input buffer on the screen.

  SetBoldColor(COLOR_CYAN);
  Print(INPUT_Y+3, INPUT_X+2, "%12d", info.GetActiveCPU()->GetInput(0));

  SetColor(COLOR_CYAN);
  for (i = 1; i < IO_SIZE; i++) {
    Print(INPUT_Y+3+i, INPUT_X+2, "%12d", info.GetActiveCPU()->GetInput(i));
  }

  // Place the visible section of the current memory onto the screen.

  SetBoldColor(COLOR_WHITE);
  if (info.GetPauseLevel() == PAUSE_OFF) {
    // If not paused, then set user desired offset to zero AND always show
    // the code that is executing (ie. do not show original Parasite code if
    // host is executing)
    memory_offset = 0;
    parasite_zoom = FALSE;
  }

  Print(14, 69, "%10d", info.GetActiveGenotypeID());
  Print(15, 69, "%10s", info.GetActiveName()());

  Print(17, 69, "%10d", info.GetActiveCPU()->GetNumErrors());
  Print(18, 69, "%10d", info.GetActiveCPU()->GetNumDivides());
  if (info.GetThreadLock() != -1) Print(19, 67, "LOCKED");
  else Print(19, 67, "      ");
  Print(19, 74, "%2d/%2d",
	info.GetActiveCPU()->GetActiveThreadID() + 1,
	info.GetActiveCPU()->GetNumThreads());

  // This line gets the creature that is currently executing. Usually the
  // creature we are viewing, but can also be a different one (if this is a
  // parasite).

  const cCodeArray & memory = (parasite_zoom) ?
    info.GetActiveCPU()->GetMemory() :
    info.GetActiveCPU()->GetInstPointer().GetMemory();
  SetColor(COLOR_WHITE);
  Print(MEMORY_Y + 1, MEMORY_X + 9, "%4d", memory.GetSize());


  // Nothing past this point gets executed for empty creatures....
  if (memory.GetSize() == 0) {
    for (i = -MEMORY_PRE_SIZE; i <= MEMORY_POST_SIZE; i++) {
      HLine(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X, 36);
    }
    return;
  }

  int adj_inst_ptr = 0;
  int base_inst_ptr = info.GetActiveCPU()->GetInstPointer().GetPosition();
  if (base_inst_ptr < 0 || parasite_zoom) base_inst_ptr = 0;

  cInstLib * inst_lib = population.GetInstLib();

  // Determine the center (must be between 0 and size - 1)
  int center_pos = (base_inst_ptr + memory_offset) % memory.GetSize();
  if (center_pos < 0) center_pos += memory.GetSize();

  for (i = -MEMORY_PRE_SIZE; i <= MEMORY_POST_SIZE; i++) {
    adj_inst_ptr = (center_pos + i) % (memory.GetSize() + 1);
    if (adj_inst_ptr < 0) adj_inst_ptr += memory.GetSize() + 1;

    if (adj_inst_ptr == memory.GetSize()) {
      HLine(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X, 36);
    } else {
      Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X, CHAR_VLINE);
      Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 35, CHAR_VLINE);

      // If we are on the instruction about to be executed by the CPU,
      // hilight it...
      if (adj_inst_ptr == base_inst_ptr) {
	SetBoldColor(COLOR_WHITE);
	Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 1, "%3d: ",
		  adj_inst_ptr);
	SetBoldColor(COLOR_CYAN);
      } else {
	if (i == 0) SetBoldColor(COLOR_YELLOW);
	Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 1, "%3d: ",
		  adj_inst_ptr);
	// If we are scrolling through memory, make the current position
	// yellow.
	if (i == 0) SetColor(COLOR_YELLOW);
	else SetColor(COLOR_CYAN);
      }

      // Print the instruction...
      Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 6, "%8s ",
	    inst_lib->GetName(memory.Get(adj_inst_ptr))());
      
      if (inst_lib->GetNumArgs() > 0 && inst_view_mode == TRUE) {
#ifdef INST_ARGS
	// And its arguments, if it has any.
	for (int arg_num = 0; arg_num < inst_lib->GetNumArgs(); arg_num++) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i,
		MEMORY_X + 15 + arg_num * 10, "%9s ",
		memory.Get(adj_inst_ptr).GetArg(arg_num).AsString(inst_lib)());
	}
	for (int j = inst_lib->GetNumArgs(); j < 2; j++) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 15 + j * 10,
		"          ");
	}
#endif
      } else {
	Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 15,
	      "                    ");
	if (memory.GetFlag(adj_inst_ptr, INST_FLAG_COPIED)) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 15, "Cpy");
	}
	if (memory.GetFlag(adj_inst_ptr, INST_FLAG_MUTATED)) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 19, "Mut");
	}
	if (memory.GetFlag(adj_inst_ptr, INST_FLAG_EXECUTED)) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 23, "Exe");
	}
	if (memory.GetFlag(adj_inst_ptr, INST_FLAG_BREAKPOINT)) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 27, "Bp");
	}

	const cHardware & hardware = info.GetActiveCPU()->GetHardware();

	if (adj_inst_ptr == hardware.GetHead(HEAD_READ).GetPosition()) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 30, "R");
	}
	if (adj_inst_ptr == hardware.GetHead(HEAD_WRITE).GetPosition()) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 31, "W");
	}
	if (adj_inst_ptr == hardware.GetHead(HEAD_FLOW).GetPosition()) {
	  Print(MEMORY_Y + MEMORY_PRE_SIZE + 3 + i, MEMORY_X + 32, "F");
	}
      }

      SetColor(COLOR_WHITE);
    }
  }

  DrawMiniMap();

  cString & cur_fault = info.GetActiveCPU()->GetFault();
  if (cur_fault.GetSize() > 0) {
    SetBoldColor(COLOR_RED);
    Print(FAULT_Y, FAULT_X, "Fault:");
    SetBoldColor(COLOR_CYAN);
    Print(FAULT_Y, FAULT_X + 7, cur_fault());
  } else {
    Print(FAULT_Y, FAULT_X, "                                        ");
  }

  SetBoldColor(COLOR_WHITE);

  Print(13, 70, "[%3d,%3d] ",
	info.GetActiveID() % population.GetWorldX(),
	info.GetActiveID() / population.GetWorldY());

  SetBoldColor(COLOR_CYAN);
  Refresh();
}

void cZoomScreen::UpdateGenotype()
{
  SetBoldColor(COLOR_CYAN);

    Print(1, 12, "%9d", info.GetActiveGenotypeID());
    Print(2, 12, "%9s", info.GetActiveName()());
    Print(3, 12, "%9d", info.GetActiveSpeciesID());
    
  if (info.GetActiveGenotype() != NULL) {
    cGenotype & genotype = *(info.GetActiveGenotype());
    Print(5, 12, "%9d", genotype.GetNumCPUs());
    Print(6, 12, "%9d", genotype.GetLength());
    Print(7, 12, "%9d", genotype.GetCopiedSize());
    Print(8, 12, "%9d", genotype.GetExecutedSize());
    
    PrintFitness(10, 14, genotype.GetFitness());
    Print(11, 12, "%9d", genotype.GetGestationTime());
    Print(12, 14, "%9f", genotype.GetMerit());
    Print(13, 12, "%9f", genotype.GetReproRate());
    
    // Column 2
    Print(1, 40, "%9d", genotype.GetUpdateBorn());
    Print(2, 40, "%9d", genotype.GetParentID());
    Print(3, 40, "%9d", genotype.GetParentDistance());
    Print(4, 40, "%9d", genotype.GetDepth());
    
    Print(7, 40,  "%9d", genotype.GetThisDeaths());
    Print(8, 40,  "%9d", genotype.GetThisBirths());
    Print(9, 40,  "%9d", genotype.GetThisBreedTrue());
    Print(10, 40, "%9d", genotype.GetThisBreedIn());
    Print(11, 40, "%9d", genotype.GetThisBirths() - genotype.GetThisBreedTrue());
    
    Print(14, 40, "%9d", genotype.GetTotalCPUs());
    Print(15, 40, "%9d", genotype.GetBirths());
    Print(16, 40, "%9d", genotype.GetBreedTrue());
    Print(17, 40, "%9d", genotype.GetBreedIn());
    Print(18, 40, "%9d", genotype.GetBirths() - genotype.GetBreedTrue());
  }
  else {
    Print(5, 12, "  -------");
    Print(6, 12, "  -------");
    Print(7, 12, "  -------");
    Print(8, 12, "  -------");
    
    Print(10, 12, "  -------");
    Print(11, 12, "  -------");
    Print(12, 12, "  -------");
    Print(13, 12, "  -------");
    
    // Column 2
    Print(1, 40, "  -------");
    Print(2, 40, "  -------");
    Print(3, 40, "  -------");
    Print(4, 40, "  -------");
    
    Print(7, 40,  "  -------");
    Print(8, 40,  "  -------");
    Print(9, 40,  "  -------");
    Print(10, 40, "  -------");
    Print(11, 40, "  -------");
    
    Print(14, 40, "  -------");
    Print(15, 40, "  -------");
    Print(16, 40, "  -------");
    Print(17, 40, "  -------");
    Print(18, 40, "  -------");
  }
}

void cZoomScreen::EditMemory()
{
  // Collect all of the needed variables.
  cInstLib * inst_lib = population.GetInstLib();
  cCPUHead edit_head(info.GetActiveCPU()->GetInstPointer());
  if (parasite_zoom) {
    edit_head.Set(0, info.GetActiveCPU());
  }
  edit_head.LoopJump(memory_offset);

  // Assemble first choice window.
  cMenuWindow menu1(NUM_INST_EDITS);
  menu1.SetTitle("Choose Edit method:");
  menu1.AddOption(INST_EDIT_BREAKPOINT, "Toggle [B]reakpoint");
  menu1.AddOption(INST_EDIT_JUMP_IP,    "[J]ump IP");
  menu1.AddOption(INST_EDIT_CHANGE,     "[E]dit Instruction");
  menu1.AddOption(INST_EDIT_INSERT,     "[I]nsert Instruction");
  menu1.AddOption(INST_EDIT_REMOVE,     "[D]elete Instruction");
  menu1.SetActive(INST_EDIT_CHANGE);
  int edit_method = menu1.Activate();
  cView::Redraw();

  // If we need to choose a new instruction, bring up a window for it.

  int new_inst = 0;
  if (edit_method == INST_EDIT_CHANGE || edit_method == INST_EDIT_INSERT) {
    cMenuWindow inst_menu(inst_lib->GetSize());
    inst_menu.SetTitle("Choose new instruction: ");
    for (int j = 0; j < inst_lib->GetSize(); j++) {
      inst_menu.AddOption(j, inst_lib->GetInstEntry(j).GetName()());
    }
    inst_menu.SetActive(edit_head.GetInst().GetOp());
    new_inst = inst_menu.Activate();

    cView::Redraw();
    if (new_inst == -1) {
      //  cView::Notify("Aborted!");
      return;
    }
  }

  // Finally, act on the edit method!
  switch (edit_method) {
  case INST_EDIT_BREAKPOINT:
    edit_head.ToggleFlag(INST_FLAG_BREAKPOINT);
    break;
  case INST_EDIT_JUMP_IP:
    info.GetActiveCPU()->SetInstPointer(edit_head);
    memory_offset = 0;
    break;
  case INST_EDIT_CHANGE:
    edit_head.SetInst(cInstruction(new_inst));
    break;
  case INST_EDIT_INSERT:
    edit_head.InsertInst(cInstruction(new_inst));
    break;
  case INST_EDIT_REMOVE:
    edit_head.RemoveInst();
    break;
  default:
    //    cView::Notify("Aborted!");
    break;
  }

  Update();
}

void cZoomScreen::ViewMemory()
{
  // Collect all of the needed variables.
  cInstLib * inst_lib = population.GetInstLib();
  cCPUHead view_head(info.GetActiveCPU()->GetInstPointer());
  if (parasite_zoom) {
    view_head.Set(0, info.GetActiveCPU());
  }
  view_head.LoopJump(memory_offset);

  int view_method = INST_VIEW_DETAILS;
  if (inst_lib->GetNumArgs() > 0) {
    // Assemble first choice window.
    cMenuWindow menu1(NUM_INST_VIEWS);
    menu1.SetTitle("Choose VIEW method:");
    menu1.AddOption(INST_VIEW_DETAILS, "Display Instruction Info");
    menu1.AddOption(INST_VIEW_MODE,    "Toggle Instruction Mode");
    menu1.SetActive(INST_VIEW_DETAILS);
    view_method = menu1.Activate();
    cView::Redraw();
  }

  // Act on the view method!
  switch (view_method) {
  case INST_VIEW_DETAILS:
    ViewInstruction();
    break;
  case INST_VIEW_MODE:
    if (inst_view_mode == TRUE) inst_view_mode = FALSE;
    else inst_view_mode = TRUE;
    break;
  default:
    cView::Notify("Aborted!");
    break;
  }

  Update();
}

void cZoomScreen::ThreadOptions()
{
  int thread_method = THREAD_OPTIONS_VIEW;

  // Assemble first choice window.
  cMenuWindow menu1(NUM_THREAD_OPTIONS);
  menu1.SetTitle("Choose threading option:");
  menu1.AddOption(THREAD_OPTIONS_VIEW, "[V]iew Thread Info");
  menu1.AddOption(THREAD_OPTIONS_LOCK, "[T]oggle Thread Lock");
  menu1.SetActive(THREAD_OPTIONS_VIEW);
  thread_method = menu1.Activate();
  cView::Redraw();

  // Act on the view method!
  switch (thread_method) {
  case THREAD_OPTIONS_VIEW:
    ViewThreads();
    break;
  case THREAD_OPTIONS_LOCK:
    if (info.GetThreadLock() != -1) info.SetThreadLock(-1);
    else info.SetThreadLock(info.GetActiveCPU()->GetActiveThreadID());

    break;
  default:
     break;
  }

  Update();
}

void cZoomScreen::ViewInstruction()
{
  cTextWindow * window = new cTextWindow(10, 40, 7, 20);
  window->SetBoldColor(COLOR_WHITE);
  window->Box();

  // Place the data.

  cCPUHead inst_ptr(info.GetActiveCPU()->GetInstPointer());
  if (parasite_zoom) {
    inst_ptr.Set(0, info.GetActiveCPU());
  }
  inst_ptr.LoopJump(memory_offset);

  const cInstLib * inst_lib = population.GetInstLib();
  const cInstEntry & inst_entry = inst_lib->GetInstEntry(inst_ptr.GetInst().GetOp());

  window->SetBoldColor(COLOR_YELLOW);
#ifdef INST_ARGS
  int num_args = inst_lib->GetNumArgs();

  if (num_args >= 2) {
    window->Print(2, 16, "%s %s %s", inst_lib->GetName(inst_ptr.GetInst())(),
	  inst_ptr.GetInst().GetArg(0).AsString(inst_lib)(),
	  inst_ptr.GetInst().GetArg(1).AsString(inst_lib)() );
  } else if (num_args == 1) {
    window->Print(2, 16, "%s %s", inst_lib->GetName(inst_ptr.GetInst())(),
	  inst_ptr.GetInst().GetArg(1).AsString(inst_lib)() );
  } else
#endif
    window->Print(2, 16, "%s", inst_lib->GetName(inst_ptr.GetInst())());

  window->SetBoldColor(COLOR_WHITE);

  window->Print(2, 2, "Instruction: ");
  window->Print(4, 2, "Line Num...: ");
  window->Print(5, 2, "Args Used..: ");
  window->Print(6, 2, "Redundancy.: ");
  window->Print(7, 2, "Cost.......: ");

  window->SetBoldColor(COLOR_CYAN);

  window->Print(4, 14, "%3d", inst_ptr.GetPosition());
  window->Print(5, 14, "%3d", inst_entry.GetNumArgs());
  window->Print(6, 14, "%3d", inst_entry.GetRedundance());
  window->Print(7, 14, "%3d", inst_entry.GetCost());

  if (inst_ptr.GetFlag(INST_FLAG_COPIED)) window->SetBoldColor(COLOR_CYAN);
  else window->SetColor(COLOR_CYAN);
  window->Print(4, 25, "Copied");

  if (inst_ptr.GetFlag(INST_FLAG_MUTATED)) window->SetBoldColor(COLOR_CYAN);
  else window->SetColor(COLOR_CYAN);
  window->Print(5, 25, "Mutated");

  if (inst_ptr.GetFlag(INST_FLAG_EXECUTED)) window->SetBoldColor(COLOR_CYAN);
  else window->SetColor(COLOR_CYAN);
  window->Print(6, 25, "Executed");

  if (inst_ptr.GetFlag(INST_FLAG_BREAKPOINT)) window->SetBoldColor(COLOR_CYAN);
  else window->SetColor(COLOR_CYAN);
  window->Print(7, 25, "Breakpoint");


  // Print it!
  window->Refresh();

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

  delete window;
  cView::Redraw();
}

void cZoomScreen::ViewRegisters()
{
  cTextWindow * window = new cTextWindow(9, 54, 4, 13);

  window->SetBoldColor(COLOR_WHITE);
  window->Box();
    
  window->Print(2, 2, "Registers");
  
  for (int i = 0; i < 3; i++) {
    const char reg_letter = 'A' + i;
    const int reg_value = info.GetActiveCPU()->GetRegister(i);
    window->SetBoldColor(COLOR_WHITE);
    window->Print(4+i, 2, "%cX:", reg_letter);
    window->Print(4+i, 17, '[');
    window->Print(4+i, 50, ']');
    window->SetBoldColor(COLOR_CYAN);
    window->Print(4+i, 6, "%10d", reg_value);
    window->PrintBinary(4+i, 18, reg_value);
  }
    
  // Print it!
  window->Refresh();
    
  // Wait for the results.
  bool finished = false;
  while (!finished) {
    int cur_char = GetInput();
    switch (cur_char) {
    case 'q':
    case 'Q':
    case ' ':
    case '\n':
    case '\r':
      finished = true;
      break;
    }
  }
  
  delete window;
  cView::Redraw();
}


void cZoomScreen::ViewStack()
{
  cTextWindow * window = new cTextWindow(16, 50, 4, 15);

  bool finished = false;
  int active_stack = info.GetActiveCPU()->GetActiveStack();

  
  while (!finished) {
    window->SetBoldColor(COLOR_WHITE);
    window->Box();
    
    char stack_letter = 'A' + active_stack;
    window->Print(2, 5, "[<] Stack %c [>]", stack_letter);
    window->SetBoldColor(COLOR_CYAN);
    window->Print(2, 6, '<');
    window->Print(2, 18, '>');
    
    window->SetBoldColor(COLOR_WHITE);
    for (int i = 0; i < 10; i++) {
      const int stack_value = info.GetActiveCPU()->GetStack(i,active_stack);
      window->Print(4+i, 2, "%10d : ", stack_value);
      window->PrintBinary(4+i, 15, stack_value);
    }
    
    // Print it!
    window->Refresh();
    
    // Wait for the results.
    bool legal_keypress = false;
    while (!legal_keypress) {
      int cur_char = GetInput();
      switch (cur_char) {
      case ',':
      case '<':
      case '.':
      case '>':
	active_stack = 1 - active_stack;
	legal_keypress = true;
	break;
      case 'q':
      case 'Q':
      case ' ':
      case '\n':
      case '\r':
	finished = true;
	legal_keypress = true;
	break;
      }
    }

  }

  delete window;
  cView::Redraw();
}


void cZoomScreen::ViewInputs()
{
  cTextWindow * window = new cTextWindow(9, 54, 4, 13);

  window->SetBoldColor(COLOR_WHITE);
  window->Box();
    
  window->Print(2, 2, "Inputs");
  
  for (int i = 0; i < 3; i++) {
    const int input_value = info.GetActiveCPU()->GetInput(i);
    window->SetBoldColor(COLOR_WHITE);
    window->Print(4+i, 2, "%d:", i);
    window->Print(4+i, 17, '[');
    window->Print(4+i, 50, ']');
    window->SetBoldColor(COLOR_CYAN);
    window->Print(4+i, 6, "%10d", input_value);
    window->PrintBinary(4+i, 18, input_value);
  }
    
  // Print it!
  window->Refresh();
    
  // Wait for the results.
  bool finished = false;
  while (!finished) {
    int cur_char = GetInput();
    switch (cur_char) {
    case 'q':
    case 'Q':
    case ' ':
    case '\n':
    case '\r':
      finished = true;
      break;
    }
  }
  
  delete window;
  cView::Redraw();
}


void cZoomScreen::ViewThreads()
{
}

cCoords cZoomScreen::GetSectionCoords(int in_section)
{
  switch (in_section) {
  case ZOOM_SECTION_MEMORY:
    return cCoords(MEMORY_X, MEMORY_Y);
    break;
  case ZOOM_SECTION_REGISTERS:
    return cCoords(REG_X, REG_Y);
    break;
  case ZOOM_SECTION_STACK:
    return cCoords(STACK_X, STACK_Y);
    break;
  case ZOOM_SECTION_INPUTS:
    return cCoords(INPUT_X, INPUT_Y);
    break;
  case ZOOM_SECTION_MAP:
    return cCoords(MINI_MAP_X, MINI_MAP_Y);
    break;
  }

  return cCoords(0,0);
}

cString cZoomScreen::GetSectionName(int in_section)
{
  switch (in_section) {
  case ZOOM_SECTION_MEMORY:
    return cString("Memory");
    break;
  case ZOOM_SECTION_REGISTERS:
    return cString("Registers");
    break;
  case ZOOM_SECTION_STACK:
    return cString("Stack");
    break;
  case ZOOM_SECTION_INPUTS:
    return cString("Inputs");
    break;
  case ZOOM_SECTION_MAP:
    return cString("Mini-Map");
    break;
  }

  return cString("Unknown!");
}

void cZoomScreen::SetActiveSection(int in_section)
{
  if (in_section != active_section) {
    // Set the old section to be normal white
    cCoords sect_coords(GetSectionCoords(active_section));
    sect_coords.Translate(2, 1);
    SetColor(COLOR_WHITE);
    Print(sect_coords.GetY(), sect_coords.GetX(),
	  GetSectionName(active_section)());
    active_section = in_section;
  }

  cCoords sect_coords(GetSectionCoords(active_section));
  sect_coords.Translate(2, 1);
  SetBoldColor(COLOR_BLUE);
  Print(sect_coords.GetY(), sect_coords.GetX(),
	GetSectionName(active_section)());
  SetColor(COLOR_WHITE);
}

void cZoomScreen::DoInput(int in_char)
{
  // First do the Mode specific io...
  if (mode == ZOOM_MODE_CPU      && DoInputCPU(in_char)) return;
  if (mode == ZOOM_MODE_STATS    && DoInputStats(in_char)) return;
  if (mode == ZOOM_MODE_GENOTYPE && DoInputGenotype(in_char)) return;
  
  switch(in_char) {
  case 't':
  case 'T':
    ThreadOptions();
    break;

  /*
    case 'x':
    case 'X':    
      // Toggle the parasite_zoom flag IF we are on a parasite
      if (info.GetPauseLevel() && info.GetActiveCPU() &&
      info.GetActiveCPU()->IsParasite()) {
      parasite_zoom = !parasite_zoom;
      Update();
      }
      break;
    */
  case ' ':
    memory_offset = 0;
    parasite_zoom = FALSE;
    info.GetActiveCPU()->GetFault() = "";
    info.EngageStepMode();
    break;
  case '>':
  case '.':
    mode++;
    if (mode == NUM_ZOOM_MODES) mode = 0;
    Clear();
    Draw();
    break;
  case '<':
  case ',':
    mode--;
    if (mode == -1) mode = NUM_ZOOM_MODES - 1;

    Clear();
    Draw();
    break;

  default:
    g_debug.Warning("Unknown Command (#%d): [%c]", in_char, in_char);
    break;
  }
}


bool cZoomScreen::DoInputCPU(int in_char)
{
  switch(in_char) {
  case '2':
  case KEY_DOWN:
    if (active_section == ZOOM_SECTION_MAP) {
      mini_center_id += map_x_size;
      mini_center_id %= population.GetSize();
      if (population.GetCPU(mini_center_id).GetActiveGenotype() != NULL) {
	memory_offset = 0;
	info.SetActiveCPU( &(population.GetCPU(mini_center_id)));
      }
      Update();
    } else if (active_section == ZOOM_SECTION_MEMORY) {
      memory_offset++;
      Update();
    }
    break;
  case '8':
  case KEY_UP:
    if (active_section == ZOOM_SECTION_MAP) {
      mini_center_id -= map_x_size;
      if (mini_center_id < 0) mini_center_id += population.GetSize();
      if (population.GetCPU(mini_center_id).GetActiveGenotype() != NULL) {
	memory_offset = 0;
	info.SetActiveCPU( &(population.GetCPU(mini_center_id)));
      }
      Update();
    } else if (active_section == ZOOM_SECTION_MEMORY) {
      memory_offset--;
      Update();
    }
    break;
  case '6':
  case KEY_RIGHT:
    if (active_section == ZOOM_SECTION_MAP) {
      mini_center_id++;
      if (mini_center_id == population.GetSize()) mini_center_id = 0;
      if (population.GetCPU(mini_center_id).GetActiveGenotype() != NULL) {
	memory_offset = 0;
	info.SetActiveCPU( &(population.GetCPU(mini_center_id)));
      }
      Update();
    }
    break;
  case '4':
  case KEY_LEFT:
    if (active_section == ZOOM_SECTION_MAP) {
      mini_center_id--;
      if (mini_center_id < 0) mini_center_id += population.GetSize();
      if (population.GetCPU(mini_center_id).GetActiveGenotype() != NULL) {
	memory_offset = 0;
	info.SetActiveCPU( &(population.GetCPU(mini_center_id)));
      }
      Update();
    }
    break;
  case '+':
  case '=':
    ++map_mode %= NUM_MAPS;
    Update();
    break;
  case '-':
  case '_':
    map_mode += NUM_MAPS;
    --map_mode %= NUM_MAPS;
    Update();
    break;
  case '\n':
  case '\r':
    if (active_section == ZOOM_SECTION_MAP) {
      memory_offset = 0;
      info.SetActiveCPU( &(population.GetCPU(mini_center_id)));
    }
    Update();
    break;
  case '\t':
    {
      int next_section = active_section + 1;
      if (next_section == NUM_ZOOM_SECTIONS) next_section = 0;
      SetActiveSection(next_section);
    }
    Refresh();
    break;

  case 'e':
  case 'E':
    if( active_section == ZOOM_SECTION_MEMORY) {
      EditMemory();
    }
    break;

  case 'v':
  case 'V':
    switch (active_section) {
    case ZOOM_SECTION_MEMORY:
      ViewMemory();
      break;
    case ZOOM_SECTION_REGISTERS:
      ViewRegisters();
      break;
    case ZOOM_SECTION_STACK:
      ViewStack();
      break;
    case ZOOM_SECTION_INPUTS:
      ViewInputs();
      break;
    }
    break;


  default:
    return false;
  };

  return true;
}

bool cZoomScreen::DoInputStats(int in_char)
{
  switch(in_char) {
  case '6':
  case KEY_RIGHT:
    {
      const int new_task_offset = task_offset + Height() - TASK_Y - 2;
      if (new_task_offset < cConfig::GetNumTasks()) {
	task_offset = new_task_offset;
	Draw();
      }
    }
    break;
  case '4':
  case KEY_LEFT:
    {
      const int new_task_offset = task_offset - Height() + TASK_Y + 2;
      if (new_task_offset >= 0) {
	task_offset = new_task_offset;
	Draw();
      }
    }
    break;

  default:
    return false;
  }

  return true;
}

bool cZoomScreen::DoInputGenotype(int in_char)
{
  return false;
}

void cZoomScreen::DrawMiniMap()
{
  // if (GetSubWindow(0) == NULL) AddSubWindow(map_screen, 0);
  // map_screen->Update();

  char * map = NULL;
  char * color_map = NULL;
  int x, y, cur_x, cur_y, virtual_x, virtual_y, index;

  // Setup the start color for the map...
  SetColor(COLOR_WHITE);

  switch (map_mode) {
  case MAP_BASIC:
    if (HasColors()) color_map = population.GetBasicGrid();
    else map = population.GetBasicGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "Genotypes");
    break;
  case MAP_SPECIES:
    if (HasColors()) color_map = population.GetSpeciesGrid();
    else map = population.GetSpeciesGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, " Species ");
    break;
  case MAP_COMBO:
    color_map = population.GetBasicGrid();
    map = population.GetSpeciesGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "  Combo  ");
    break;
  case MAP_INJECT:
    map = population.GetModifiedGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "Modified ");
    break;
  case MAP_RESOURCE:
    map = population.GetResourceGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "Resources");
    break;
  case MAP_AGE:
    map = population.GetAgeGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "   Age   ");
    break;
  case MAP_BREED_TRUE:
    if (HasColors()) color_map = population.GetBreedGrid();
    else map = population.GetBreedGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "BreedTrue");
    break;
  case MAP_PARASITE:
    if (HasColors()) color_map = population.GetParasiteGrid();
    else map = population.GetParasiteGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "Parasite ");
    break;
  case MAP_POINT_MUT:
    if (HasColors()) color_map = population.GetPointMutGrid();
    else map = population.GetPointMutGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, "Point Mut");
    break;
  case MAP_THREAD:
    if (HasColors()) color_map = population.GetThreadGrid();
    else map = population.GetThreadGrid();
    Print(MINI_MAP_Y + 11, MINI_MAP_X + 4, " Thread  ");
    break;
  }

  virtual_x = (mini_center_id % map_x_size) + map_x_size;
  virtual_y = (mini_center_id / map_x_size) + map_y_size;

  for (y = -3; y <= 3 && y < map_y_size - 3; y++) {
    Move(MINI_MAP_Y + 6 + y, MINI_MAP_X + 2);
    cur_y = (y + virtual_y) % map_y_size;
    for (x = -3; x <= 3 && x < map_x_size - 3; x++) {
      cur_x = (x + virtual_x) % map_x_size;
      index = cur_y * map_x_size + cur_x;
      if (color_map) SetSymbolColor(color_map[index]);
      if (map) Print(map[index]);
      else Print(CHAR_BULLET);

      // Space between columns
      for (int i = 0; i < AVIDA_MAP_X_SPACING - 1; i++)
	Print(' ');
    }
  }
  SetColor(COLOR_WHITE);
  Print(MINI_MAP_Y + 6, MINI_MAP_X + 7, '[');
  Print(MINI_MAP_Y + 6, MINI_MAP_X + 9, ']');

  Refresh();
  if (map) g_memory.Free(map, population.GetSize());
  if (color_map) g_memory.Free(color_map, population.GetSize());  
}

#endif
