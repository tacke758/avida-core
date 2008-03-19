//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "config.hh"

#include "../tools/tools.hh"
#include "cPopulation_descr.hi" // declarations and definitions
#include "cPopulation_descr.ci" // for event documentation


using namespace std;


bool cConfig::analyze_mode;
bool cConfig::primitive_mode;
cString cConfig::default_dir;
cString cConfig::genesis_filename;
cString cConfig::inst_filename;
cString cConfig::event_filename;
cString cConfig::analyze_filename;
cString cConfig::env_filename;
cString cConfig::start_creature;
cString cConfig::clone_filename;
cString cConfig::load_pop_filename;
int cConfig::max_updates;
int cConfig::max_generations;
int cConfig::end_condition_mode;
int cConfig::world_x;
int cConfig::world_y;
int cConfig::rand_seed;
double cConfig::point_mut_prob;
double cConfig::copy_mut_prob;
double cConfig::ins_mut_prob;
double cConfig::del_mut_prob;
double cConfig::div_mut_prob;
double cConfig::divide_mut_prob;
double cConfig::divide_ins_prob;
double cConfig::divide_del_prob;
double cConfig::parent_mut_prob;
double cConfig::crossover_prob;
double cConfig::aligned_cross_prob;
double cConfig::exe_err_prob;
int cConfig::num_instructions;
int cConfig::hardware_type;
int cConfig::max_cpu_threads;
int cConfig::thread_slicing_method;
int cConfig::size_merit_method;
int cConfig::base_size_merit;
int cConfig::task_merit_method;
int cConfig::max_label_exe_size;
int cConfig::max_num_tasks_rewarded;
int cConfig::merit_time;
int cConfig::num_tasks;
int cConfig::num_reactions;
int cConfig::num_resources;
int cConfig::slicing_method;
int cConfig::birth_method;
int cConfig::death_method;
int cConfig::alloc_method;
int cConfig::divide_method;
int cConfig::required_task;
int cConfig::lineage_creation_method;
int cConfig::generation_inc_method;
int cConfig::age_limit;
double cConfig::age_deviation;
double cConfig::child_size_range;
double cConfig::min_copied_lines;
double cConfig::min_exe_lines;
int cConfig::require_allocate;
bool cConfig::test_on_divide;
double cConfig::revert_fatal;
double cConfig::revert_neg;
double cConfig::revert_neut;
double cConfig::revert_pos;
double cConfig::sterilize_fatal;
double cConfig::sterilize_neg;
double cConfig::sterilize_neut;
double cConfig::sterilize_pos;
int cConfig::fail_implicit;
int cConfig::ave_time_slice;
int cConfig::species_threshold;
int cConfig::threshold;
int cConfig::genotype_print;
int cConfig::species_print;
int cConfig::species_recording;
int cConfig::genotype_print_dom;
int cConfig::test_cpu_time_mod;
int cConfig::track_main_lineage;
bool cConfig::log_threshold_only;
bool cConfig::log_breed_count;
bool cConfig::log_creatures;
bool cConfig::log_phylogeny;
bool cConfig::log_genotypes;
bool cConfig::log_threshold;
bool cConfig::log_species;
bool cConfig::log_landscape;
bool cConfig::log_mutations;
bool cConfig::log_lineages;
int cConfig::debug_level;
int cConfig::view_mode;
int cConfig::viewer_type;



void cConfig::Setup(int argc, char * argv[])
{
  default_dir = DEFAULT_DIR;

  cGenesis genesis;
  genesis.SetVerbose();
  ProcessConfiguration(argc, argv, genesis);

  // Load the default directory...
  default_dir = genesis.ReadString("DEFAULT_DIR", DEFAULT_DIR);
  char dir_tail = default_dir[default_dir.GetSize() - 1];
  if (dir_tail != '\\' && dir_tail != '/') default_dir += "/";

  // Input files...
  inst_filename  = genesis.ReadString("INST_SET", "inst_set");
  event_filename = genesis.ReadString("EVENT_FILE", "events.cfg");
  analyze_filename = genesis.ReadString("ANALYZE_FILE", "analyze.cfg");
  env_filename = genesis.ReadString("ENVIRONMENT_FILE", "environment.cfg");
  start_creature = genesis.ReadString("START_CREATURE");


  // Load Archetecture...
  max_updates     = genesis.ReadInt("MAX_UPDATES", -1);
  max_generations = genesis.ReadInt("MAX_GENERATIONS", -1);
  end_condition_mode = genesis.ReadInt("END_CONDITION_MODE", 0);
  world_x         = genesis.ReadInt("WORLD-X");
  world_y         = genesis.ReadInt("WORLD-Y");
  hardware_type   = genesis.ReadInt("HARDWARE_TYPE");

  birth_method   = genesis.ReadInt("BIRTH_METHOD", POSITION_CHILD_AGE);
  death_method   = genesis.ReadInt("DEATH_METHOD", DEATH_METHOD_OFF);
  alloc_method   = genesis.ReadInt("ALLOC_METHOD", ALLOC_METHOD_DEFAULT);
  divide_method  = genesis.ReadInt("DIVIDE_METHOD", DIVIDE_METHOD_SPLIT);
  required_task   = genesis.ReadInt("REQUIRED_TASK", -1);
  lineage_creation_method =
    genesis.ReadInt("LINEAGE_CREATION_METHOD", 0);
  generation_inc_method =
    genesis.ReadInt("GENERATION_INC_METHOD", GENERATION_INC_BOTH);
  age_limit      = genesis.ReadInt("AGE_LIMIT", -1);
  age_deviation    = genesis.ReadFloat("AGE_DEVIATION", 0);
  child_size_range = genesis.ReadFloat("CHILD_SIZE_RANGE", 2.0);
  min_copied_lines = genesis.ReadFloat("MIN_COPIED_LINES", 0.5);
  min_exe_lines    = genesis.ReadFloat("MIN_EXE_LINES", 0.5);
  require_allocate = genesis.ReadInt("REQUIRE_ALLOCATE", 1);

  revert_fatal = genesis.ReadFloat("REVERT_FATAL", 0.0);
  revert_neg   = genesis.ReadFloat("REVERT_DETRIMENTAL", 0.0);
  revert_neut  = genesis.ReadFloat("REVERT_NEUTRAL", 0.0);
  revert_pos   = genesis.ReadFloat("REVERT_BENEFICIAL", 0.0);
  sterilize_fatal = genesis.ReadFloat("STERILIZE_FATAL", 0.0);
  sterilize_neg   = genesis.ReadFloat("STERILIZE_DETRIMENTAL", 0.0);
  sterilize_neut  = genesis.ReadFloat("STERILIZE_NEUTRAL", 0.0);
  sterilize_pos   = genesis.ReadFloat("STERILIZE_BENEFICIAL", 0.0);
  test_on_divide = (revert_fatal > 0.0) || (revert_neg > 0.0) ||
    (revert_neut > 0.0) || (revert_pos > 0.0) || (sterilize_fatal > 0.0) ||
    (sterilize_neg > 0.0) || (sterilize_neut > 0.0) || (sterilize_pos > 0.0);
  fail_implicit = genesis.ReadInt("FAIL_IMPLICIT", 0);

  // Genealogy
  species_threshold  = genesis.ReadInt("SPECIES_THRESHOLD");
  threshold          = genesis.ReadInt("THRESHOLD");
  genotype_print     = genesis.ReadInt("GENOTYPE_PRINT");
  species_print      = genesis.ReadInt("SPECIES_PRINT");
  species_recording  = genesis.ReadInt("SPECIES_RECORDING");
  genotype_print_dom = genesis.ReadInt("GENOTYPE_PRINT_DOM");
  test_cpu_time_mod  = genesis.ReadInt("TEST_CPU_TIME_MOD", 20);
  track_main_lineage = genesis.ReadInt("TRACK_MAIN_LINEAGE", 0);

  // Thread Info
  max_cpu_threads = genesis.ReadInt("MAX_CPU_THREADS", 1);
  thread_slicing_method = genesis.ReadInt("THREAD_SLICING_METHOD", 0);
  

  // Time Slicing Info
  slicing_method = genesis.ReadInt("SLICING_METHOD", SLICE_CONSTANT);
  size_merit_method = genesis.ReadInt("SIZE_MERIT_METHOD", 0);
  base_size_merit   = genesis.ReadInt("BASE_SIZE_MERIT", 0);
  ave_time_slice = genesis.ReadInt("AVE_TIME_SLICE", 30);
  merit_time = genesis.ReadInt("MERIT_TIME", 0);

  // Task Merit Method
  task_merit_method = genesis.ReadInt("TASK_MERIT_METHOD", TASK_MERIT_NORMAL);
  max_num_tasks_rewarded = genesis.ReadInt("MAX_NUM_TASKS_REWARDED", -1);
  max_label_exe_size = genesis.ReadInt("MAX_LABEL_EXE_SIZE", 1);

  // Load Mutation Info
  point_mut_prob  = genesis.ReadFloat("POINT_MUT_PROB");
  copy_mut_prob   = genesis.ReadFloat("COPY_MUT_PROB");
  ins_mut_prob    = genesis.ReadFloat("INS_MUT_PROB");
  del_mut_prob    = genesis.ReadFloat("DEL_MUT_PROB");
  div_mut_prob    = genesis.ReadFloat("DIV_MUT_PROB");
  divide_mut_prob = genesis.ReadFloat("DIVIDE_MUT_PROB");
  divide_ins_prob = genesis.ReadFloat("DIVIDE_INS_PROB");
  divide_del_prob = genesis.ReadFloat("DIVIDE_DEL_PROB");
  parent_mut_prob = genesis.ReadFloat("PARENT_MUT_PROB");
  crossover_prob  = genesis.ReadFloat("CROSSOVER_PROB");
  aligned_cross_prob = genesis.ReadFloat("ALIGNED_CROSS_PROB");
  exe_err_prob    = genesis.ReadFloat("EXE_ERROR_PROB");

  // Load Viewer Info...
  view_mode = genesis.ReadInt("VIEW_MODE");
  viewer_type = genesis.ReadInt("VIEWER_TYPE",0);

  log_breed_count = genesis.ReadInt("LOG_BREED_COUNT", 0);
  log_creatures   = genesis.ReadInt("LOG_CREATURES", 0);
  log_phylogeny   = genesis.ReadInt("LOG_PHYLOGENY", 0);

  log_genotypes = genesis.ReadInt("LOG_GENOTYPES", 0);
  log_threshold_only = false;
  if (log_genotypes > 1) log_threshold_only = true;

  log_threshold = genesis.ReadInt("LOG_THRESHOLD", 0);
  log_species   = genesis.ReadInt("LOG_SPECIES", 0);
  log_landscape = genesis.ReadInt("LOG_LANDSCAPE", 0);
  log_mutations = genesis.ReadInt("LOG_MUTATIONS", 0);
  log_lineages = genesis.ReadInt("LOG_LINEAGES", 0);

  genesis.WarnUnused();
}

void cConfig::SetupMS()
{
#ifdef PLATFORM_WINDOWS
  OSVERSIONINFO	osVersion;

  SetFileApisToOEM();             // use console character set for files
  memset (&osVersion, 0, sizeof (osVersion));
  osVersion.dwOSVersionInfoSize = sizeof (osVersion);
  GetVersionEx (&osVersion);
  if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) {
    // Only need to set Code Pages for NT. Default console on Win95 is OEM.
    // In fact, these APIs fail on Win95 w/ rc = 120 (API only works in NT
    //  mode).
    if (!SetConsoleCP (GetOEMCP()))        // and for Console input
      cerr << "WARNING: Unable to set input character set, rc="
	   << GetLastError() << endl;
    if (!SetConsoleOutputCP (GetOEMCP()))  // and for Console output
      cerr << "WARNING: Unable to set output character set, rc="
	   << GetLastError() << endl;
  }
#endif
}


//  void cConfig::PerturbTaskSet(double max_factor)
//  {
//    default_task_lib->Perturb(max_factor);
//  }

//  void cConfig::ChangeTaskSet(double prob_change, double min_bonus,
//  			    double max_bonus)
//  {
//    default_task_lib->Change(prob_change, min_bonus, max_bonus);
//  }


// This function takes in the genesis file and the input arguments, and puts
// out a pointer to a built-up cGenesis object.

// @COW This function depends on the inclusion of the file
// '../event/cPopulation_descr.ci' for the automatic event documentation.
// If you move the function away from here, move this include as well.


void cConfig::ProcessConfiguration(int argc, char * argv[], cGenesis & genesis)
{
  genesis_filename = "genesis"; // Name of genesis file.
  int arg_num = 1;              // Argument number being looked at.
  analyze_mode = false;         // Initialize analyze_mode tp be off.
  primitive_mode = false;       // Initialize primitive_mode tp be off.

  // Load all of the args into string objects for ease of access.
  cString * args = new cString[argc];
  for (int i = 0; i < argc; i++) args[i] = argv[i];


  // -genesis option

  if (argc > 1 && (args[1] == "-g" || args[1] == "-genesis")) {
    if (argc < 3) {
      cerr << "Need filename for genesis file used." << endl;
      exit(0);
    }
    genesis_filename = args[2];
    arg_num += 2;
  }


  // Open and verify the genesis file.

  genesis.Open(genesis_filename);
  cString version_id = genesis.ReadString("VERSION_ID", "Unknown");
  if (genesis.IsOpen() == true && version_id != AVIDA_VERSION) {
    cerr << "/  WARNING   WARNING   WARNING   WARNING   WARNING  \\" << endl
	 << "|   Using incorrect genesis file.                   |" << endl
	 << "|   Version needed = \"" << AVIDA_VERSION
	 << "\".  Version used = \"" << version_id() << "\"   |" << endl
	 << "\\  WARNING   WARNING   WARNING   WARNING   WARNING  /" << endl
	 << endl;
  }

  // Then scan through and process the rest of the args.

  while (arg_num < argc || genesis.IsOpen() == false) {
    cString cur_arg = genesis.IsOpen() ? static_cast<cString>( args[arg_num] )
		      : static_cast<cString>( "--help" );

    // Test against the possible inputs.
    if (cur_arg == "-events" || cur_arg == "-e") {
      cout << "Known events:" << endl;
      for ( int i=0; i<cEventDescrs::num_of_events; i++ ){
	cout << "-----  "
	     << cEventDescrs::entries[i].GetName()
	     << "  -----" << endl;
	cout << cEventDescrs::entries[i].GetDescription() << endl;
      }
      exit(0);
    }
    else if (cur_arg == "--help" || cur_arg == "-help" ||
	cur_arg == "-h"     || genesis.IsOpen() == false) {
      cout << "Options:"<<endl
	   << "  -g[enesis] <filename> Set genesis file to be <filename>"<<endl
	   << "  -h[elp]               Help on options (this listing)"<<endl
	   << "  -e[vents]             Print a list of all known events"<< endl
	   << "  -s[eed] <value>       Set random seed to <value>"<<endl
	   << "  -viewer <value>       Sets Viewer to <value>"<<endl
	   << "  -v[ersion]            Prints the version number"<<endl
	   << "  -set <name> <value>   Overide the genesis file"<<endl
	   << "  -l[oad] <filename>    Load a clone file"<<endl
	   << "  -loadpop <filename>   Load a saved population file (precedence over load)"<<endl
	   << "  -a[nalyze]            Process analyze.cfg instead of normal run."<<endl
	// <<"  -p[rimitive]          Overide viewer to be primitive."<<endl
	   << endl;
	
      exit(0);
    }
    else if (cur_arg == "-seed" || cur_arg == "-s") {
      int in_seed = 0;
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include a number as the random seed!"<<endl;
	exit(0);
      } else {
	arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
	in_seed = cur_arg.AsInt();
      }
      genesis.AddInput("RANDOM_SEED", in_seed);
    } else if (cur_arg == "-analyze" || cur_arg == "-a") {
      analyze_mode = true;
    } else if (cur_arg == "-primitive" || cur_arg == "-p") {
      primitive_mode = true;
    } else if (cur_arg == "-load" || cur_arg == "-l") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include a filename to load from"<<endl;
	exit(0);
      } else {
	arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
	clone_filename = cur_arg;
      }
    } else if (cur_arg == "-loadpop" || cur_arg == "-lp") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include a filename to load from"<<endl;
	exit(0);
      } else {
	arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
	load_pop_filename = cur_arg;
      }
    } else if (cur_arg == "-version" || cur_arg == "-v") {
      cout << " by Charles Ofria" << endl;
      cout << " designed by Charles Ofria, Chris Adami, Travis Collier, C. Titus Brown, and Claus Wilke" << endl;
      cout << " For more information, see: http://dllab.caltech.edu/avida/" << endl;
      exit(0);
    } else if (cur_arg == "-set") {
      if (arg_num + 1 == argc || arg_num + 2 == argc) {
	cerr << "'-set' option must be followed by name and value" << endl;
	exit(0);
      }
      arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
      cString name(cur_arg);
      arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
      cString value(cur_arg);
      cout << "SET " << name() << " = " << value() << endl;
      genesis.AddInput(name(), value());
    } else if (cur_arg == "-viewer") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include viewer type"<<endl;
	exit(0);
      } else {
	arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
	viewer_type = cur_arg.AsInt();
      }
      genesis.AddInput("VIEWER_TYPE", viewer_type);


    } else if (cur_arg == "-g" || cur_arg == "-genesis") {
      cerr << "Error: -g[enesis] option must be listed first." << endl;
      exit(0);
    } else {
      cerr << "Unknown Option: " << argv[arg_num] << endl
	   << "Type: \"" << argv[0] << " -h\" for a full option list." << endl;
      exit(0);
    }

    arg_num++;  if (arg_num < argc) cur_arg = args[arg_num];
  }

  rand_seed = genesis.ReadInt("RANDOM_SEED");
  cout << "Random Seed: " << rand_seed;
  g_random.ResetSeed(rand_seed);
  if( rand_seed != g_random.GetSeed() ) cout << " -> " << g_random.GetSeed();
  cout << endl;

  delete [] args;
}
