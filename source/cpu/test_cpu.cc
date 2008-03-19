//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "test_cpu.hh"
#include "hardware.hh"
#include "hardware.ii"

#include <iomanip.h>

cDivideRecord cTestCPU::ProcessGestation(cCPUTestInfo & test_info,
					 int time_penalty)
{
  int i;  // This can be an unused variable depending on #define's.
  (void) i;

  int time_allocated = cConfig::GetTestCPUTimeMod() *
    cpu.hardware.GetMemorySize() - time_penalty;

#ifdef THREAD_STATS
  tMatrix<int> thread_exe;
  if (test_info.TestThreads() == TRUE) {
    thread_exe.ResizeClear(3, cpu.GetMemorySize() * 2);
    for (i = 0; i < thread_exe.GetNumCols(); i++) {
      thread_exe[0][i] = 0;
      thread_exe[1][i] = 0;
      thread_exe[2][i] = 0;
    }
  }
#endif

  // Reset the CPU
  cpu.phenotype->Clear(1);
  cpu.info.gestation_time = 0;
  cpu.info.fitness = 0.0;
  cpu.info.num_errors = 0;
  cpu.info.copied_size = 0;
  cpu.info.executed_size = 0;
  cpu.repro_data.copy_true = FALSE;

#ifdef QUICK_BASE_TEST_CPU
  // This checks to make sure a 'copy', 'divide', and 'allocate' are all in
  // the creatures, and if not doesn't even bother to test it.
  static UCHAR copy_id  = cpu.hardware.GetInstLib().GetInstID("copy");
  static UCHAR div_id   = cpu.hardware.GetInstLib().GetInstID("divide");
  static UCHAR alloc_id = cpu.hardware.GetInstLib().GetInstID("allocate");
#endif

#ifdef QUICK_HEAD_TEST_CPU
  // This checks to make sure a 'copy', 'divide', and 'allocate' are all in
  // the creatures, and if not doesn't even bother to test it.
  static UCHAR copy_id  = cpu.hardware.GetInstLib().GetInstID("h-write");
  static UCHAR div_id   = cpu.hardware.GetInstLib().GetInstID("h-divide");
  static UCHAR alloc_id = cpu.hardware.GetInstLib().GetInstID("h-alloc");
#endif


#ifdef QUICK_TEST_CPU
  int copy_found = FALSE;
  int div_found = FALSE;
  int alloc_found = FALSE;

  const cCodeArray & test_code = cpu.hardware.GetMemory();
  for (i = 0; i < test_code.GetSize(); i++) {
    if (test_code.Get(i).GetOp() == copy_id)  copy_found  = TRUE;
    if (test_code.Get(i).GetOp() == div_id)   div_found   = TRUE;
    if (test_code.Get(i).GetOp() == alloc_id) alloc_found = TRUE;
  }

  if (copy_found == FALSE || div_found == FALSE || alloc_found == FALSE) {
    time_allocated = 0;
  }
#endif

  int time_used = 0;
  int thread_time_used = 0;
  int divide_count = cpu.info.num_divides;
  while (time_used < time_allocated && cpu.hardware.GetMemorySize() &&
	 cpu.info.num_divides == divide_count) {
    time_used++;

    /////////////////////////
    //  SingleTestProcess()
    /////////////////////////

    cpu.flags.SetFlag(CPU_FLAG_ADVANCE_IP);
    cpu.hardware.IncTimeUsed();

#ifdef THREADS
    if (cpu.hardware.GetNumThreads() > 1) thread_time_used++;
    for (i = 0; i < cpu.hardware.GetNumThreads(); i++)
#endif
    {
      cpu.hardware.SetupProcess();

      // Print the status of this CPU at each step... (for debug)
      // cpu.PrintStatus();


      // Find the instruction to be executed and get a pointer to the
      // corrisponding method...

      const cInstruction & cur_inst = cpu.hardware.GetCurInst();
      tCPUMethod inst_ptr =
	cpu.hardware.GetInstLib().GetActiveFunction(cur_inst);

#ifdef THREAD_STATS
      // NOTE: Need to add something to watch out for parasites. @CAO
      if (test_info.TestThreads() == TRUE) {
	int ip_pos = cpu.hardware.GetIPPosition();
	if (ip_pos >= thread_exe.GetNumCols()) {
	  thread_exe.Resize(3, cpu.hardware.GetMemorySize());
	}

	// Keep a count of every time the line was executed.
	thread_exe(2, ip_pos)++;

	// And, during multi-threading, keep a count for each thread.
	if (cpu.hardware.GetNumThreads() == 2 &&
	    cpu.hardware.GetCurThreadID() < 2) {
	  thread_exe(cpu.hardware.GetCurThreadID(), ip_pos)++;
	}
      }
#endif


#ifdef INST_ARGS
      if (cur_inst.GetOp() >= cpu.hardware.GetInstLib().GetSize()) {
	// @CAO This is where a page fault would occur!
	inst_ptr = cpu.hardware.GetInstLib().
	  GetActiveFunction(cInstruction::GetInstDefault());
      }
#endif

      // Mark the instruction as executed
      cpu.hardware.SetIPFlag(INST_FLAG_EXECUTED);

      // And execute it.
      (cpu.*inst_ptr)();

      if (cpu.flags.GetFlag(CPU_FLAG_ADVANCE_IP)) cpu.hardware.AdvanceIP();
    }


  }

  // Do any final calculations...

  double thread_time_diff = 0.0;
  double thread_code_diff = 0.0;

#ifdef THREAD_STATS
  if (test_info.TestThreads() == TRUE) {
    int total_exe = 0;
    int diff_exe = 0;
    int total_code = 0;
    int diff_code = 0;
    tArray<int> diff_count(thread_exe.GetNumCols());
    for (i = 0; i < thread_exe.GetNumCols(); i++) {
      diff_count[i] = thread_exe(0, i) - thread_exe(1, i);
      if (diff_count[i] < 0) diff_count[i] *= -1;
      total_exe += thread_exe(0, i) + thread_exe(1, i);
      diff_exe += diff_count[i];
      if (thread_exe(0, i) || thread_exe(1,i)) total_code += 1;
      if (diff_count[i]) diff_code += 1;
    }
    thread_time_diff = ((double) diff_exe) / ((double) total_exe);
    thread_code_diff = ((double) diff_code) / ((double) total_code);
  }
  if (test_info.PrintThreads() == TRUE) {
    static ofstream fp0("thread0_exe.dat");
    static ofstream fp1("thread1_exe.dat");
    static ofstream fp_main("code_exe.dat");

    fp0     << cStats::GetUpdate();
    fp1     << cStats::GetUpdate();
    fp_main << cStats::GetUpdate();

    for (i = 0; i < thread_exe.GetNumCols(); i++) {
      fp0     << " " << thread_exe(0, i);
      fp1     << " " << thread_exe(1, i);
      fp_main << " " << thread_exe(2, i);
    }

    fp0 << endl;
    fp1 << endl;
    fp_main << endl;
  }

#endif

  double thread_frac = (time_used == 0) ? 0 :
    ((double) thread_time_used) / ((double) time_used);

  cpu.phenotype->TestDivideReset(cpu.GetActiveGenotype()->GetLength(),
			     cpu.info.copied_size, cpu.info.executed_size);

  // Now that merit is setup, recalculate fitness.
  cpu.info.fitness =
    cpu.phenotype->GetCurMerit().CalcFitness(time_used);

  // Save the record of this divide.
  return cDivideRecord(divide_count + 1,
     cpu.phenotype->GetCurMerit().GetDouble(), time_used,
     cpu.info.fitness, cpu.info.num_errors,
     cpu.info.copied_size, cpu.info.executed_size,
     cpu.GetActiveGenotype()->GetLength(),
     cpu.GetCopyTrue(), thread_frac, thread_time_diff, thread_code_diff);
}


///////////////////////
//  Static Methods...
///////////////////////


int cTestCPU::TestCode(cCPUTestInfo & test_info, const cCodeArray & code)
{
  test_info.Clear();
  TestCode_Body(test_info, code, 0);

  return test_info.is_viable;

  //  static ofstream fp("test.dat");
  //  return TestCode(test_info, code, fp);
}

int cTestCPU::TestCode(cCPUTestInfo & test_info, const cCodeArray & code,
		       ofstream & out_fp)
{
  test_info.Clear();
  TestCode_Body(test_info, code, 0);

  ////////////////////////////////////////////////////////////////
  // IsViable() == FALSE
  //   max_depth == 0
  //     num_divides == 0: (0) Parent doesn't divide.
  //     num_divides == 1: (1) Parent doesn't divide second time.
  //   max_depth == 1
  //    (2) Parent does divide, but child does not.
  //   max_depth >= 2
  //    (3) Parent and child do divide, but neither true.
  // ------------------------------------------------------------
  // IsViable() == TRUE
  //   max_depth == 0
  //    (4) Parent Breed True
  //   max_depth == 1
  //    (5) Parent NOT Breed True, but Child Does
  //   max_depth >= 2
  //    (6) Multiple levels of non-breed true, but finally do.
  ////////////////////////////////////////////////////////////////


  int depth_comp = test_info.max_depth;
  if (depth_comp > 2) depth_comp = 2;
  int repro_type = test_info.is_viable * 3 + 1 + depth_comp;

  if (test_info.is_viable == 0 && test_info.max_depth == 0 &&
      test_info.num_divides == 1)
    repro_type = 0;

  out_fp << test_info.is_viable << " "
	 << test_info.max_depth << " "
	 << test_info.depth_found << " "
	 << test_info.num_divides << " "
	 << test_info.max_cycle << " "
	 << repro_type << endl;

  return test_info.is_viable;
}

int cTestCPU::TestCode_Body(cCPUTestInfo & test_info, const cCodeArray & code,
			    int cur_depth)
{
  if (cur_depth > test_info.max_depth) test_info.max_depth = cur_depth;

  test_genotype[cur_depth].SetCode(code);
  test_cpu[cur_depth].ChangeGenotype(&(test_genotype[cur_depth]));
  test_cpu[cur_depth].cpu.ResetVariables();
  int time_penalty = 0;
  int num_copies = 0;

  // Must be able to divide twice in order to form a successful colony,
  // assuming the CPU doesn't get reset on divides.

  const int copies_required =
    (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT) ? 1 : 2;
  while (num_copies < copies_required) {
    test_info.AddDivideRecord(
      test_cpu[cur_depth].ProcessGestation(test_info, time_penalty),
      cur_depth );

    // The possibilities after this gestation cycle are:
    //  1: It did not copy at all => Exit this level.
    //  2: It copied true => Check next gestation cycle, or set is_viable.
    //  3: Its copy looks like an ancestor => copy true.
    //  4: It copied false => we must check the child.

    // Case 1:
    if (test_cpu[cur_depth].cpu.info.gestation_time == 0) {
      //g_debug.Comment("    ...No copy!");
      return FALSE;
    }

    // Case 2:
    if (test_cpu[cur_depth].cpu.GetCopyTrue()) {
      //g_debug.Comment("    ...Copy TRUE!");
      num_copies++;
      time_penalty = 0;
      continue;
    }

    // Case 3:
    int is_ancestor = FALSE;
    for (int anc_depth = 0; anc_depth < cur_depth; anc_depth++) {
      if (*(test_cpu[cur_depth].cpu.GetChild()) ==
	  test_genotype[anc_depth].GetCode()) {
	is_ancestor = TRUE;
	if (test_info.max_cycle < cur_depth - anc_depth)
	  test_info.max_cycle = cur_depth - anc_depth;
	test_info.div_records[cur_depth]->SetCycle(anc_depth);
      }
    }
    if (is_ancestor) {
      //g_debug.Comment("    ...Matches Ancestor!");
      num_copies++;
      time_penalty = 0;
      continue;
    }

    // Case 4:
    //g_debug.Comment("    ...Imperfect Copy... decending!");

    // If we haven't reached maximum depth yet, check out the child.
    if (cur_depth < test_info.GetGenerationTests()) {
      // Run the child's code.
      TestCode_Body(test_info, *(test_cpu[cur_depth].cpu.GetChild()),
		    cur_depth+1);

      // If we've set is_viable at a lower level, we are done!
      if (test_info.is_viable == TRUE) return TRUE;
    }

    // Otherwise treat the child as if it failed.
    if (cConfig::GetDivideMethod() == DIVIDE_METHOD_SPLIT) {
      // Its only possible child failed; we are done!
      break;
    } else {
      // Lets see if it can have another child.
      time_penalty += test_cpu[cur_depth].cpu.info.gestation_time;
    }


  }

  if (num_copies >= copies_required) {
    test_info.depth_found = cur_depth;
    test_info.is_viable = TRUE;
  }

  return test_info.is_viable;
}

// Add a variable size TestCode using:
//  for (int div_test = 0; div_test < test_info.GetDivideTests(); div_test++) {
//
// in order to do a variable number of tests for the printing.

void cTestCPU::PrintCode(const cCodeArray & code)
{
  cString filename;
  filename.Set("%03d-unnamed", code.GetSize());
  PrintCode(code, filename);
}

void cTestCPU::PrintCode(const cCodeArray & code, cString filename, cGenotype * genotype)
{
  if (!code.GetSize()) return;
  int i;

  // Build the test info for printing.
  cCPUTestInfo test_info(TEST_CPU_GENERATIONS, TRUE);
  TestCode(test_info, code);

  // Open the file...

  ofstream fp(filename());

  // @CAO Fix!!!!!!
  if( !fp.good() ) {
    g_debug.Warning("Unable to open %s\n", filename());
    return;
  }

  // Print the useful info at the top...

  int num_levels = test_info.GetMaxDepth() + 1;
  int * num_records = new int[num_levels];
  for (i = 0; i < num_levels; i++) {
    num_records[i] = test_info.GetNumRecords(i);
  }

  fp << "# Filename........: " << filename                             << endl
     << "# Update Output...: " << cStats::GetUpdate()                  << endl
     << "# Is Viable.......: " << test_info.IsViable()                 << endl
     << "# Repro Cycle Size: " << test_info.GetMaxCycle()              << endl
     << "# Depth to Viable.: " << test_info.GetDepthFound()            << endl;

  if (genotype != NULL) {
    fp << "# Update Created..: " << genotype->GetUpdateBorn() << endl
       << "# Genotype ID.....: " << genotype->GetID()         << endl;
    // if (genotype->GetSpecies() != NULL)
    // fp << "# Species ID......: " << genotype->GetSpecies()->GetID() << endl;
    fp << "# Parent Gen ID...: " << genotype->GetParentID()   << endl;
  }
  fp << endl;

  for (int j = 0; j < num_levels; j++) {
    const bool print_ave =
      (j == 0  &&  cConfig::GetDivideMethod() != DIVIDE_METHOD_SPLIT);

    if (cConfig::GetDivideMethod() != DIVIDE_METHOD_SPLIT) {
      fp << "# Generation: " << j << "    ";
      for (i = 0; i < num_records[j]; i++)
	fp << "     Divide-"
	   << test_info.GetDivideRecordByID(i,j)->GetDivideNum();
      if (print_ave) fp << "      Average";
      fp << endl;
    }

    fp << "# Merit...........:";
    for (i = 0; i < num_records[j]; i++)
	fp << " " << setw(12) << test_info.GetDivideRecordByID(i,j)->GetMerit();
    if (print_ave) fp << " " << setw(12) << test_info.CalcMerit();
    fp << endl;


    fp << "# Gestation Time..:";
    for (i = 0; i < num_records[j]; i++)
      fp << " "
	 << setw(12) << test_info.GetDivideRecordByID(i,j)->GetGestationTime();
    if (print_ave) fp << " " << setw(12) << test_info.CalcGestation();
    fp << endl;

    fp << "# Fitness.........:";
    for (i = 0; i < num_records[j]; i++)
      fp << " " << setw(12) << test_info.GetDivideRecordByID(i,j)->GetFitness();
    if (print_ave) fp << " " << setw(12) << test_info.CalcFitness();
    fp << endl;

    fp << "# Errors..........:";
    for (i = 0; i < num_records[j]; i++)
      fp << " " << setw(12) << test_info.GetDivideRecordByID(i,j)->GetNumErrors();
    fp << endl;

    fp << "# Code Size.......:";
    for (i = 0; i < num_records[j]; i++)
      fp << " " << setw(12) << test_info.GetDivideRecordByID(i,j)->GetCodeSize();
    fp << endl;

    fp << "# Copied Size.....:";
    for (i = 0; i < num_records[j]; i++)
      fp << " " << setw(12) << test_info.GetDivideRecordByID(i,j)->GetCopiedSize();
    if (print_ave) fp << " " << setw(12) << test_info.CalcCopiedSize();
    fp << endl;

    fp << "# Executed Size...:";
    for (i = 0; i < num_records[j]; i++)
      fp << " " << setw(12) << test_info.GetDivideRecordByID(i,j)->GetExeSize();
    if (print_ave) fp << " " << setw(12) << test_info.CalcExeSize();
    fp << endl;

    fp << "# Offspring.......:";
    for (i = 0; i < num_records[j]; i++) {
      if (test_info.GetDivideRecordByID(i,j)->GetGestationTime() == 0)
	fp << " " << setw(12) << "NONE";
      else if (test_info.GetDivideRecordByID(i,j)->GetCopyTrue() == TRUE)
	fp << " " << setw(12) << "SELF";
      else if (test_info.GetDivideRecordByID(i,j)->GetCycleTo() != -1)
	fp << " " << setw(12)
	   << test_info.GetDivideRecordByID(i,j)->GetCycleTo();
      else
	fp << " " << setw(12) << (j+1);
    }
    fp << endl;


#ifdef THREADS
    fp << endl;

    fp << "# Thread Time Frac:";
    for (i = 0; i < num_records[j]; i++)
      fp << " "
	 << setw(12) << test_info.GetDivideRecordByID(i,j)->GetThreadFrac();
    fp << endl;

    fp << "# Thread Time Diff:";
    for (i = 0; i < num_records[j]; i++)
      fp << " "
	 << setw(12) << test_info.GetDivideRecordByID(i,j)->GetThreadTimeDiff();
    fp << endl;

    fp << "# Thread Code Diff:";
    for (i = 0; i < num_records[j]; i++)
      fp << " "
	 << setw(12) << test_info.GetDivideRecordByID(i,j)->GetThreadCodeDiff();
    fp << endl;
#endif

    fp << endl;
  }

  for( i=0; i < test_cpu[0].cpu.phenotype->GetNumTasks(); ++i ){
    fp <<"# "<< test_cpu[0].cpu.phenotype->GetTaskLib()->GetTaskName(i)
       <<"\t" << test_cpu[0].cpu.phenotype->GetTaskCount(i)
       <<endl;
  }
  fp<<endl;

  // Display the genome

  const cInstLib & inst_lib = test_cpu[0].cpu.hardware.GetInstLib();
  inst_lib.PrintCode(code, fp);
}

void cTestCPU::PrintTasks(ostream & fp)
{
  for (int i=0; i < test_cpu[0].cpu.phenotype->GetNumTasks(); i++){
    fp << test_cpu[0].cpu.phenotype->GetTaskCount(i) << " ";
  }
  fp << endl;
}


cTestCPU * cTestCPU::test_cpu;
cGenotype cTestCPU::test_genotype[MAX_TEST_DEPTH];
