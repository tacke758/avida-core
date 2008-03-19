//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "stats.hh"
#include "genotype.hh"
#include "inst_lib.hh"
#include "../cpu/environment.hh"

void cConfig::Setup(int argc, char * argv[])
{
  cAssert::Init(argv[0]);
  default_dir = DEFAULT_DIR;

  cGenesis genesis;
  ProcessConfiguration(argc, argv, genesis);

  // Load the default directory...
  default_dir = genesis.ReadString("DEFAULT_DIR", DEFAULT_DIR);
  char dir_tail = default_dir[default_dir.GetSize() - 1];
  if (dir_tail != '\\' && dir_tail != '/') default_dir += "/";

  // Input files...
  inst_filename  = genesis.ReadString("INST_SET", "inst_set");
  task_filename  = genesis.ReadString("TASK_SET", "task_set");
  event_filename = genesis.ReadString("EVENT_FILE", "event_list");
  start_creature = genesis.ReadString("START_CREATURE");


  // Load Archetecture...
  max_updates     = genesis.ReadInt("MAX_UPDATES", -1);
  max_generations = genesis.ReadInt("MAX_GENERATIONS", -1);
  world_x         = genesis.ReadInt("WORLD-X");
  world_y         = genesis.ReadInt("WORLD-Y");

  birth_method   = genesis.ReadInt("BIRTH_METHOD", POSITION_CHILD_AGE);
  death_method   = genesis.ReadInt("DEATH_METHOD", DEATH_METHOD_OFF);
  alloc_method   = genesis.ReadInt("ALLOC_METHOD", ALLOC_METHOD_DEFAULT);
  age_limit      = genesis.ReadInt("AGE_LIMIT", -1);
  child_size_range = genesis.ReadFloat("CHILD_SIZE_RANGE", 2.0);
  ave_time_slice = genesis.ReadInt("AVE_TIME_SLICE", 30);

  // Geneology
  species_threshold  = genesis.ReadInt("SPECIES_THRESHOLD");
  threshold          = genesis.ReadInt("THRESHOLD");
  genotype_print     = genesis.ReadInt("GENOTYPE_PRINT");
  species_print      = genesis.ReadInt("SPECIES_PRINT");
  species_recording  = genesis.ReadInt("SPECIES_RECORDING");
  genotype_print_dom = genesis.ReadInt("GENOTYPE_PRINT_DOM");

  // CPU Info
  max_cpu_threads = genesis.ReadInt("MAX_CPU_THREADS", 1);

  // Time Slicing Info
  slicing_method = genesis.ReadInt("SLICING_METHOD", SLICE_CONSTANT);
  size_merit_method = genesis.ReadInt("SIZE_MERIT_METHOD", 0);
  base_size_merit   = genesis.ReadInt("BASE_SIZE_MERIT", 0);

  // Task Merit Method
  task_merit_method = genesis.ReadInt("TASK_MERIT_METHOD", TASK_MERIT_NORMAL);
  max_num_tasks_rewarded = genesis.ReadInt("MAX_NUM_TASKS_REWARDED", -1);

  // Load Mutation Info
  point_mut_prob  = genesis.ReadFloat("POINT_MUT_PROB");
  copy_mut_prob   = genesis.ReadFloat("COPY_MUT_PROB");
  ins_mut_prob    = genesis.ReadFloat("INS_MUT_PROB");
  del_mut_prob    = genesis.ReadFloat("DEL_MUT_PROB");
  divide_mut_prob = genesis.ReadFloat("DIVIDE_MUT_PROB");
  divide_ins_prob = genesis.ReadFloat("DIVIDE_INS_PROB");
  divide_del_prob = genesis.ReadFloat("DIVIDE_DEL_PROB");


  // Load Viewer Info...
  view_mode = genesis.ReadInt("VIEW_MODE");


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

  // *** Other Defaults ***
  // Setup Default Task Lib -- this would be better moved  @TCC
  // Must be done after max_num_tasks_rewarded is set!
  default_task_lib.Load(cConfig::GetTaskFilename());
  num_tasks = default_task_lib.GetNumTasks();

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
    fprintf(stderr, "Error: Using incorrect genesis file.\n");
    fprintf(stderr, "       Version needed = \"%s\".  Version used = \"%s\"\n",
	    AVIDA_VERSION, version_id());
    exit(0);
  } else {
    printf("Avida Version %s\n", AVIDA_VERSION);
  }


  // Then scan through and process the rest of the args.

  while (arg_num < argc) {
    // Test against the possible inputs.
    if (args[arg_num] == "-help" || args[arg_num] == "-h") {
      printf("Options:\n");
      printf("  -g[enesis] <filename>    Set genesis file to be <filename>\n");
      printf("  -h[elp]                  Help on options (this listing)\n");
      printf("  -s[eed] <value>          Set random seed to <value>\n");
      printf("  -v[ersion]               Prints the version number\n");
      printf("  -set <name> <value>      Overide the genesis file\n");
      printf("  -l[oad] <filename>       Load a clone file\n");
      exit(0);
    } else if (args[arg_num] == "-seed" || args[arg_num] == "-s") {
      int in_seed = 0;
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	fprintf(stderr, "Must include a number as the random seed!\n");
	exit(0);
      } else {
	arg_num++;
	in_seed = args[arg_num].AsInt();
      }
      genesis.AddInput("RANDOM_SEED", in_seed);
    } else if (args[arg_num] == "-load" || args[arg_num] == "-l") {
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	fprintf(stderr, "Must include a filename to load from\n");
	exit(0);
      } else {
	arg_num++;
	clone_filename = args[arg_num];
      }
    } else if (args[arg_num] == "-version" || args[arg_num] == "-v") {
      printf(" by Charles Ofria\n");
      printf(" designed by Charles Ofria, Chris Adami, C. Titus Brown, and Travis Collier\n");
      printf(" portions of avida were coded by Travis Collier and Dennis Adler\n");
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
      genesis.AddInput(name(), value());
    } else {
      fprintf(stderr, "Unknown Option: %s\n", argv[arg_num]);
      fprintf(stderr, "Type: \"%s -h\" for a full option list.\n", argv[0]);
      exit(0);
    }

    arg_num++;
  }

  int rand_seed = genesis.ReadInt("RANDOM_SEED");
  printf("Random Seed: %d", rand_seed);
  g_random.ResetSeed(rand_seed);
  if( rand_seed != g_random.GetSeed() )
    printf(" -> %d", g_random.GetSeed());
  printf("\n");

  delete [] args;
}


////////////
//  cStats
////////////

void cStats::Setup()
{
  int i;

  g_memory.Add(C_STATISTICS);

  current_update = -1;
  ave_generation = 0;
  avida_time = 0;

  // Open Status files.

  if (cConfig::GetGenotypeStatusInterval())
    fp_genotype_status.open("genotype.status");
  else fp_genotype_status.close();

  if (cConfig::GetDiversityStatusInterval())
    fp_diversity_status.open("diversity.status");
  else fp_diversity_status.close();


  // Open log files.

  if (cConfig::GetLogBreedCount())
    fp_breed_log.open("breed_count.log");
  else fp_breed_log.close();

  if (cConfig::GetLogCreatures())
    fp_creature_log.open("creature.log");
  else fp_creature_log.close();

  if (cConfig::GetLogPhylogeny())
    fp_phylogeny_log.open("phylogeny.log");
  else fp_phylogeny_log.close();

#ifdef GENEOLOGY
  if (cConfig::GetLogGeneology())
    fp_geneology_log.open("geneology.log");
  else fp_geneology_log.close();
#endif

  if (cConfig::GetLogGenotypes())
    fp_genotype_log.open("genotype.log");
  else fp_genotype_log.close();

  if (cConfig::GetLogThreshold())
    fp_threshold_log.open("threshold.log");
  else fp_threshold_log.close();

  if (cConfig::GetLogSpecies())
    fp_species_log.open("species.log");
  else fp_species_log.close();

  if (cConfig::GetLogLandscape())
    fp_landscape_log.open("landscape.log");
  else fp_landscape_log.close();

  // Update information

  best_merit     = 0;
  best_fitness   = 0;
  best_gestation = 0.0;
  max_fitness    = 0.0;

  entropy        = 0.0;
  energy         = 0.0;

  num_deaths     = 0;
  num_births     = 0;
  num_executed   = 0;
  num_creatures  = 0;
  num_genotypes  = 0;
  num_threshold  = 0;
  num_species    = 0;
  num_thresh_species = 0;
  num_breed_true = 0;
  num_breed_true_creatures = 0;

  num_genotypes_last = 1;

  tot_creatures = 0;
  tot_genotypes = 0;
  tot_threshold = 0;
  tot_species   = 0;
  tot_thresh_species = 0;
  tot_executed  = 0;

  task_count = g_memory.GetInts(cConfig::GetNumTasks());
  for (i = 0; i < cConfig::GetNumTasks(); i++) {
    task_count[i] = 0;
  }
}


void cStats::ZeroTasks(){
  for( int i=0; i<cConfig::GetNumTasks(); ++i )
    task_count[i] = 0;
}


void cStats::Shutdown()
{
  g_memory.Remove(C_STATISTICS);
  g_memory.Free(task_count, cConfig::GetNumTasks());

  cout<<"Closing stats object..."<<endl;

  // Close all the statistics files.

  data_file_manager.FlushAll();

  fp_genotype_status.close();
  fp_diversity_status.close();

  fp_breed_log.close();
  fp_creature_log.close();
  fp_genotype_log.close();
  fp_threshold_log.close();
  fp_species_log.close();
  fp_landscape_log.close();
}

void cStats::SetBestGenotype(cGenotype * best_genotype)
{
  dom_genotype = best_genotype;
  best_merit = best_genotype->GetApproxMerit();
  best_gestation = best_genotype->GetApproxGestationTime();
  best_repro_rate = best_genotype->GetReproRate();
  best_fitness = best_genotype->GetApproxFitness();
  best_copied_size = best_genotype->GetApproxCopiedSize();
  best_exe_size = best_genotype->GetApproxExecutedSize();

  best_size = best_genotype->GetLength();
  best_genotype_id = best_genotype->GetID();
  best_name = best_genotype->GetName();
  best_births = best_genotype->GetThisBirths();
  best_breed_true = best_genotype->GetThisBreedTrue();
  best_breed_in = best_genotype->GetThisBreedIn();
  best_abundance = best_genotype->GetNumCPUs();
  best_gene_depth = best_genotype->GetDepth();
}

void cStats::ProcessUpdate(int new_update)
{
  static int last_update = 0;
  (void)new_update;

  // Increment the "avida_time"
  if( sum_merit.Count() > 0  &&  sum_merit.Average() > 0 ){
    avida_time += ((double)(current_update-last_update)) / sum_merit.Average();
  }
  last_update = current_update;

  // Average generation
  ave_generation = sum_generation.Average();

  int i;

  // Print average.dat
  if( cConfig::GetAverageDataInterval() > 0
      && (current_update % cConfig::GetAverageDataInterval() == 0) ){
    PrintAverageData("average.dat");
    // StdError of Average
    data_file_manager.Get("error.dat",&(cStats::OutputErrorData)).Output();
    // Variance of Average
    data_file_manager.Get("variance.dat",&(cStats::OutputVarianceData)).Output();
  }


  // Print dominant.dat
  if( cConfig::GetDominantDataInterval() > 0
      && (current_update % cConfig::GetDominantDataInterval() == 0) ){
    data_file_manager.Get("dominant.dat",&(cStats::OutputDominantData)).Output();
  }

  // Print stats.dat
  if( cConfig::GetStatsDataInterval() > 0
      && (current_update % cConfig::GetStatsDataInterval() == 0)) {
    data_file_manager.Get("stats.dat",&(cStats::OutputStatsData)).Output();
  }

  // Print count.dat
  if( cConfig::GetCountDataInterval() > 0
      && (current_update % cConfig::GetCountDataInterval() == 0)) {
    data_file_manager.Get("count.dat",&(cStats::OutputCountData)).Output();
  }

  // Print totals.dat
  tot_executed += num_executed;
  if( cConfig::GetTotalsDataInterval() > 0
      && (current_update % cConfig::GetTotalsDataInterval() == 0)) {
    data_file_manager.Get("totals.dat",&(cStats::OutputTotalsData)).Output();
  }

  // Print tasks.dat
  if( cConfig::GetTasksDataInterval() > 0
      && (current_update % cConfig::GetTasksDataInterval() == 0)) {
    data_file_manager.Get("tasks.dat",&(cStats::OutputTasksData)).Output();
  }

  // Time.dat
  if( cConfig::GetTimeDataInterval() > 0
      && (current_update % cConfig::GetTimeDataInterval() == 0)) {
    data_file_manager.Get("time.dat",&(cStats::OutputTimeData)).Output();
  }


  // Zero-out any variables which need to be cleared at end of update.

  num_births = 0;
  num_deaths = 0;
  num_breed_true = 0;
  num_executed = 0;
  for (i = 0; i < cConfig::GetNumTasks(); i++) {
    task_count[i] = 0;
  }

  best_merit = 0;
  best_gestation = 0.0;
  best_fitness = 0.0;
  max_fitness = 0.0;

  // Clear Sums that are collected durring the update
  sum_repro_rate.Clear();
}


void cStats::TestThreads(const cCodeArray & code)
{
  static ofstream fp("threads.dat");

  cCPUTestInfo test_info(TEST_CPU_DIVIDES, TEST_CPU_GENERATIONS,
			 TEST_ALLOC_MOD, TRUE);
  cTestCPU::TestCode(test_info, code);

  cDivideRecord * divide1 = test_info.GetDivideRecord(1);
  cDivideRecord * divide2 = test_info.GetDivideRecord(2);

  fp << cStats::GetUpdate()            << " "   // 1
     << code.GetSize()                 << " "   // 2
     << sum_num_threads.Average()      << " "   // 3
     << sum_thread_dist.Average()      << " ";  // 4

  if (divide1) {
    fp << divide1->GetMerit()          << " "   // 5
       << divide1->GetGestationTime()  << " "   // 6
       << divide1->GetFitness()        << " "   // 7
       << divide1->GetThreadFrac()     << " "   // 8
       << divide1->GetThreadTimeDiff() << " "   // 9
       << divide1->GetThreadCodeDiff() << " ";  // 10
  } else {
    fp << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0;
  }

  if (divide2) {
    fp << divide2->GetMerit()          << " "   // 11
       << divide2->GetGestationTime()  << " "   // 12
       << divide2->GetFitness()        << " "   // 13
       << divide2->GetThreadFrac()     << " "   // 14
       << divide2->GetThreadTimeDiff() << " "   // 15
       << divide2->GetThreadCodeDiff() << " ";  // 16
  } else {
    fp << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0;
  }

  fp << endl;
}


void cStats::FlushFP(){
  data_file_manager.FlushAll();

  fp_genotype_status.flush();
  fp_diversity_status.flush();

  fp_creature_log.flush();
  fp_breed_log.flush();
  fp_genotype_log.flush();
  fp_threshold_log.flush();
  fp_species_log.flush();
  fp_landscape_log.flush();
}

////////////////////////////////////////////////
//  And now, a whole mess of static variables!
////////////////////////////////////////////////

// -- cConfig --

cString cConfig::inst_filename;
cString cConfig::task_filename;
cString cConfig::event_filename;
cString cConfig::start_creature;
cString cConfig::clone_filename;
int cConfig::max_updates;
int cConfig::max_generations;
int cConfig::world_x;
int cConfig::world_y;
double cConfig::point_mut_prob;
double cConfig::copy_mut_prob;
double cConfig::ins_mut_prob;
double cConfig::del_mut_prob;
double cConfig::divide_mut_prob;
double cConfig::divide_ins_prob;
double cConfig::divide_del_prob;
int cConfig::num_instructions;
int cConfig::max_cpu_threads;
int cConfig::size_merit_method;
int cConfig::base_size_merit;
int cConfig::task_merit_method;
int cConfig::max_num_tasks_rewarded;
cTaskLib cConfig::default_task_lib;
int cConfig::num_tasks;
int cConfig::slicing_method;
int cConfig::birth_method;
int cConfig::death_method;
int cConfig::alloc_method;
int cConfig::age_limit;
double cConfig::child_size_range; 
int cConfig::ave_time_slice;
int cConfig::species_threshold;
int cConfig::threshold;
int cConfig::genotype_print;
int cConfig::species_print;
int cConfig::species_recording;
int cConfig::genotype_print_dom;
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
int cConfig::debug_level;
int cConfig::view_mode;

// -- cStats --
int cStats::current_update;
double cStats::ave_generation;
int cStats::sub_update;
double cStats::avida_time;

cDataFileManager cStats::data_file_manager;

ofstream cStats::fp_genotype_status;
ofstream cStats::fp_diversity_status;
ofstream cStats::fp_creature_log;
ofstream cStats::fp_phylogeny_log;
ofstream cStats::fp_geneology_log;
ofstream cStats::fp_breed_log;
ofstream cStats::fp_genotype_log;
ofstream cStats::fp_threshold_log;
ofstream cStats::fp_species_log;
ofstream cStats::fp_landscape_log;
cRunningSum cStats::sum_repro_rate;
cRunningSum cStats::sum_merit;
cRunningSum cStats::sum_mem_size;
cRunningSum cStats::sum_creature_age;
cRunningSum cStats::sum_generation;
cRunningSum cStats::sum_neutral_metric;
cRunningSum cStats::sum_lineage_label;
cRunningSum cStats::sum_num_threads;
cRunningSum cStats::sum_thread_dist;
cRunningSum cStats::sum_gestation;
cRunningSum cStats::sum_fitness;
cRunningSum cStats::sum_size;
cRunningSum cStats::sum_copy_size;
cRunningSum cStats::sum_exe_size;
cRunningSum cStats::sum_genotype_age;
cRunningSum cStats::sum_abundance;
cRunningSum cStats::sum_genotype_depth;
cRunningSum cStats::sum_threshold_age;
cRunningSum cStats::sum_species_age;
double cStats::entropy;
double cStats::species_entropy;
double cStats::energy;
double cStats::dom_fidelity;
double cStats::ave_fidelity;
cGenotype * cStats::dom_genotype;
double cStats::best_merit;
double cStats::best_gestation;
double cStats::best_repro_rate;
double cStats::best_fitness;
int cStats::best_size;
double cStats::best_copied_size;
double cStats::best_exe_size;
double cStats::max_fitness;
double cStats::max_merit;
int cStats::best_genotype_id;
cString cStats::best_name;
int cStats::best_births;
int cStats::best_breed_true;
int cStats::best_breed_in;
int cStats::best_abundance;
int cStats::best_gene_depth;
int cStats::num_births;
int cStats::num_deaths;
int cStats::num_breed_in;
int cStats::num_breed_true;
int cStats::num_breed_true_creatures;
int cStats::num_creatures;
int cStats::num_genotypes;
int cStats::num_threshold;
int cStats::num_species;
int cStats::num_thresh_species;
int cStats::num_executed;
int cStats::num_parasites;
int cStats::num_genotypes_last;
int cStats::tot_creatures;
int cStats::tot_genotypes;
int cStats::tot_threshold;
int cStats::tot_species;
int cStats::tot_thresh_species;
int cStats::tot_executed;
int * cStats::task_count;




void cStats::OutputAverageData(ofstream & fp){
  assert(fp.good());
  fp<< GetUpdate()                          << " "  // 1
    << sum_merit.Average()                  << " "  // 2
    << sum_gestation.Average()              << " "  // 3
    << sum_fitness.Average()                << " "  // 4
    << sum_repro_rate.Average()             << " "  // 5
    << sum_size.Average()                   << " "  // 6
    << sum_copy_size.Average()              << " "  // 7
    << sum_exe_size.Average()               << " "  // 8
    << sum_abundance.Average()              << " "  // 9
    << (double)num_births/num_creatures     << " "  // 10
    << (double)num_breed_true/num_creatures << " "  // 11
    << sum_genotype_depth.Average()         << " "  // 12
    << sum_num_threads.Average()            << " "  // 13
    << sum_thread_dist.Average()            << " "  // 14
    << sum_generation.Average()             << " "  // 15
    << sum_neutral_metric.Average()         << " "  // 16
    << sum_lineage_label.Average()          <<endl; // 17
}


void cStats::OutputErrorData(ofstream & fp){
  assert(fp.good());
  fp<< GetUpdate()                          << " "  // 1
    << sum_merit.StdError()                 << " "  // 2
    << sum_gestation.StdError()             << " "  // 3
    << sum_fitness.StdError()               << " "  // 4
    << sum_repro_rate.StdError()            << " "  // 5
    << sum_size.StdError()                  << " "  // 6
    << sum_copy_size.StdError()             << " "  // 7
    << sum_exe_size.StdError()              << " "  // 8
    << sum_abundance.StdError()             << " "  // 9
    << -1                                   << " "  // 10
    << -1                                   << " "  // 11
    << sum_genotype_depth.StdError()        << " "  // 12
    << sum_num_threads.StdError()           << " "  // 13
    << sum_thread_dist.StdError()           << " "  // 14
    << sum_generation.StdError()            << " "  // 15
    << sum_neutral_metric.StdError()        << " "  // 16
    << sum_lineage_label.StdError()         << endl; // 17
}


void cStats::OutputVarianceData(ofstream & fp){
  assert(fp.good());
  fp<<GetUpdate()                           << " "  // 1
    << sum_merit.Variance()                 << " "  // 2
    << sum_gestation.Variance()             << " "  // 3
    << sum_fitness.Variance()               << " "  // 4
    << sum_repro_rate.Variance()            << " "  // 5
    << sum_size.Variance()                  << " "  // 6
    << sum_copy_size.Variance()             << " "  // 7
    << sum_exe_size.Variance()              << " "  // 8
    << sum_abundance.Variance()             << " "  // 9
    << -1                                   << " "  // 10
    << -1                                   << " "  // 11
    << sum_genotype_depth.Variance()        << " "  // 12
    << sum_num_threads.Variance()           << " "  // 13
    << sum_thread_dist.Variance()           << " "  // 14
    << sum_generation.Variance()            << " "  // 15
    << sum_neutral_metric.Variance()        << " "  // 16
    << sum_lineage_label.Variance()         << endl; // 17
}


void cStats::OutputDominantData(ofstream & fp){
  assert(fp.good());
  fp<<GetUpdate()            <<" "   // 1
    <<best_merit             <<" "   // 2
    <<best_gestation         <<" "   // 3
    <<best_fitness           <<" "   // 4
    <<best_repro_rate        <<" "   // 5
    <<best_size              <<" "   // 6
    <<best_copied_size       <<" "   // 7
    <<best_exe_size          <<" "   // 8
    <<best_abundance         <<" "   // 9
    <<best_births            <<" "   // 10
    <<best_breed_true        <<" "   // 11
    <<best_gene_depth        <<" "   // 12
    // These would be nice
    //<<best_num_threads       <<" "   // 13
    //<<best_thread_dist       <<" "   // 14
    //<<best_generation        <<" "   // 15
    //<<best_neutral_metric    <<" "   // 16
    //<<best_lineage_label     <<" "   // 17
    <<best_breed_in          <<" "   // 13
    <<max_fitness            <<" "   // 14
    <<best_genotype_id       <<" "   // 15
    <<best_name              <<endl; // 16
}


void cStats::OutputStatsData(ofstream & fp){
  int genotype_change = num_genotypes - num_genotypes_last;
  assert(fp.good());
  fp<<GetUpdate()<<" "            // 1
    <<energy<<" "                 // 2
    <<(1.0-ave_fidelity)<<" "     // 3
    <<1.0-dom_fidelity<<" ";      // 4
  if( ave_fidelity > 0 )
    fp<<-Log(ave_fidelity)<<" ";  // 5
  else
    fp<<0<<" ";
  if( ave_fidelity > 0 )
    fp<<-Log(dom_fidelity)<<" ";  // 6
  else
    fp<<0<<" ";
  fp<<genotype_change<<" "        // 7
    <<entropy<<" "                // 8
    <<species_entropy<<endl;      // 9
}


void cStats::OutputCountData(ofstream & fp){
  assert(fp.good());
  fp<< GetUpdate()              << " "  // 1
    << num_executed             << " "  // 2
    << num_creatures            << " "  // 3
    << num_genotypes            << " "  // 4
    << num_threshold            << " "  // 5
    << num_species              << " "  // 6
    << num_thresh_species       << " "  // 7
    << num_births               << " "  // 8
    << num_deaths               << " "  // 9
    << num_breed_true           << " "  // 10
    << num_breed_true_creatures << " "  // 11
    << num_parasites            << " "  // 12
    << endl;
}


void cStats::OutputTotalsData(ofstream & fp){
  assert(fp.good());
  tot_executed += num_executed;
  fp<<GetUpdate()<<" "    // 1
    <<tot_executed<<" "   // 2
    <<tot_creatures<<" "  // 3
    <<tot_genotypes<<" "  // 4
    <<tot_threshold<<" "  // 5
    <<tot_species<<endl;  // 6
}


void cStats::OutputTasksData(ofstream & fp){
  int i;
  assert(fp.good());
  fp<<GetUpdate();
  for( i=0; i<cConfig::GetNumTasks(); i++ ){
    fp<<" "<<task_count[i];
  }
  fp<<endl;
}


void cStats::OutputTimeData(ofstream & fp){
  assert(fp.good());
  fp<< GetUpdate()        <<" "   // 1
    << avida_time         <<" "   // 2
    << GetAveGeneration() <<endl; // 3
}

