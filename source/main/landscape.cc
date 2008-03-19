//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1998 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "landscape.hh"
#include "../cpu/test_cpu.hh"

////////////////
//  cLandscape
////////////////

cLandscape::cLandscape(const cCodeArray & in_code, cInstLib * in_inst_lib) {
  inst_lib = in_inst_lib;
  Reset(in_code);
}
 
cLandscape::~cLandscape() {
  delete [] site_count;
}

void cLandscape::Reset(const cCodeArray & in_code)
{
  base_code       = in_code;
  peak_code       = in_code;
  base_fitness    = 0.0;
  base_merit      = 0.0;
  base_gestation  = 0;
  peak_fitness    = 0.0;
  total_fitness   = 0.0;
  total_sqr_fitness = 0.0;
  distance        = 0;
  del_distance    = 0;
  ins_distance    = 0;
  trials          = 0;

  total_count   = 0;
  dead_count    = 0;
  neg_count     = 0;
  neut_count    = 0;
  pos_count     = 0;

  site_count = new int[base_code.GetSize()];
  for (int i = 0; i < base_code.GetSize(); i++) site_count[i] = 0;

  total_entropy = 0.0;
  complexity = 0.0;
  neut_min = 0.0;
  neut_max = 0.0;
}

void cLandscape::HillClimb(ofstream & fp)
{
  cCodeArray cur_code(base_code);
  int gen = 0;
  HillClimb_Body(fp, cur_code, gen);
}

void cLandscape::HillClimb_Body(ofstream & fp, cCodeArray & cur_code, int & gen)
{
  cCodeArray mod_code(base_code);

  const int inst_size = inst_lib->GetSize();
  
  double pos_frac = 1.0;

  int finished = FALSE;
  while (finished == FALSE) {
    if (pos_frac == 0.0) finished = TRUE;

    // Search the landscape for the next best.
    Reset(cur_code);
    const int max_line = cur_code.GetSize();

    // Try all Mutations...
    Process(1);
    
    // Try Insertion Mutations.
    
    mod_code = cur_code;
    for (int line_num = 0; line_num <= max_line; line_num++) {
      // Loop through all instructions...
      for (int inst_num = 0; inst_num < inst_size; inst_num++) {
	mod_code.Insert(line_num, cInstruction(inst_num));
	ProcessCode(mod_code);
	mod_code.Remove(line_num);
      }   
    }

    // Try all deletion mutations.

    for (int line_num = 0; line_num < max_line; line_num++) {
      int cur_inst = cur_code.Get(line_num).GetOp();
      mod_code.Remove(line_num);
      ProcessCode(mod_code);
      mod_code.Insert(line_num, cInstruction(cur_inst));
    }

    pos_frac = GetProbPos();

    // Print the information on the current best.
    HillClimb_Print(fp, cur_code, gen);

    // Move on to the peak code found.
    cur_code = GetPeakCode();
    gen++;
  }
  
  // Write out the best creature to a file:
  cTestCPU::PrintCode(cur_code, "hillclimb-creature");
}

void cLandscape::HillClimb_Print(ofstream & fp, const cCodeArray & _code, const int gen) const
{
  cCPUTestInfo test_info;
  cTestCPU::TestCode(test_info, _code);
  fp << gen << " "
     << test_info.CalcMerit() << " "
     << test_info.CalcGestation() << " "
     << test_info.CalcFitness() << " "
     << _code.GetSize() << " "
     << GetProbDead() << " "
     << GetProbNeg() << " "
     << GetProbNeut() << " "
     << GetProbPos() << " "
     << endl;
  cout << "Hillclimbing: " << gen << " "
     << test_info.CalcMerit() << " "
     << test_info.CalcGestation() << " "
     << test_info.CalcFitness() << " "
     << _code.GetSize() << " "
     << GetProbDead() << " "
     << GetProbNeg() << " "
     << GetProbNeut() << " "
     << GetProbPos() << " "
     << endl;
}

void cLandscape::ProcessCode(cCodeArray & in_code)
{
  cTestCPU::TestCode(test_info, in_code);

  double fitness = test_info.CalcFitness();

  total_fitness += fitness;
  total_sqr_fitness += fitness * fitness;
  
  total_count++;
  if (fitness == 0) dead_count++;
  else if (fitness < neut_min) {
    neg_count++;
  } else if (fitness <= neut_max) {
    neut_count++;
  } else {
    pos_count++;
    if (fitness > peak_fitness) {
      peak_fitness = fitness;
      peak_code = in_code;
    }
  }
}

void cLandscape::ProcessBase()
{
  // Collect info on base creature.

  cTestCPU::TestCode(test_info, base_code);

  base_fitness = test_info.CalcFitness();
  base_merit = test_info.CalcMerit();
  base_gestation = test_info.CalcGestation();
  peak_fitness = base_fitness;
  peak_code = base_code;

  neut_min = base_fitness * LANDSCAPE_NEUTRAL_MIN;
  neut_max = base_fitness * LANDSCAPE_NEUTRAL_MAX;

}

void cLandscape::Process(int in_distance, ostream * fp_mutants)
{
  distance = in_distance;
  
  // Get the info about the base creature.
  ProcessBase();
  
  if (fp_mutants) {
    *fp_mutants << " " << base_fitness <<
                "\n# Instantaneous_Depth  Op_Number  New_Opcode  Fitness\n";
  }

  // Now Process the new creature at the proper distance.
  Process_Body(base_code, distance, 0, fp_mutants);

  // Calculate the complexity...

  double max_ent = log((double) inst_lib->GetSize());
  for (int i = 0; i < base_code.GetSize(); i++) {
    total_entropy += (log((double) site_count[i] + 1) / max_ent);
  }
  complexity = base_code.GetSize() - total_entropy;
}

/** Given a cCodeArray and a distance N, compute the fitness of all N-mutants
 *  of the genotype.  Calls itself recursively when distance is > 1.
 *  If an ostream is passed in, several values will be written out.
 */
void cLandscape::Process_Body(cCodeArray & cur_code, int cur_distance,
			      int start_line, ostream * fp_mutants)
{
  const int max_line = base_code.GetSize() - cur_distance + 1;
  const int inst_size = inst_lib->GetSize();
  double currFitness = 0;
  
  cCodeArray mod_code(cur_code);

  // Loop through all the lines of code, testing trying all combinations.
  for (int line_num = start_line; line_num < max_line; line_num++) {
    int cur_inst = base_code.Get(line_num).GetOp();
    
    // Loop through all instructions...
    for (int inst_num = 0; inst_num < inst_size; inst_num++) {
      if (cur_inst == inst_num) continue;
      
      mod_code[line_num].SetOp(inst_num);
      if (cur_distance <= 1) {
	ProcessCode(mod_code);
        currFitness = test_info.CalcFitness();
        // If we've got a file handle, write out some info
        if ( fp_mutants )
          (*fp_mutants) << cur_distance << " " << line_num <<
          " " << inst_lib->GetName(mod_code.Get(line_num)) << " " << 
          currFitness << endl;
	if (currFitness >= neut_min) site_count[line_num]++;
      } else {
        // I added the following code to print out intermediate fitness
        // values.  I'm a bit concerned that ProcessCode or CalcFitness
        // might have unwanted side-effects, so if I forget to delete this
        // you probably should. -- NG
        ProcessCode(mod_code);   // !!!DELETE THIS!!!
        currFitness = test_info.CalcFitness();   /// !!!DELETE THIS!!!
        if ( fp_mutants ) {
          (*fp_mutants) << cur_distance << " " << line_num <<
          " " << inst_lib->GetName(mod_code.Get(line_num)) << " " <<
          currFitness << endl; // This is OK and probably useful
        }
        // End NG added code
	Process_Body(mod_code, cur_distance - 1, line_num + 1, fp_mutants);
      }
    }   

    mod_code[line_num].SetOp(cur_inst);
  }

}

void cLandscape::ProcessDelete()
{
  // Get the info about the base creature.
  ProcessBase();

  const int max_line = base_code.GetSize();
  cCodeArray mod_code(base_code);

  // Loop through all the lines of code, testing trying all combinations.
  for (int line_num = 0; line_num < max_line; line_num++) {
    int cur_inst = base_code.Get(line_num).GetOp();
    mod_code.Remove(line_num);
    ProcessCode(mod_code);
    mod_code.Insert(line_num, cInstruction(cur_inst));
  }

}

void cLandscape::ProcessInsert()
{
  // Get the info about the base creature.
  ProcessBase();

  const int max_line = base_code.GetSize();
  const int inst_size = inst_lib->GetSize();
  
  cCodeArray mod_code(base_code);

  // Loop through all the lines of code, testing trying all combinations.
  for (int line_num = 0; line_num <= max_line; line_num++) {
    // Loop through all instructions...
    for (int inst_num = 0; inst_num < inst_size; inst_num++) {
      mod_code.Insert(line_num, cInstruction(inst_num));
      ProcessCode(mod_code);
      mod_code.Remove(line_num);
    }   
  }

}

void cLandscape::SampleProcess(int in_trials)
{
  trials = in_trials;  // Trials _per line_
  distance = 1;

  cCodeArray mod_code(base_code);
  int code_size = base_code.GetSize();

  ProcessBase();
  
  // Loop through all the lines of code, testing each line.
  for (int line_num = 0; line_num < code_size; line_num++) {
    cInstruction cur_inst( base_code.Get(line_num) );

    for (int i = 0; i < trials; i++) {
      // Choose the new instruction for that line...
      cInstruction new_inst( inst_lib->GetRandomInst() );
      if (inst_lib->Compare(cur_inst, new_inst) == TRUE) { i--; continue; }

      // Make the change, and test it!
      mod_code[line_num] = new_inst;
      ProcessCode(mod_code);
    }

    mod_code[line_num] = cur_inst;
  }
}


void cLandscape::RandomProcess(int in_trials, int in_distance, int min_found,
			       int max_trials, int print_if_found)
{
  static int last_update = -1;
  static int process_OK = TRUE;
  if (last_update != cStats::GetUpdate()) {
    last_update = cStats::GetUpdate();
    process_OK = TRUE;
  }

  if (process_OK == FALSE) {
    distance = in_distance;
    dead_count = 1;
    total_count = 1;
    trials = 0;
    return;
  }


  distance = in_distance;
  
  cCodeArray mod_code(base_code);
  int code_size = base_code.GetSize();

  ProcessBase();

  int mut_num;
  tArray<UINT> mut_lines(distance);

  // Loop through all the lines of code, testing many combinations.
  int cur_trial = 0;
  int total_found = 0;

  for (cur_trial = 0;
       (cur_trial < in_trials) ||
	 (total_found < min_found && cur_trial < max_trials);
       cur_trial++) {
    // Choose the lines to mutate...
    g_random.Choose(code_size, mut_lines);

    // Choose the new instructions for those lines...
    for (mut_num = 0; mut_num < distance; mut_num++) {
      const cInstruction new_inst( inst_lib->GetRandomInst() );
      const cInstruction & cur_inst = base_code.Get(mut_lines[mut_num]);
      if (inst_lib->Compare(cur_inst, new_inst) == TRUE) {
	mut_num--;
	continue;
      }

      mod_code.Set(mut_lines[mut_num], new_inst);
    }

    // And test it!

    ProcessCode(mod_code);

    if (test_info.IsViable()) {
      total_found++;
      // If we are supposed to print those found, do so!    
      if (print_if_found) {
	cString filename;
	filename.Set("creature.land.%d.%d", distance, cur_trial);
	cTestCPU::PrintCode(mod_code, filename);
      }
    }


    // And reset the code.
    for (mut_num = 0; mut_num < distance; mut_num++) {
      mod_code.Set(mut_lines[mut_num],
			     base_code[mut_lines[mut_num]]);
    }
  }

  trials = cur_trial;

  if (total_found < min_found) { process_OK = FALSE; }
}

void cLandscape::BuildFitnessChart()
{
  // First, resize the fitness_chart.
  const int max_line = base_code.GetSize();
  const int inst_size = inst_lib->GetSize();
  fitness_chart.ResizeClear(max_line, inst_size);
  
  cCodeArray mod_code(base_code);

  // Loop through all the lines of code, testing trying all combinations.
  for (int line_num = 0; line_num < max_line; line_num++) {
    int cur_inst = base_code.Get(line_num).GetOp();
    
    // Loop through all instructions...
    for (int inst_num = 0; inst_num < inst_size; inst_num++) {
      if (cur_inst == inst_num) {
	fitness_chart(line_num, inst_num) = 0.0;
	continue;
      }
      
      mod_code[line_num].SetOp(inst_num);
      ProcessCode(mod_code);
      fitness_chart(line_num, inst_num) = test_info.CalcFitness();
    }   

    mod_code[line_num].SetOp(cur_inst);
  }

}

void cLandscape::TestPairs(int in_trials, ostream & fp)
{
  trials = in_trials;
  
  ProcessBase();
  if (base_fitness == 0.0) return;

  BuildFitnessChart();

  cCodeArray mod_code(base_code);
  const int code_size = base_code.GetSize();

  tArray<UINT> mut_lines(2);
  tArray<cInstruction> mut_insts(2);

  // Loop through all the lines of code, testing many combinations.
  for (int i = 0; i < trials; i++) {
    // Choose the lines to mutate...
    g_random.Choose(code_size, mut_lines);

    // Choose the new instructions for those lines...
    for (int mut_num = 0; mut_num < 2; mut_num++) {
      const cInstruction new_inst( inst_lib->GetRandomInst() );
      const cInstruction & cur_inst = base_code.Get(mut_lines[mut_num]);
      if (inst_lib->Compare(cur_inst, new_inst) == TRUE) {
	mut_num--;
	continue;
      }

      mut_insts[mut_num] = new_inst;
    }

    TestMutPair(mod_code, mut_lines[0], mut_lines[1], mut_insts[0],
		mut_insts[1], fp);

  }
  
}


void cLandscape::TestAllPairs(ostream & fp)
{
  ProcessBase();
  if (base_fitness == 0.0) return;

  BuildFitnessChart();

  const int max_line = base_code.GetSize();
  const int inst_size = inst_lib->GetSize();
  cCodeArray mod_code(base_code);
  cInstruction inst1, inst2;

  // Loop through all the lines of code, testing trying all combinations.
  for (int line1_num = 0; line1_num < max_line - 1; line1_num++) {
    for (int line2_num = line1_num + 1; line2_num < max_line; line2_num++) {
    
      // Loop through all instructions...
      for (int inst1_num = 0; inst1_num < inst_size; inst1_num++) {
	inst1.SetOp(inst1_num);
	if (inst1 == base_code[line1_num]) continue;
	for (int inst2_num = 0; inst2_num < inst_size; inst2_num++) {
	  inst2.SetOp(inst2_num);
	  if (inst2 == base_code[line2_num]) continue;
	  TestMutPair(mod_code, line1_num, line2_num, inst1, inst2, fp);
	} // inst2_num loop
      } //inst1_num loop;

    } // line2_num loop
  } // line1_num loop.

}


void cLandscape::TestMutPair(cCodeArray & mod_code, int line1, int line2,
    const cInstruction & mut1, const cInstruction & mut2, ostream & fp)
{
  mod_code.Set(line1, mut1);
  mod_code.Set(line2, mut2);
  cTestCPU::TestCode(test_info, mod_code);
  double combo_fitness = test_info.CalcFitness() / base_fitness;
  mod_code.Set(line1, base_code[line1]);
  mod_code.Set(line2, base_code[line2]);

  /*  
  fp << line1        << " "
     << line2        << " "
     << ( (int) mut1.GetOp() ) << " "
     << ( (int) mut2.GetOp() ) << " ";
  */

  fp << ( fitness_chart(line1, mut1.GetOp()) / base_fitness ) << " "
     << ( fitness_chart(line2, mut2.GetOp()) / base_fitness ) << " "
     << combo_fitness << endl;
}


void cLandscape::PrintStats(ofstream & fp)
{
  fp << cStats::GetUpdate()    << " "   // 1
     << GetProbDead()          << " "   // 2
     << GetProbNeg()           << " "   // 3
     << GetProbNeut()          << " "   // 4
     << GetProbPos()           << " "   // 5
     << total_count            << " "   // 6
     << distance               << " "   // 7
     << base_fitness           << " "   // 8
     << base_merit             << " "   // 9
     << base_gestation         << " "   // 10
     << peak_fitness           << " "   // 11
     << GetAveFitness()        << " "   // 12
     << GetAveSqrFitness()     << " "   // 13
     << total_entropy          << " "   // 14
     << complexity             << endl; // 15
  fp.flush();
}

void cLandscape::PrintEntropy(ofstream & fp)
{
  double max_ent = log((double) inst_lib->GetSize());
  for (int j = 0; j < base_code.GetSize(); j++) {
    fp << (log((double) site_count[j] + 1) / max_ent) << " ";
  }
  fp << endl;
  fp.flush();
}

void cLandscape::PrintSiteCount(ofstream & fp)
{
  for (int j = 0; j < base_code.GetSize(); j++) {
    fp << (site_count[j] + 1) << " ";
  }
  fp << endl;
  fp.flush();
}


void cLandscape::PrintBase(cString filename)
{
  cTestCPU::PrintCode(base_code, filename);
}

void cLandscape::PrintPeak(cString filename)
{
  cTestCPU::PrintCode(peak_code, filename);
}

