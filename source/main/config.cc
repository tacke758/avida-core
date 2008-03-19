//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "config.hh"
#include "genotype.hh"
#include "inst_lib.hh"
#include "../cpu/environment.hh"

// -- cConfig --

bool cConfig::analyze_mode;
cString cConfig::inst_filename;
cString cConfig::task_filename;
cString cConfig::resource_filename;
cString cConfig::event_filename;
cString cConfig::analyze_filename;
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
double cConfig::divide_mut_prob;
double cConfig::divide_ins_prob;
double cConfig::divide_del_prob;
double cConfig::crossover_prob;
double cConfig::aligned_cross_prob;
double cConfig::exe_err_prob;
int cConfig::num_instructions;
int cConfig::max_cpu_threads;
int cConfig::size_merit_method;
int cConfig::base_size_merit;
int cConfig::task_merit_method;
int cConfig::max_label_exe_size;
int cConfig::max_num_tasks_rewarded;
int cConfig::resources;
int cConfig::merit_time;
cTaskLib cConfig::default_task_lib;
int cConfig::num_tasks;
int cConfig::slicing_method;
int cConfig::birth_method;
int cConfig::death_method;
int cConfig::alloc_method;
int cConfig::divide_method;
int cConfig::lineage_creation_method;
int cConfig::generation_count_method;
int cConfig::age_limit;
double cConfig::child_size_range;
double cConfig::min_copied_lines;
double cConfig::min_exe_lines;
int cConfig::require_allocate;
bool cConfig::test_on_divide;
double cConfig::revert_fatal;
double cConfig::revert_neg;
double cConfig::revert_neut;
double cConfig::revert_pos;
double cConfig::reset_fatal;
double cConfig::reset_neg;
double cConfig::reset_neut;
double cConfig::reset_pos;
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
int cConfig::average_data_interval;
int cConfig::dominant_data_interval;
int cConfig::count_data_interval;
int cConfig::totals_data_interval;
int cConfig::tasks_data_interval;
int cConfig::stats_data_interval;
int cConfig::time_data_interval;
int cConfig::genotype_status_interval;
int cConfig::diversity_status_interval;
int cConfig::log_threshold_only;
int cConfig::log_geneology_leaves;
int cConfig::log_breed_count;
int cConfig::log_creatures;
int cConfig::log_phylogeny;
int cConfig::log_geneology;
int cConfig::log_genotypes;
int cConfig::log_threshold;
int cConfig::log_species;
int cConfig::log_landscape;
int cConfig::log_mutations;
int cConfig::log_lineages;
int cConfig::debug_level;
int cConfig::view_mode;
int cConfig::viewer_type;



void cConfig::Setup(int argc, char * argv[])
{
  cAssert::Init(argv[0]);
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
  task_filename  = genesis.ReadString("TASK_SET", "task_set");
  resource_filename  = genesis.ReadString("RESOURCE_SET", "resource_set");
  event_filename = genesis.ReadString("EVENT_FILE", "event_list");
  analyze_filename = genesis.ReadString("ANALYZE_FILE", "analyze.cfg");
  start_creature = genesis.ReadString("START_CREATURE");


  // Load Archetecture...
  max_updates     = genesis.ReadInt("MAX_UPDATES", -1);
  max_generations = genesis.ReadInt("MAX_GENERATIONS", -1);
  end_condition_mode = genesis.ReadInt("END_CONDITION_MODE", 0);
  world_x         = genesis.ReadInt("WORLD-X");
  world_y         = genesis.ReadInt("WORLD-Y");

  birth_method   = genesis.ReadInt("BIRTH_METHOD", POSITION_CHILD_AGE);
  death_method   = genesis.ReadInt("DEATH_METHOD", DEATH_METHOD_OFF);
  alloc_method   = genesis.ReadInt("ALLOC_METHOD", ALLOC_METHOD_DEFAULT);
  divide_method  = genesis.ReadInt("DIVIDE_METHOD", DIVIDE_METHOD_OFFSPRING);
  lineage_creation_method =
    genesis.ReadInt("LINEAGE_CREATION_METHOD", 0);
  generation_count_method =
    genesis.ReadInt("GENERATION_COUNT_METHOD", GENERATION_COUNT_METHOD_OFFSPRING );
  age_limit      = genesis.ReadInt("AGE_LIMIT", -1);
  child_size_range = genesis.ReadFloat("CHILD_SIZE_RANGE", 2.0);
  min_copied_lines = genesis.ReadFloat("MIN_COPIED_LINES", 0.5);
  min_exe_lines    = genesis.ReadFloat("MIN_EXE_LINES", 0.5);
  require_allocate = genesis.ReadInt("REQUIRE_ALLOCATE", 1);

  revert_fatal = genesis.ReadFloat("REVERT_FATAL", 0.0);
  revert_neg   = genesis.ReadFloat("REVERT_DETRIMENTAL", 0.0);
  revert_neut  = genesis.ReadFloat("REVERT_NEUTRAL", 0.0);
  revert_pos   = genesis.ReadFloat("REVERT_BENEFICIAL", 0.0);
  reset_fatal = genesis.ReadFloat("RESET_FATAL", 0.0);
  reset_neg   = genesis.ReadFloat("RESET_DETRIMENTAL", 0.0);
  reset_neut  = genesis.ReadFloat("RESET_NEUTRAL", 0.0);
  reset_pos   = genesis.ReadFloat("RESET_BENEFICIAL", 0.0);
  test_on_divide = (revert_fatal + revert_neg + revert_neut + revert_pos +
		    reset_fatal + reset_neg + reset_neut + reset_pos) != 0.0;
  fail_implicit = genesis.ReadInt("FAIL_IMPLICIT", 0);

  // Geneology
  species_threshold  = genesis.ReadInt("SPECIES_THRESHOLD");
  threshold          = genesis.ReadInt("THRESHOLD");
  genotype_print     = genesis.ReadInt("GENOTYPE_PRINT");
  species_print      = genesis.ReadInt("SPECIES_PRINT");
  species_recording  = genesis.ReadInt("SPECIES_RECORDING");
  genotype_print_dom = genesis.ReadInt("GENOTYPE_PRINT_DOM");
  test_cpu_time_mod  = genesis.ReadInt("TEST_CPU_TIME_MOD", 20);
  track_main_lineage = genesis.ReadInt("TRACK_MAIN_LINEAGE", 0);

  // CPU Info
  max_cpu_threads = genesis.ReadInt("MAX_CPU_THREADS", 1);

  // Time Slicing Info
  slicing_method = genesis.ReadInt("SLICING_METHOD", SLICE_CONSTANT);
  size_merit_method = genesis.ReadInt("SIZE_MERIT_METHOD", 0);
  base_size_merit   = genesis.ReadInt("BASE_SIZE_MERIT", 0);
  ave_time_slice = genesis.ReadInt("AVE_TIME_SLICE", 30);

  // Task Merit Method
  task_merit_method = genesis.ReadInt("TASK_MERIT_METHOD", TASK_MERIT_NORMAL);
  max_num_tasks_rewarded = genesis.ReadInt("MAX_NUM_TASKS_REWARDED", -1);
  max_label_exe_size = genesis.ReadInt("MAX_LABEL_EXE_SIZE", 1);

  // Resources
  resources = genesis.ReadInt("RESOURCES", 0);
  merit_time = genesis.ReadInt("MERIT_TIME", 0);

  // Load Mutation Info
  point_mut_prob  = genesis.ReadFloat("POINT_MUT_PROB");
  copy_mut_prob   = genesis.ReadFloat("COPY_MUT_PROB");
  ins_mut_prob    = genesis.ReadFloat("INS_MUT_PROB");
  del_mut_prob    = genesis.ReadFloat("DEL_MUT_PROB");
  divide_mut_prob = genesis.ReadFloat("DIVIDE_MUT_PROB");
  divide_ins_prob = genesis.ReadFloat("DIVIDE_INS_PROB");
  divide_del_prob = genesis.ReadFloat("DIVIDE_DEL_PROB");
  crossover_prob  = genesis.ReadFloat("CROSSOVER_PROB");
  aligned_cross_prob = genesis.ReadFloat("ALIGNED_CROSS_PROB");
  exe_err_prob    = genesis.ReadFloat("EXE_ERROR_PROB");


  // Load Viewer Info...
  view_mode = genesis.ReadInt("VIEW_MODE");
  viewer_type = genesis.ReadInt("VIEWER_TYPE",0);


  // Load Output File Info...

  average_data_interval = genesis.ReadInt("SAVE_AVERAGE_DATA");
  dominant_data_interval = genesis.ReadInt("SAVE_DOMINANT_DATA");
  count_data_interval = genesis.ReadInt("SAVE_COUNT_DATA");
  totals_data_interval = genesis.ReadInt("SAVE_TOTALS_DATA");
  tasks_data_interval = genesis.ReadInt("SAVE_TASKS_DATA");
  stats_data_interval = genesis.ReadInt("SAVE_STATS_DATA");
  time_data_interval = genesis.ReadInt("SAVE_TIME_DATA");

  genotype_status_interval = genesis.ReadInt("SAVE_GENOTYPE_STATUS");
  diversity_status_interval = genesis.ReadInt("SAVE_DIVERSITY_STATUS");

  log_breed_count = genesis.ReadInt("LOG_BREED_COUNT");
  log_creatures   = genesis.ReadInt("LOG_CREATURES");
  log_phylogeny   = genesis.ReadInt("LOG_PHYLOGENY");

#ifdef GENEOLOGY
  log_geneology   = genesis.ReadInt("LOG_GENEOLOGY");
  log_geneology_leaves = TRUE;
  if (log_geneology == 2) log_geneology_leaves = FALSE;
#endif

  log_genotypes = genesis.ReadInt("LOG_GENOTYPES");
  log_threshold_only = FALSE;
  if (log_genotypes > 1) log_threshold_only = TRUE;

  log_threshold = genesis.ReadInt("LOG_THRESHOLD");
  log_species   = genesis.ReadInt("LOG_SPECIES");
  log_landscape = genesis.ReadInt("LOG_LANDSCAPE");
  log_mutations = genesis.ReadInt("LOG_MUTATIONS");
  log_lineages = genesis.ReadInt("LOG_LINEAGES");


  // *** Other Defaults ***
  // Setup Default Task Lib -- this would be better moved  @TCC
  // Must be done after max_num_tasks_rewarded is set!
  LoadTaskSet();

  // Setup Debugging
  debug_level = genesis.ReadInt("DEBUG_LEVEL");
  g_debug.SetLevel(debug_level);
  //g_debug.Error("Testing...");
  g_debug.Warning("Testing...");
  g_debug.Comment("Testing...");

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
      printf ("WARNING: Unable to set input character set, rc= %i\n",
	      GetLastError());
    if (!SetConsoleOutputCP (GetOEMCP()))  // and for Console output
      printf ("WARNING: Unable to set output character set, rc= %i\n",
	      GetLastError());
  }
#endif
}


// This function takes in the genesis file and the input arguments, and puts
// out a pointer to a built-up cGenesis object.

void cConfig::ProcessConfiguration(int argc, char * argv[], cGenesis & genesis)
{
  cString filename("genesis"); // Name of genesis file.
  int arg_num = 1;             // Argument number being looked at.
  analyze_mode = false;        // Initialize analyze_mode tp be off.

  // Load all of the args into string objects for ease of access.
  cString * args = new cString[argc];
  for (int i = 0; i < argc; i++) args[i] = argv[i];


  // -genesis option

  if (argc > 1 && (args[1] == "-g" || args[1] == "-genesis")) {
    if (argc < 3) {
      fprintf(stderr, "Need filename for genesis file used.\n");
      exit(0);
    }
    filename = args[2];
    arg_num += 2;
  }


  // Open and verify the genesis file.

  genesis.Open(filename);
  cString version_id = genesis.ReadString("VERSION_ID", "Unknown");
  if (version_id != AVIDA_VERSION) {
    fprintf(stderr, "/  WARNING   WARNING   WARNING   WARNING   WARNING  \\\n");
    fprintf(stderr, "|   Using incorrect genesis file.                   |\n");
    fprintf(stderr, "|   Version needed = \"%s\".  Version used = \"%s\"   |\n",
	    AVIDA_VERSION, version_id());
    fprintf(stderr, "\\  WARNING   WARNING   WARNING   WARNING   WARNING  /\n\n");
  }

  // Then scan through and process the rest of the args.

  while (arg_num < argc) {
    // Test against the possible inputs.
    if (args[arg_num] == "-help" || args[arg_num] == "-h") {
      cout<<"Options:"<<endl
	  <<"  -g[enesis] <filename> Set genesis file to be <filename>"<<endl
	  <<"  -h[elp]               Help on options (this listing)"<<endl
	  <<"  -s[eed] <value>       Set random seed to <value>"<<endl
	  <<"  -viewer <value>       Sets Viewer to <value>"<<endl
	  <<"  -v[ersion]            Prints the version number"<<endl
	  <<"  -set <name> <value>   Overide the genesis file"<<endl
	  <<"  -l[oad] <filename>    Load a clone file"<<endl
	  <<"  -loadpop <filename>   Load a saved population file (precedence over load)"<<endl
	  <<"  -a[nalyze]            Process analyze.cfg instead of normal run."<<endl
	  << endl;
	
      exit(0);
    } else if (args[arg_num] == "-seed" || args[arg_num] == "-s") {
      int in_seed = 0;
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include a number as the random seed!"<<endl;
	exit(0);
      } else {
	arg_num++;
	in_seed = args[arg_num].AsInt();
      }
      genesis.AddInput("RANDOM_SEED", in_seed);
    } else if (args[arg_num] == "-analyze" || args[arg_num] == "-a") {
      analyze_mode = true;
    } else if (args[arg_num] == "-load" || args[arg_num] == "-l") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include a filename to load from"<<endl;
	exit(0);
      } else {
	arg_num++;
	clone_filename = args[arg_num];
      }
    } else if (args[arg_num] == "-loadpop" || args[arg_num] == "-lp") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include a filename to load from"<<endl;
	exit(0);
      } else {
	arg_num++;
	load_pop_filename = args[arg_num];
      }
    } else if (args[arg_num] == "-version" || args[arg_num] == "-v") {
      printf(" by Charles Ofria\n");
      printf(" designed by Charles Ofria, Chris Adami, Travis Collier, C. Titus Brown, and Claus Wilke\n");
      // printf(" portions of avida also coded by Travis Collier, Dennis Adler and Grace Hsu\n");
      printf(" For more information, see: http://dllab.caltech.edu/avida/\n");
      exit(0);
    } else if (args[arg_num] == "-set") {
      if (arg_num + 1 == argc || arg_num + 2 == argc) {
	fprintf(stderr, "'-set' option must be followed by name and value\n");
	exit(0);
      }
      arg_num++;
      cString name(args[arg_num]);
      arg_num++;
      cString value(args[arg_num]);
      printf("SET %s = %s\n",name(),value());
      genesis.AddInput(name(), value());
    } else if (args[arg_num] == "-viewer") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	cerr<<"Must include viewer type"<<endl;
	exit(0);
      } else {
	arg_num++;
	viewer_type = args[arg_num].AsInt();
      }
      genesis.AddInput("VIEWER_TYPE", viewer_type);


    } else if (args[arg_num] == "-g" || args[arg_num] == "-genesis") {
      fprintf(stderr, "Error: -g[enesis] option must be listed first.\n");
      exit(0);
    } else {
      fprintf(stderr, "Unknown Option: %s\n", argv[arg_num]);
      fprintf(stderr, "Type: \"%s -h\" for a full option list.\n", argv[0]);
      exit(0);
    }

    arg_num++;
  }

  rand_seed = genesis.ReadInt("RANDOM_SEED");
  printf("Random Seed: %d", rand_seed);
  g_random.ResetSeed(rand_seed);
  if( rand_seed != g_random.GetSeed() )
    printf(" -> %d", g_random.GetSeed());
  printf("\n");

  delete [] args;
}
