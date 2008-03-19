//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include "avida.hh"
#include "genotype.hh"
#include "genebank.hh"
#include "species.hh"
#include "../viewers/view.hh"
#include <stdlib.h>

int main(int argc, char * argv[])
{
  cConfig::Setup(argc, argv);
  cConfig::SetupMS();
  cStats::Setup();

  // Setup the population & viewer
  printf("Initializing Population...\n");
  cPopulation population;

#ifdef VIEW_PRIMITIVE
  cView viewer;
#else
  printf("Setting up Viewer...\n");
  cView viewer(population);
#endif

  population.SetViewer(viewer);

  // Initialize the population & viewer.
  cTestCPU::Init();
  population.InitSoup();
  viewer.SetViewMode(cConfig::GetViewMode());

  // And finally, process the population...

#ifndef VIEW_PRIMITIVE  
  viewer.NotifyUpdate();
  viewer.Refresh();
#endif
  
  while( ( cConfig::GetMaxUpdates() < 0 ||
	   cStats::GetUpdate() < cConfig::GetMaxUpdates())  &&
	 ( cConfig::GetMaxGenerations() < 0 ||
	   cStats::GetAveGeneration() <= cConfig::GetMaxGenerations()) ){
    cStats::IncCurrentUpdate();
    population.ProcessEvents();

    // Setup the viewer for the new update.
    viewer.NewUpdate();

    // Handle all data collection for previous update.
    if (cStats::GetUpdate() > 0) CollectData(population.GetGenebank());

    // Process the update.
    population.DoUpdate();
    if (population.GetNumCreatures() == 0) break;
  }

  population.Clear();
  cStats::Shutdown();
  EndProg(1);

  return 0;
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

#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  viewer->NotifyError(in_string);
#endif
}

void cPopulation::NotifyWarning(const cString & in_string)
{
  // Send a warning to the user.

#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  viewer->NotifyWarning(in_string);
#endif
}

void cPopulation::NotifyComment(const cString & in_string)
{
  // Send a commment to the user.

#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  viewer->NotifyComment(in_string);
#endif
}

void cPopulation::Pause()
{
#ifndef VIEW_PRIMITIVE
  viewer->Pause();
#endif
}

void cPopulation::DoBreakpoint()
{
#ifndef VIEW_PRIMITIVE
  viewer->DoBreakpoint();
#endif
}

void CollectData(cGenebank & genebank)
{
  int update = cStats::GetUpdate();

  // Tell the stats object to do update calculations and printing.
  cStats::ProcessUpdate(update);

  // Print any status files...

  int interval = 0;

  // genotype.status

  interval = cConfig::GetGenotypeStatusInterval();
  if (update && interval && (update % interval == 0)) {
    cStats::GetGenotypeStatusFP()<<update;
    for (cGenotype * cur_genotype = genebank.ResetThread(0);
	 cur_genotype != NULL && cur_genotype->GetThreshold();
	 cur_genotype = genebank.NextGenotype(0)) {
      cStats::GetGenotypeStatusFP()
	<<" : "<<cur_genotype->GetID()<<" "
	<<cur_genotype->GetNumCPUs()<<" "
	<<cur_genotype->GetSpecies()->GetID()<<" "
	<<cur_genotype->GetLength();
    }
    cStats::GetGenotypeStatusFP() << endl;
    cStats::GetGenotypeStatusFP().flush();
  }

  // diversity.status
  interval = cConfig::GetDiversityStatusInterval();
  if (update && interval && (update % interval == 0)) {
    int distance_chart[MAX_CREATURE_SIZE];
    int distance, cur_size, max_distance = 0, i;

    for (i = 0; i < MAX_CREATURE_SIZE; i++) distance_chart[i] = 0;

    // compare all the pairs of genotypes.

    for (cGenotype * cur_genotype = genebank.ResetThread(0);
	 cur_genotype != NULL && cur_genotype->GetThreshold();
	 cur_genotype = genebank.NextGenotype(0)) {
      cur_size = cur_genotype->GetNumCPUs();

      // Place the comparisions on this genotype to itself in the chart.
      distance_chart[0] += cur_size * (cur_size - 1) / 2;

      // Compare it to all the genotypes which come before it in the queue.
      for (cGenotype * genotype2 = genebank.ResetThread(1);
	   genotype2 != cur_genotype;
	   genotype2 = genebank.NextGenotype(1)) {
	distance =
	  cur_genotype->GetCode().FindSlidingDistance(genotype2->GetCode());
	distance_chart[distance] += cur_size * genotype2->GetNumCPUs();
	if (distance > max_distance) max_distance = distance;
      }
    }

    // Finally, print the results.
    cStats::GetDiversityStatusFP() << cStats::GetUpdate();
    for (i = 0; i < max_distance; i++)
      cStats::GetDiversityStatusFP()<<" "<<distance_chart[i];
    cStats::GetDiversityStatusFP()<<endl;
  }
  cStats::GetDiversityStatusFP().flush();

  // Update all the genotypes for the end of this update.
  for (cGenotype * cur_genotype = genebank.ResetThread(0);
       cur_genotype != NULL && cur_genotype->GetThreshold();
       cur_genotype = genebank.NextGenotype(0)) {
    cur_genotype->UpdateReset();
  }
}
