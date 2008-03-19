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

cDivideRecord cTestCPU::ProcessGestation(cCPUTestInfo & test_info)
{
  int i;  // This can be an unused variable depending on #define's.
  (void) i;

  int time_allocated = cConfig::GetTestCPUTimeMod() *
    cpu.hardware.GetMemorySize();

  // Setup thread tracking, if needed.
  tMatrix<int> thread_exe;
  if (test_info.GetTestThreads() == true) {
    thread_exe.ResizeClear(3, cpu.GetMemorySize() * 2);
    for (i = 0; i < thread_exe.GetNumCols(); i++) {
      thread_exe[0][i] = 0;
      thread_exe[1][i] = 0;
      thread_exe[2][i] = 0;
    }
  }

  // Reset the CPU
  cpu.phenotype->Clear(1);
  cpu.info.gestation_time = 0;
  cpu.info.fitness = 0.0;
  cpu.info.num_errors = 0;
  cpu.info.copied_size = 0;
  cpu.info.executed_size = 0;
  cpu.repro_data.copy_true = FALSE;

  // Setup task-order tracking, in case its needed later.
  cTaskCount last_task_count(cpu.phenotype->GetTaskCount());
  double last_bonus = cpu.phenotype->GetBonus();
  cString task_order = "";

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
  static UCHAR copy_id  = cpu.hardware.GetInstLib().GetInstID("h-copy");
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
  //  int divide_count = cpu.info.num_divides;
  while (time_used < time_allocated && cpu.hardware.GetMemorySize() &&
	 cpu.info.num_divides == 0) {
    time_used++;

    /////////////////////////
    //  SingleTestProcess()
    /////////////////////////

    cpu.advance_ip = true;
    cpu.hardware.IncTimeUsed();

#ifdef THREADS
    if (cpu.hardware.GetNumThreads() > 1) thread_time_used++;
    for (i = 0; i < cpu.hardware.GetNumThreads(); i++)
#endif
    {
      cpu.hardware.SetupProcess();

      // Print the status of this CPU at each step... (for debug)
      if (test_info.GetTraceExecution() == true) {
	ofstream & trace_fp(test_info.GetTraceFP());
	cpu.PrintStatus(trace_fp);
	trace_fp << "  Merit:" << cpu.GetCurMerit()
		 << " Errors:" << cpu.GetNumErrors()
		 << " MemSize:" << cpu.GetMemorySize()
		 << endl
		 << "  " << cpu.GetMemory().AsString()
		 << endl;
      }

      // Find the instruction to be executed and get a pointer to the
      // corrisponding method...

      const cInstruction & cur_inst = cpu.hardware.GetCurInst();
      tCPUMethod inst_ptr =
	cpu.hardware.GetInstLib().GetActiveFunction(cur_inst);

      // NOTE: Need to add something to watch out for parasites. @CAO
      if (test_info.GetTestThreads() == true) {
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


#ifdef INST_ARGS
      if (cur_inst.GetOp() >= cpu.hardware.GetInstLib().GetSize()) {
	// @CAO This is where a page fault would occur!
	inst_ptr = cpu.hardware.GetInstLib().
	  GetActiveFunction(cInstruction::GetInstDefault());
      }
#endif

      // Mark the instruction as executed
      cpu.hardware.SetIPFlag(INST_FLAG_EXECUTED);

      (cpu.*inst_ptr)();

      // If we need to order tasks, do so.
      if (test_info.GetTraceTaskOrder() == true) {
	const cTaskCount & task_count = cpu.phenotype->GetTaskCount();
	const double bonus = cpu.phenotype->GetBonus();
	bool bonus_increase = (bonus > last_bonus);
	bool bonus_decrease = (bonus < last_bonus);
	for (int i = 0; i < task_count.GetNumTasks(); i++) {
	  if (task_count[i] != last_task_count[i]) {
	    char symbol = '=';
	    if (bonus_increase == true) symbol = '+';
	    else if (bonus_decrease == true) symbol = '-';
	    task_order += cString::Stringf("%c%d", symbol, i);
	  }
	}
	last_task_count = task_count;
	last_bonus = bonus;
      }

      if (cpu.advance_ip == true) cpu.hardware.AdvanceIP();
    }


  }

  // Do any final calculations...

  double thread_time_diff = 0.0;
  double thread_code_diff = 0.0;

  if (test_info.GetTestThreads() == true) {
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

    if (test_info.GetPrintThreads() == true) {
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
  }

  double thread_frac = (time_used == 0) ? 0 :
    ((double) thread_time_used) / ((double) time_used);

  cpu.phenotype->TestDivideReset(cpu.GetActiveGenotype()->GetLength(),
			     cpu.info.copied_size, cpu.info.executed_size);

  // Now that merit is setup, recalculate fitness.
  cpu.info.fitness =
    cpu.phenotype->GetCurMerit().CalcFitness(time_used);

  // Save the record of this divide.
  return cDivideRecord(cpu.phenotype->GetCurMerit().GetDouble(),  time_used,
     cpu.info.fitness,  cpu.info.num_errors,  cpu.info.copied_size,
     cpu.info.executed_size,  cpu.GetActiveGenotype()->GetLength(),
     cpu.GetCopyTrue(),  thread_frac,  thread_time_diff,  thread_code_diff,
     task_order);
}


///////////////////////
//  Static Methods...
///////////////////////


bool cTestCPU::TestCode(cCPUTestInfo & test_info, const cCodeArray & code)
{
  test_info.Clear();
  TestCode_Body(test_info, code, 0);

  return test_info.is_viable;
}

bool cTestCPU::TestCode(cCPUTestInfo & test_info, const cCodeArray & code,
		       ofstream & out_fp)
{
  test_info.Clear();
  TestCode_Body(test_info, code, 0);

  ////////////////////////////////////////////////////////////////
  // IsViable() == FALSE
  //   max_depth == 0  : (0) Parent doesn't divide
  //   max_depth == 1  : (2) Parent does divide, but child does not.
  //   max_depth >= 2  : (3) Parent and child do divide, but neither true.
  // ------------------------------------------------------------
  // IsViable() == TRUE
  //   max_depth == 0  : (4) Parent Breed True
  //   max_depth == 1  : (5) Parent NOT Breed True, but Child Does
  //   max_depth >= 2  : (6) Multiple levels of non-breed true.
  ////////////////////////////////////////////////////////////////


  int depth_comp = test_info.max_depth;
  if (depth_comp > 2) depth_comp = 2;
  int repro_type = ((int) test_info.is_viable) * 3 + 1 + depth_comp;

  if (test_info.is_viable == false && test_info.max_depth == 0) {
    repro_type = 0;
  }

  out_fp << test_info.is_viable << " "
	 << test_info.max_depth << " "
	 << test_info.depth_found << " "
	 << test_info.max_cycle << " "
	 << repro_type << endl;

  return test_info.is_viable;
}

bool cTestCPU::TestCode_Body(cCPUTestInfo & test_info, const cCodeArray & code,
			    int cur_depth)
{
  if (cur_depth > test_info.max_depth) test_info.max_depth = cur_depth;

  test_genotype[cur_depth].SetCode(code);
  test_cpu[cur_depth].ChangeGenotype(&(test_genotype[cur_depth]));
  test_cpu[cur_depth].cpu.ResetVariables();

  // Must be able to divide twice in order to form a successful colony,
  // assuming the CPU doesn't get reset on divides.

  test_info.div_records[cur_depth] =
    test_cpu[cur_depth].ProcessGestation(test_info);

  // The possibilities after this gestation cycle are:
  //  1: It did not copy at all => Exit this level.
  //  2: It copied true => Check next gestation cycle, or set is_viable.
  //  3: Its copy looks like an ancestor => copy true.
  //  4: It copied false => we must check the child.

  // Case 1:  ////////////////////////////////////
  if (test_cpu[cur_depth].cpu.info.gestation_time == 0) {
    return false;
  }

  // Case 2:  ////////////////////////////////////
  if (test_cpu[cur_depth].cpu.GetCopyTrue()) {
    test_info.depth_found = cur_depth;
    test_info.is_viable = true;
    return true;
  }

  // Case 3:  ////////////////////////////////////
  bool is_ancestor = false;
  for (int anc_depth = 0; anc_depth < cur_depth; anc_depth++) {
    if (*(test_cpu[cur_depth].cpu.GetChild()) ==
	test_genotype[anc_depth].GetCode()) {
      is_ancestor = true;
      if (test_info.max_cycle < cur_depth - anc_depth)
	test_info.max_cycle = cur_depth - anc_depth;
      test_info.div_records[cur_depth].SetCycle(anc_depth);
    }
  }
  if (is_ancestor) {
    test_info.depth_found = cur_depth;
    test_info.is_viable = true;
    return true;
  }

  // Case 4:  ////////////////////////////////////
  // If we haven't reached maximum depth yet, check out the child.
  if (cur_depth < test_info.GetGenerationTests()) {
    // Run the child's code.
    return TestCode_Body(test_info, *(test_cpu[cur_depth].cpu.GetChild()),
			 cur_depth+1);
  }


  // All options have failed; just return false.
  return false;
}



// This method will test a fragment of code for the phenotypic
// characteristic it conveys...
bool cTestCPU::TestCodeFragment(cCPUTestInfo & test_info,
				const cCodeArray & code, int cpu_time)
{
  test_info.Clear();

  test_genotype[0].SetCode(code);
  test_cpu[0].ChangeGenotype(&(test_genotype[0]));
  test_cpu[0].cpu.ResetVariables();

  //////////////////

  // Reset the CPU
  test_cpu[0].cpu.phenotype->Clear(1);
  test_cpu[0].cpu.info.gestation_time = 0;
  test_cpu[0].cpu.info.fitness = 0.0;
  test_cpu[0].cpu.info.num_errors = 0;
  test_cpu[0].cpu.info.copied_size = 0;
  test_cpu[0].cpu.info.executed_size = 0;
  test_cpu[0].cpu.repro_data.copy_true = FALSE;

  int time_used = 0;
  //  int divide_count = cpu.info.num_divides;
  while (time_used < cpu_time && test_cpu[0].cpu.hardware.GetMemorySize() &&
	 test_cpu[0].cpu.info.num_divides == 0) {
    time_used++;

    /////////////////////////
    //  SingleTestProcess()
    /////////////////////////

    test_cpu[0].cpu.advance_ip = true;
    test_cpu[0].cpu.hardware.IncTimeUsed();

#ifdef THREADS
    for (i = 0; i < test_cpu[0].cpu.hardware.GetNumThreads(); i++)
#endif
    {
      test_cpu[0].cpu.hardware.SetupProcess();

      // Print the status of this CPU at each step... (for debug)
      if (test_info.GetTraceExecution() == true) {
	ofstream & trace_fp(test_info.GetTraceFP());
	test_cpu[0].cpu.PrintStatus(trace_fp);
	trace_fp << "  Merit:" << test_cpu[0].cpu.GetCurMerit()
		 << " Errors:" << test_cpu[0].cpu.GetNumErrors()
		 << " MemSize:" << test_cpu[0].cpu.GetMemorySize()
		 << endl
		 << "  " << test_cpu[0].cpu.GetMemory().AsString()
		 << endl;
      }

      // Find the instruction to be executed and get a pointer to the
      // corrisponding method...

      const cInstruction & cur_inst = test_cpu[0].cpu.hardware.GetCurInst();
      tCPUMethod inst_ptr =
	test_cpu[0].cpu.hardware.GetInstLib().GetActiveFunction(cur_inst);

      // Mark the instruction as executed
      test_cpu[0].cpu.hardware.SetIPFlag(INST_FLAG_EXECUTED);

      // And execute it.
      (test_cpu[0].cpu.*inst_ptr)();

      if (test_cpu[0].cpu.advance_ip == true) {
	test_cpu[0].cpu.hardware.AdvanceIP();
      }
    }


  }

  // Do any final calculations...

  test_cpu[0].cpu.phenotype->TestDivideReset(
      test_cpu[0].cpu.GetActiveGenotype()->GetLength(),
      test_cpu[0].cpu.info.copied_size, test_cpu[0].cpu.info.executed_size);

  // Now that merit is setup, recalculate fitness.
  test_cpu[0].cpu.info.fitness =
    test_cpu[0].cpu.phenotype->GetCurMerit().CalcFitness(time_used);

  // Save the record of this divide.
  test_info.div_records[0] = cDivideRecord(
     test_cpu[0].cpu.phenotype->GetCurMerit().GetDouble(),  time_used,
     test_cpu[0].cpu.info.fitness,  test_cpu[0].cpu.info.num_errors,
     test_cpu[0].cpu.info.copied_size, test_cpu[0].cpu.info.executed_size,
     test_cpu[0].cpu.GetActiveGenotype()->GetLength(),
     test_cpu[0].cpu.GetCopyTrue());

  return true;
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
  cCPUTestInfo test_info;
  test_info.TestThreads();
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

  fp << "# Filename........: " << filename                             << endl
     << "# Update Output...: " << cStats::GetUpdate()                  << endl
     << "# Is Viable.......: " << test_info.IsViable()                 << endl
     << "# Repro Cycle Size: " << test_info.GetMaxCycle()              << endl
     << "# Depth to Viable.: " << test_info.GetDepthFound()            << endl;

  if (genotype != NULL) {
    fp << "# Update Created..: " << genotype->GetUpdateBorn()     << endl
       << "# Genotype ID.....: " << genotype->GetID()             << endl
       << "# Parent Gen ID...: " << genotype->GetParentID()       << endl
       << "# Parent Distance.: " << genotype->GetParentDistance() << endl;
  }
  fp << endl;

  for (int j = 0; j < num_levels; j++) {
    const bool print_colony =
      (j == 0) && (test_info.GetGenotypeCopyTrue() == false);

    fp << "# Generation: " << j << "    ";
    if (print_colony) fp << "                      Colony";
    fp << endl;

    fp << "# Merit...........:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeMerit(j);
    if (print_colony) fp << "   " << setw(12) << setfill(' ') << test_info.GetColonyMerit();
    fp << endl;

    fp << "# Gestation Time..:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeGestation(j);
    if (print_colony) fp << "   " << setw(12) << setfill(' ') << test_info.GetColonyGestation();
    fp << endl;

    fp << "# Fitness.........:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeFitness(j);
    if (print_colony) fp << "   " << setw(12) << setfill(' ') << test_info.GetColonyFitness();
    fp << endl;


    fp << "# Errors..........:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeNumErrors(j);
    fp << endl;

    fp << "# Code Size.......:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeCodeSize(j);
    if (print_colony) fp << "   " << setw(12) << setfill(' ') << test_info.GetColonyCodeSize();
    fp << endl;

    fp << "# Copied Size.....:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeCopiedSize(j);
    if (print_colony) fp << "   " << setw(12) << setfill(' ') << test_info.GetColonyCopiedSize();
    fp << endl;

    fp << "# Executed Size...:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeExeSize(j);
    if (print_colony) fp << "   " << setw(12) << setfill(' ') << test_info.GetColonyExeSize();
    fp << endl;

    fp << "# Offspring.......:";
    if (test_info.GetGenotypeFitness(j) == 0)
      fp << " " << setw(12) << setfill(' ') << "NONE";
    else if (test_info.GetGenotypeCopyTrue(j) == TRUE)
      fp << " " << setw(12) << setfill(' ') << "SELF";
    else if (test_info.GetCycleTo() != -1)
      fp << " " << setw(12) << setfill(' ') << test_info.GetCycleTo();
    else
      fp << " " << setw(12) << setfill(' ') << (j+1);
    fp << endl;


#ifdef THREADS
    fp << endl;

    fp << "# Thread Time Frac:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeThreadFrac(j);
    fp << endl;

    fp << "# Thread Time Diff:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeThreadTimeDiff(j);
    fp << endl;

    fp << "# Thread Code Diff:";
    fp << " " << setw(12) << setfill(' ') << test_info.GetGenotypeThreadCodeDiff(j);
    fp << endl;
#endif

    fp << endl;
  }

  for( i=0; i < test_cpu[0].cpu.phenotype->GetNumTasks(); ++i ){
    fp << "# "<< test_cpu[0].cpu.phenotype->GetTaskLib()->GetTaskName(i)
       << "\t" << test_cpu[0].cpu.phenotype->GetTaskCount(i)
       << endl;
  }
  fp << endl;

  // Display the genome

  const cInstLib & inst_lib = test_cpu[0].cpu.hardware.GetInstLib();
  inst_lib.PrintCode(code, fp);
}

void cTestCPU::TraceCode(const cCodeArray & code, cString filename)
{
  if (code.GetSize() == 0) return;

  // Build the test info for printing.
  cCPUTestInfo test_info;
  test_info.TestThreads();

  test_info.Clear();
  test_info.SetTraceExecution(filename);
  TestCode_Body(test_info, code, 0);
}

int cTestCPU::GetNumTasks()
{
  return test_cpu[0].cpu.phenotype->GetNumTasks();
}

int cTestCPU::GetTaskCount(int task_id)
{
  return test_cpu[0].cpu.phenotype->GetTaskCount(task_id);
}

const cString & cTestCPU::GetTaskKeyword(int task_id)
{
  return test_cpu[0].cpu.phenotype->GetTaskKeyword(task_id);
}


cTestCPU * cTestCPU::test_cpu;
cGenotype cTestCPU::test_genotype[MAX_TEST_DEPTH];
