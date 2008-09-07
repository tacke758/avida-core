//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "avida.hh"
#include "playback.hh"
#include "genotype.hh"
#include "genebank.hh"
#include "species.hh"
#include "../viewers/view.hh"
#include <stdlib.h>
 
int main(int argc, char * argv[])
{
  default_dir = DEFAULT_DIR;
  cGenesis * genesis = ProcessConfiguration(argc, argv);
  
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
  
  SetupDebug(genesis->ReadInt("DEBUG_LEVEL"));
  
  // Run the proper populations...
  
  //  int num_populations = genesis->ReadInt("NUM_POPULATIONS");
  //  if (num_populations > 1) RunMultiPopulations(*genesis);
  //  else

  RunSinglePopulation(*genesis);

  return 0;
}

void RunMultiPopulations(cGenesis & /* genesis */)
{
  // Fill this in later...
}

void RunSinglePopulation(cGenesis & genesis)
{
  // Load the default directory...
  default_dir = genesis.ReadString("DEFAULT_DIR", DEFAULT_DIR);
  char dir_tail = default_dir[default_dir.GetSize() - 1];
  if (dir_tail != '\\' && dir_tail != '/') default_dir += "/";

  // Setup the population.

  cPopulation * population = BuildPopulation(genesis);

  // Setup the viewer.

  cView * viewer = BuildViewer(population);
  population->SetViewer(viewer);

  // Initialize the population.
  population->InitSoup();
  
  // Get remaining variables from genesis file...
  
  int max_updates = genesis.ReadInt("MAX_UPDATES");
  stats.SetMaxUpdates(max_updates);
  viewer->SetViewMode(genesis.ReadInt("VIEW_MODE"));

  // And finally, process the population...
 
  for (int update = 0;
       (update <= max_updates || !max_updates) &&
	 population->GetNumCreatures();
       update++) {
    stats.SetCurrentUpdate(update);

    // Process the update.
    population->DoUpdate();
    if (population->GetNumCreatures() == 0) break;
    
    // Setup the viewer for the new update.
    viewer->NewUpdate();

    // Handle all data collection for this update.
    CollectData(population->GetLocalStats(), population->GetGenebank());
  }

  population->Clear();
  delete population;
  delete viewer;
  EndProg(1);
}

cPopulation * BuildPopulation(cGenesis & genesis)
{
  int mode = genesis.ReadInt("MODE");
  switch(mode) {
  case MODE_GA:
    printf("Initializing GA population...\n");
    return new cGAPopulation(genesis);
  case MODE_TIERRA:
    printf("Initializing Tierra population...\n");
    return new cTierraPopulation(genesis);
  case MODE_AVIDA:
    printf("Initializing Avida population...\n");
    return new cAvidaPopulation(genesis);
  case MODE_PLAYBACK:
    // printf("Initializing Playback population...\n");
    // return new cPlaybackPopulation(genesis);
    printf("Playback mode not implemented.");
    exit(0);
  default:
    printf("Type %d population invalid!\n", mode);
    exit(0);
  }

  return NULL; // Should not be able to get here!
}

cView * BuildViewer(cPopulation * population)
{
#ifdef VIEW_PRIMITIVE
  return new cView;
#else
  int mode = population->Genesis().ReadInt("MODE");

  switch(mode) {
  case MODE_GA:
    printf("Setting up GA viewer...\n");
    return new cGAView((cGAPopulation *) population);
  case MODE_TIERRA:
    printf("Setting up Tierra viewer...\n");
    return new cTierraView((cTierraPopulation *) population);
  case MODE_AVIDA:
    printf("Setting up Avida viewer...\n");
    return new cAvidaView((cAvidaPopulation *) population);
    // case MODE_PLAYBACK:
    //  printf("Setting up Playback viewer...\n");
    //  return new cPlaybackView((cPlaybackPopulation *) population);
  default:
    printf("Invalid Viewer Type!\n");
    exit(0);
	return NULL;	// this is a no-op but the VC++ compiler throws warnings about not
					// returning values w/out this.
  }
#endif
}

void cPopulation::NotifyUpdate()
{
  // If we are in a view mode that can handle it,
  // update the information on the view-screen.

#ifndef VIEW_PRIMITIVE
  viewer->NotifyUpdate();
#endif
}

void cPopulation::NotifyError(const cString & in_string)
{
  // Send an error to the user.

#ifndef VIEW_PRIMITIVE
  viewer->NotifyError(in_string);
#endif
}

void cPopulation::NotifyWarning(const cString & in_string)
{
  // Send a warning to the user.

#ifndef VIEW_PRIMITIVE
  viewer->NotifyWarning(in_string);
#endif
}

void cPopulation::NotifyComment(const cString & in_string)
{
  // Send a commment to the user.

#ifndef VIEW_PRIMITIVE
  viewer->NotifyComment(in_string);
#endif
}

void cPopulation::Pause()
{
#ifndef VIEW_PRIMITIVE
  viewer->Pause();
#endif
}

// This function takes in the genesis file and the input arguments, and puts
// out a pointer to a built-up cGenesis object.
cGenesis * ProcessConfiguration(int argc, char * argv[])
{
  cGenesis * genesis = NULL;  // Genesis file storing info.
  int arg_num = 1;            // Argument number being looked at.

  // Load all of the args into string objects for ease of access.
  cString * args = new cString[argc];
  for (int i = 0; i < argc; i++) args[i] = argv[i];

  // First check to see if we should use an alternate genesis file.

  if (argc > 1 && (args[1] == "-g" || args[1] == "-genesis")) {
    // There must be one more arg here for the filename...
    if (argc < 3) {
      printf("Need filename for genesis file used.\n");
      exit(0);
    }
    genesis = new cGenesis(argv[2]);
    arg_num += 2;
  } else {
    genesis = new cGenesis;
  }

  // Make sure we have the proper genesis file for this version of avida.

  cString version_id = genesis->ReadString("VERSION_ID", "Unknown");
  if (version_id != AVIDA_VERSION) {
    printf("Error: Using incorrect genesis file.\n");
    printf("       Version needed = \"%s\".   Version used = \"%s\"\n",
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
      // printf("  -p[layback]              Engage playback mode\n");
      printf("  -s[eed] <value>          Set random seed to <value>\n");
      printf("  -v[ersion]               Prints the version number\n");
      exit(0);
      // } else if (args[arg_num] == "-playback" || args[arg_num] == "-p") {
      //  genesis->AddInput("MODE", MODE_PLAYBACK);
    } else if (args[arg_num] == "-seed" || args[arg_num] == "-s") {
      int in_seed = 0;
      if (arg_num + 1 == argc || args[arg_num + 1][0] == '-') {
	printf("Must include a number as the random seed!\n");
	exit(0);
      } else {
	arg_num++;
	in_seed = args[arg_num].AsInt();
      }
      genesis->AddInput("RANDOM_SEED", in_seed);
    } else if (args[arg_num] == "-version" || args[arg_num] == "-v") {
      printf("Avida version 1.0.0 by Charles Ofria\n");
      printf(" designed by Charles Ofria, Chris Adami, and C. Titus Brown\n");
      printf(" portions of avida were also coded by Travis Collier and Dennis Adler\n");
      exit(0);
    } else {
      printf("Unknown Option: %s\n", argv[arg_num]);
      printf("Type: \"%s -h\" for a full list of options.\n", argv[0]);
      exit(0);
    }

    arg_num++;
  }

  int rand_seed = genesis->ReadInt("RANDOM_SEED");
  printf("Random Seed: %d\n", rand_seed);
  g_random.ResetSeed(rand_seed);

  delete [] args;

  return genesis;
}

void SetupDebug(int debug_level)
{
#ifdef DEBUG
  // Create and initialize debug variable...
    
  g_debug.SetLevel(debug_level);
  g_debug.Error("Testing...");
  g_debug.Warning("Testing...");
  g_debug.Comment("Testing...");
#else
  (void) debug_level;
#endif
}

void CollectData(cLocalStats * local_stats, cGenebank * genebank)
{
  int update = stats.GetUpdate();

  // Tell the stats object to do update calculations and printing.
  local_stats->ProcessUpdate(update);
  
  // Print any status files...

  int interval = 0;

  // genotype.status

  interval = local_stats->GetGenotypeStatusInterval();
  if (update && interval && (update % interval == 0)) {
    local_stats->GetGenotypeStatusFP()<<update;
    for (cGenotype * cur_genotype = genebank->ResetThread(0);
	 cur_genotype != NULL && cur_genotype->GetThreshold();
	 cur_genotype = genebank->NextGenotype(0)) {
      local_stats->GetGenotypeStatusFP()
	<<" : "<<cur_genotype->GetID()<<" "
	<<cur_genotype->GetNumCPUs()<<" "
	<<cur_genotype->GetSpecies()->GetID()<<" "
	<<cur_genotype->GetLength();
    }
    local_stats->GetGenotypeStatusFP()<<endl;
    local_stats->GetGenotypeStatusFP().flush();
  }

  // diversity.status
  interval = local_stats->GetDiversityStatusInterval();
  if (update && interval && (update % interval == 0)) {
    int distance_chart[MAX_CREATURE_SIZE];
    int distance, cur_size, max_distance = 0, i;
    
    for (i = 0; i < MAX_CREATURE_SIZE; i++) distance_chart[i] = 0;
    
    // compare all the pairs of genotypes.
    
    for (cGenotype * cur_genotype = genebank->ResetThread(0);
	 cur_genotype != NULL && cur_genotype->GetThreshold();
	 cur_genotype = genebank->NextGenotype(0)) {
      cur_size = cur_genotype->GetNumCPUs();
      
      // Place the comparisions on this genotype to itself in the chart.
      distance_chart[0] += cur_size * (cur_size - 1) / 2;
      
      // Compare it to all the genotypes which come before it in the queue.
      for (cGenotype * genotype2 = genebank->ResetThread(1);
	   genotype2 != cur_genotype;
	   genotype2 = genebank->NextGenotype(1)) {
	distance =
	  cur_genotype->GetCode()->FindSlidingDistance(genotype2->GetCode());
	distance_chart[distance] += cur_size * genotype2->GetNumCPUs();
	if (distance > max_distance) max_distance = distance;
      }
    }
    
    // Finally, print the results.
    local_stats->GetDiversityStatusFP()<<stats.GetUpdate();
    for (i = 0; i < max_distance; i++) 
      local_stats->GetDiversityStatusFP()<<" "<<distance_chart[i];
    local_stats->GetDiversityStatusFP()<<endl;
  }
  local_stats->GetDiversityStatusFP().flush();
  
  // Print the memory-use stats. (If testing is turned on).
  g_memory.Print(update);

  // Update all the genotypes for the end of this update.

  for (cGenotype * cur_genotype = genebank->ResetThread(0);
       cur_genotype != NULL && cur_genotype->GetThreshold();
       cur_genotype = genebank->NextGenotype(0)) {
    cur_genotype->UpdateReset();
  }
}
