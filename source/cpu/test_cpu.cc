//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "test_cpu.hh"

#include "../tools/tMatrix.hh"
#include "../tools/string_util.hh"
#include "../tools/functions.hh"

#include "hardware_method.hh"
#include "hardware_base.hh"

#include "../main/config.hh"
#include "../main/environment.hh"
#include "../main/inst_set.hh"
#include "../main/inst_util.hh"
#include "../main/organism.hh"
#include "../main/phenotype.hh"
#include "../main/pop_interface.hh"
#include "../main/resource_count.hh"

#include <iomanip>


using namespace std;


// Static Variables
cInstSet * cTestCPU::inst_set(NULL);
cEnvironment * cTestCPU::environment(NULL);
cPopulationInterface cTestCPU::test_interface;
tArray<int> cTestCPU::input_array;
int cTestCPU::cur_input;
cResourceCount * cTestCPU::resource_count(NULL);

bool cTestCPU::initialized(false);

/////////////////
// cCPUTestInfo
/////////////////

cCPUTestInfo::cCPUTestInfo(int max_tests)
  : generation_tests(max_tests)  // These vars not reset on Clear()
  , test_threads(false)
  , print_threads(false)
  , trace_execution(false)
  , trace_task_order(false)
  , use_random_inputs(false)
  , org_array(max_tests)
{
  org_array.SetAll(NULL);
  Clear();
}


cCPUTestInfo::~cCPUTestInfo()
{
  for (int i = 0; i < generation_tests; i++) {
    if (org_array[i] != NULL) delete org_array[i];
  }
}


void cCPUTestInfo::Clear()
{
  is_viable = false;
  max_depth = -1;
  depth_found = -1;
  max_cycle = 0;
  cycle_to = -1;

  for (int i = 0; i < generation_tests; i++) {
    if (org_array[i] == NULL) break;
    delete org_array[i];
    org_array[i] = NULL;
  }
}
 

void cCPUTestInfo::SetTraceExecution(const cString & filename)
{
  trace_execution = true;
  trace_fp.open(filename);
  assert (trace_fp.good() == true); // Unable to open trace file.
}


double cCPUTestInfo::GetGenotypeFitness()
{
  if (org_array[0] != NULL) return org_array[0]->GetPhenotype().GetFitness();
  return 0.0;
}


double cCPUTestInfo::GetColonyFitness()
{
  if (IsViable()) return GetColonyOrganism()->GetPhenotype().GetFitness();
  return 0.0;
}


//////////////////////////////
//  cTestCPU  (Static Class)
//////////////////////////////

void cTestCPU::Setup(cInstSet * in_inst_set,  cEnvironment * in_env,
		     const cPopulationInterface & in_interface)
{
  inst_set = in_inst_set;
  environment = in_env;
  test_interface = in_interface;

  resource_count = new cResourceCount(environment->GetResourceLib().GetSize());

  initialized = true;
}

void cTestCPU::SetInstSet(cInstSet * in_inst_set)
{
  inst_set = in_inst_set;
}

// NOTE: This method assumes that the organism is a fresh creation.
bool cTestCPU::ProcessGestation(cCPUTestInfo & test_info, int cur_depth)
{
  assert(initialized == true);
  assert(test_info.org_array[cur_depth] != NULL);

  cOrganism & organism = *( test_info.org_array[cur_depth] );

  // Determine how long this organism should be tested for...
  int time_allocated = cConfig::GetTestCPUTimeMod() *
    organism.GetGenome().GetSize();

  // Make sure this genome stands a chance...
  if (TestIntegrity(organism.GetGenome()) == false)  time_allocated = 0;

  // Prepare the inputs...
  cur_input = 0;

  // Determine if we're tracing and what we need to print.
  ostream * trace_fp =
    test_info.GetTraceExecution() ? &(test_info.GetTraceFP()) : NULL;

  int time_used = 0;
  while (time_used < time_allocated &&
	 organism.GetHardware().GetMemory().GetSize() &&
	 organism.GetPhenotype().GetNumDivides() == 0) {
    time_used++;
    organism.GetHardware().SingleProcess(trace_fp);
    // @CAO Need to watch out for parasites.
  }

  // For now, always return true.
  return true;
}


bool cTestCPU::TestGenome(cCPUTestInfo & test_info, const cGenome & genome)
{
  assert(initialized == true);

  test_info.Clear();
  TestGenome_Body(test_info, genome, 0);

  return test_info.is_viable;
}

bool cTestCPU::TestGenome(cCPUTestInfo & test_info, const cGenome & genome,
		       ofstream & out_fp)
{
  assert(initialized == true);

  test_info.Clear();
  TestGenome_Body(test_info, genome, 0);

  ////////////////////////////////////////////////////////////////
  // IsViable() == false
  //   max_depth == 0  : (0) Parent doesn't divide
  //   max_depth == 1  : (2) Parent does divide, but child does not.
  //   max_depth >= 2  : (3) Parent and child do divide, but neither true.
  // ------------------------------------------------------------
  // IsViable() == true
  //   max_depth == 0  : (4) Parent Breed True
  //   max_depth == 1  : (5) Parent NOT Breed True, but Child Does
  //   max_depth >= 2  : (6) Multiple levels of non-breed true.
  ////////////////////////////////////////////////////////////////


  const int depth_comp = Min(test_info.max_depth, 2);
  int repro_type = ((int) test_info.is_viable) * 3 + 1 + depth_comp;
  if (test_info.is_viable == false && depth_comp == 0)  repro_type = 0;

  out_fp << test_info.is_viable << " "
	 << test_info.max_depth << " "
	 << test_info.depth_found << " "
	 << test_info.max_cycle << " "
	 << repro_type << endl;

  return test_info.is_viable;
}


bool cTestCPU::TestGenome_Body(cCPUTestInfo & test_info,
			       const cGenome & genome, int cur_depth)
{
  assert(initialized == true);
  assert(cur_depth < test_info.generation_tests);

  if (test_info.GetUseRandomInputs() == false) {
    // We make sure that all combinations of inputs are present.  This is
    // done explicitly in the key columns... (0f, 33, and 55)
    input_array.Resize(3);
    //    input_array[0] = 0x130f149f;  // 00010011 00001111 00010100 10011111
    //    input_array[1] = 0x0833e53e;  // 00001000 00110011 11100101 00111110
    //    input_array[2] = 0x625541eb;  // 01100010 01010101 01000001 11101011

    input_array[0] = 0x0f13149f;  // 00001111 00010011 00010100 10011111
    input_array[1] = 0x3308e53e;  // 00110011 00001000 11100101 00111110
    input_array[2] = 0x556241eb;  // 01010101 01100010 01000001 11101011
  } else {
    environment->SetupInputs(input_array);
  }

  if (cur_depth > test_info.max_depth) test_info.max_depth = cur_depth;

  // Setup the organism we're working with now.
  if (test_info.org_array[cur_depth] != NULL) {
    delete test_info.org_array[cur_depth];
  }
  test_info.org_array[cur_depth] =
    new cOrganism(genome, test_interface, *environment);
  cOrganism & organism = *( test_info.org_array[cur_depth] );
  organism.GetPhenotype().SetupInject(genome.GetSize());

  // Run the current organism.
  ProcessGestation(test_info, cur_depth);

  // Must be able to divide twice in order to form a successful colony,
  // assuming the CPU doesn't get reset on divides.
  //
  // The possibilities after this gestation cycle are:
  //  1: It did not copy at all => Exit this level.
  //  2: It copied true => Check next gestation cycle, or set is_viable.
  //  3: Its copy looks like an ancestor => copy true.
  //  4: It copied false => we must check the child.

  // Case 1:  ////////////////////////////////////
  if (organism.GetPhenotype().GetNumDivides() == 0)  return false;

  // Case 2:  ////////////////////////////////////
  if (organism.GetPhenotype().CopyTrue() == true) {
    test_info.depth_found = cur_depth;
    test_info.is_viable = true;
    return true;
  }

  // Case 3:  ////////////////////////////////////
  bool is_ancestor = false;
  for (int anc_depth = 0; anc_depth < cur_depth; anc_depth++) {
    if (organism.ChildGenome() == test_info.org_array[anc_depth]->GetGenome()){
      is_ancestor = true;
      const int cur_cycle = cur_depth - anc_depth;
      if (test_info.max_cycle < cur_cycle) test_info.max_cycle = cur_cycle;
      test_info.cycle_to = anc_depth;
    }
  }
  if (is_ancestor) {
    test_info.depth_found = cur_depth;
    test_info.is_viable = true;
    return true;
  }

  // Case 4:  ////////////////////////////////////
  // If we haven't reached maximum depth yet, check out the child.
  if (cur_depth+1 < test_info.generation_tests) {
    // Run the child's genome.
    return TestGenome_Body(test_info, organism.ChildGenome(), cur_depth+1);
  }


  // All options have failed; just return false.
  return false;
}




void cTestCPU::TraceGenome(const cGenome & genome, cString filename)
{
  assert(initialized == true);

  if (genome.GetSize() == 0) return;

  // Build the test info for printing.
  cCPUTestInfo test_info;
  test_info.TestThreads();

  test_info.Clear();
  test_info.SetTraceExecution(filename);
  TestGenome_Body(test_info, genome, 0);
}


void cTestCPU::TestThreads(const cGenome & genome)
{
  assert(initialized == true);

  static ofstream fp("threads.dat");

  cCPUTestInfo test_info;
  test_info.TestThreads();
  cTestCPU::TestGenome(test_info, genome);

//  fp << cStats::GetUpdate()             << " "    // 1
//     << genome.GetSize()                << " ";   // 2
//       << cStats::GetAveNumThreads()      << " "   // 3
//       << cStats::GetAveThreadDist()      << " ";  // 4

//    fp << test_info.GetGenotypeMerit()          << " "   // 5
//       << test_info.GetGenotypeGestation()      << " "   // 6
//       << test_info.GetGenotypeFitness()        << " "   // 7
//       << test_info.GetGenotypeThreadFrac()     << " "   // 8
//       << test_info.GetGenotypeThreadTimeDiff() << " "   // 9
//       << test_info.GetGenotypeThreadCodeDiff() << " ";  // 10

//    fp << test_info.GetColonyMerit()          << " "   // 11
//       << test_info.GetColonyGestation()      << " "   // 12
//       << test_info.GetColonyFitness()        << " "   // 13
//       << test_info.GetColonyThreadFrac()     << " "   // 14
//       << test_info.GetColonyThreadTimeDiff() << " "   // 15
//       << test_info.GetColonyThreadCodeDiff() << " ";  // 16

  fp << endl;
}


void cTestCPU::PrintThreads(const cGenome & genome)
{
  assert(initialized == true);

  cCPUTestInfo test_info;
  test_info.TestThreads();
  test_info.PrintThreads();
  cTestCPU::TestGenome(test_info, genome);
}


bool cTestCPU::TestIntegrity(const cGenome & test_genome)
{
#ifdef QUICK_BASE_TEST_CPU
  // This checks to make sure a 'copy', 'divide', and 'allocate' are all in
  // the creatures, and if not doesn't even bother to test it.
  static UCHAR copy_id  = inst_set->GetInstID("copy");
  static UCHAR div_id   = inst_set->GetInstID("divide");
  static UCHAR alloc_id = inst_set->GetInstID("allocate");
#endif

#ifdef QUICK_HEAD_TEST_CPU
  // This checks to make sure a 'copy', 'divide', and 'allocate' are all in
  // the creatures, and if not doesn't even bother to test it.
  static UCHAR copy_id  = inst_set->GetInstID("h-copy");
  static UCHAR div_id   = inst_set->GetInstID("h-divide");
  static UCHAR alloc_id = inst_set->GetInstID("h-alloc");
#endif


#ifdef QUICK_TEST_CPU
  bool copy_found = false;
  bool div_found = false;
  bool alloc_found = false;

  for (int i = 0; i < test_genome.GetSize(); i++) {
    if (test_genome[i].GetOp() == copy_id)  copy_found  = true;
    if (test_genome[i].GetOp() == div_id)   div_found   = true;
    if (test_genome[i].GetOp() == alloc_id) alloc_found = true;
  }

  if (copy_found == false || div_found == false || alloc_found == false) {
    return false;
  }
#endif

  return true;
}



int cTestCPU::GetInput()
{
  if (cur_input >= input_array.GetSize()) cur_input = 0;
  return input_array[cur_input++];
}

int cTestCPU::GetInputAt(int & input_pointer)
{
  if (input_pointer >= input_array.GetSize()) input_pointer = 0;
  return input_array[input_pointer++];
}

const tArray<double> & cTestCPU::GetResources()
{
  assert(resource_count != NULL);
  return resource_count->GetResources();
}

void cTestCPU::UpdateResources(const tArray<double> & res_change)
{
  // Nada for now...
}

