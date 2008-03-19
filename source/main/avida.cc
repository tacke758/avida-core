//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <signal.h>

#include "../tools/slice.hh"

#include "avida.hh"

#include "analyze.hh"
#include "callback_util.hh"
#include "config.hh"
#include "../cpu/hardware_method.hh"
#include "environment.hh"
#include "genebank.hh"
#include "genome_util.hh"
#include "genotype.hh"
#include "inst_util.hh"
#include "organism.hh"
#include "population.hh"
#include "population_cell.hh"
#include "species.hh"

#include "../cpu/test_cpu.hh"
#include "../cpu/hardware_base.hh"
#include "../cpu/hardware_factory.hh"
#include "../cpu/hardware_util.hh"
/* FIXME beware of hardwiring. -- kgn */
#include "hardware_cpu.hh"
/* XXX */

#include "../event/event.hh"
#include "../event/event_factory.hh"
#include "../event/event_factory_manager.hh"
#include "../event/event_list.hh"
#include "../event/population_event_factory.hh"


using namespace std;


// Static variable definitions
cAvidaDriver_Base * cAvidaDriver_Base::main_driver = NULL;

void ExitAvida(int exit_code)
{
  signal(SIGINT, SIG_IGN);          // Ignore all future interupts.
  delete cAvidaDriver_Base::main_driver;
  exit(exit_code);
}


void SetupAvida(int argc, char * argv[],
		cEnvironment &environment,
		cPopulationInterface &test_interface )
{
  // Catch Interrupt making sure to close appropriately
  signal(SIGINT, ExitAvida);

  // output copyright message
  printf( "Avida version %s\nCopyright (C) 1993-2001 California Institute of Technology.\n\n", AVIDA_VERSION );
  printf( "Avida comes with ABSOLUTELY NO WARRANTY.\n" );
  printf( "This is free software, and you are welcome to redistribute it\nunder certain conditions. See file COPYING for details.\n\n" );

  // Initialize the configuration data...
  cConfig::Setup(argc, argv);
  cConfig::SetupMS();

  // Initialize the default environment...
   if (environment.Load(cConfig::GetEnvironmentFilename()) == false) {
    cerr << "Unable to load environment... aborting!" << endl;
    ExitAvida(-1);
  }
  /* FIXME Evil hardwiring! -- kgn

  if you wanted to know where cHardwareCPU's instruction superlibrary
  was hardwired into the environment, here's where it happens:
  */
  environment.GetInstLib().SetInstSuperlib(cHardwareCPU::GetInstSuperlib());
  /* XXX */
  cHardwareUtil::LoadInstLibCPU(cConfig::GetInstFilename(),
				environment.GetInstLib());
  cConfig::SetNumInstructions(environment.GetInstLib().GetSize());
  cConfig::SetNumTasks(environment.GetTaskLib().GetSize());
  cConfig::SetNumReactions(environment.GetReactionLib().GetSize());
  cConfig::SetNumResources(environment.GetResourceLib().GetSize());

  // Build a test population interface.
  test_interface.SetFun_NewHardware(&cCallbackUtil::CB_NewHardware);
  test_interface.SetFun_Recycle(&cCallbackUtil::CB_RecycleHardware);
  test_interface.SetFun_Divide(&cCallbackUtil::CB_TestDivide);
  test_interface.SetFun_GetInput(&cCallbackUtil::CB_GetInput);
  test_interface.SetFun_GetResources(&cCallbackUtil::CB_GetResources);
  test_interface.SetFun_UpdateResources(&cCallbackUtil::CB_UpdateResources);

  // Setup the test CPUs.
  cTestCPU::Setup( &(environment.GetInstLib()), &environment, test_interface );
}


///////////////////////
//  cAvidaDriver_Base
///////////////////////

cAvidaDriver_Base::cAvidaDriver_Base()
  : done_flag(false)
{
}

cAvidaDriver_Base::~cAvidaDriver_Base()
{
}

void cAvidaDriver_Base::ExitProgram(int exit_code)
{
  exit(exit_code);   // If nothing is setup, nothing needs to be shutdown.
}

void cAvidaDriver_Base::NotifyComment(const cString & in_string)
{
  cout << in_string << endl;  // Just output
}

void cAvidaDriver_Base::NotifyWarning(const cString & in_string)
{
  cerr << in_string << endl;  // Just output
}

void cAvidaDriver_Base::NotifyError(const cString & in_string)
{
  cerr << in_string << endl;  // Just output
}


//////////////////////////
//  cAvidaDriver_Analyze
//////////////////////////

cAvidaDriver_Analyze::cAvidaDriver_Analyze()
{
}

cAvidaDriver_Analyze::~cAvidaDriver_Analyze()
{
}

void cAvidaDriver_Analyze::Run()
{
  cout << "In analyze mode!!" << endl;
  cAnalyze analyze(cConfig::GetAnalyzeFilename());
}


/////////////////////////////
//  cAvidaDriver_Population
/////////////////////////////

cAvidaDriver_Population::cAvidaDriver_Population(cEnvironment & environment)
  : population(NULL)
  , event_manager(NULL)
  , event_list(NULL)
{
  // Setup Population
  cout << "Initializing Population..." << flush;
  cPopulationInterface default_interface;
  default_interface.SetFun_NewHardware(&cCallbackUtil::CB_NewHardware);
  default_interface.SetFun_Recycle(&cCallbackUtil::CB_RecycleHardware);
  default_interface.SetFun_Divide(&cCallbackUtil::CB_Divide);
  default_interface.SetFun_TestOnDivide(&cCallbackUtil::CB_TestOnDivide);
  default_interface.SetFun_GetNeighbor(&cCallbackUtil::CB_GetNeighbor);
  default_interface.SetFun_NumNeighbors(&cCallbackUtil::CB_GetNumNeighbors);
  default_interface.SetFun_Rotate(&cCallbackUtil::CB_Rotate);
  default_interface.SetFun_Breakpoint(&cCallbackUtil::CB_Breakpoint);
  default_interface.SetFun_TestFitness(&cCallbackUtil::CB_TestFitness);
  default_interface.SetFun_GetInput(&cCallbackUtil::CB_GetInput);
  default_interface.SetFun_Debug(&cCallbackUtil::CB_Debug);
  default_interface.SetFun_GetResources(&cCallbackUtil::CB_GetResources);
  default_interface.SetFun_UpdateResources(&cCallbackUtil::CB_UpdateResources);
  default_interface.SetFun_KillCell(&cCallbackUtil::CB_KillCell);

  population = new cPopulation(default_interface, environment);
  cout << " ...done" << endl;

  // Setup Event List
  cout<<"Initializing Event Factory Manager..."<<flush;
  event_manager = new cEventFactoryManager;
  cStats & stats = population->GetStats();
  event_list = new cEventList( event_manager, new cAvidaTriggers(stats) );
  cout<<"...Factories..."<<flush;

  // in principle, one could add more than one event factory here.
  // however, this is not a good idea, because the automatic documentation
  // system cannot cope with this at this point. Claus
  event_manager->AddFactory(new cPopulationEventFactory(population));
  cout<<" ...done"<<endl;

  cout<<"Reading Event List File..."<<flush;
  ReadEventListFile(cConfig::GetEventFilename());
  cout<<" ...done"<<endl;

  // Make sure the directory 'genebank' exits!
  cTools::MkDir("genebank", true);
}

cAvidaDriver_Population::~cAvidaDriver_Population()
{
  if (population != NULL) delete population;
}

void cAvidaDriver_Population::Run()
{
  // cout << "DEBUG: Turning control over to driver..." << endl;

  assert( population != NULL );

  // Process until done...
  while ( !ProcessUpdate() ) {} 
}

bool cAvidaDriver_Population::ProcessUpdate()
{
  GetEvents();
  if (done_flag == true) return true;

  // Increment the Update.
  cStats & stats = population->GetStats();
  stats.IncCurrentUpdate();

  // Handle all data collection for previous update.
  if (stats.GetUpdate() > 0) {
    // Tell the stats object to do update calculations and printing.
    stats.ProcessUpdate();

    // Update all the genotypes for the end of this update.
    cGenebank & genebank = population->GetGenebank();
    for (cGenotype * cur_genotype = genebank.ResetThread(0);
	 cur_genotype != NULL && cur_genotype->GetThreshold();
	 cur_genotype = genebank.NextGenotype(0)) {
      cur_genotype->UpdateReset();
    }
  }

  ProcessOrganisms();

  // Do Point Mutations
  if (cConfig::GetPointMutProb() > 0 ) {
    for (int i = 0; i < population->GetSize(); i++) {
      if (population->GetCell(i).IsOccupied()) {
	population->GetCell(i).GetOrganism()->
	  GetHardware().PointMutate(cConfig::GetPointMutProb());
      }
    }
  }
  

#ifdef DEBUG_CHECK_OK
  // If we're in a debug mode, make sure the population is OK.
  if (population->OK() == false) {
    g_debug.Warning("Population::OK() is failing.");
  }
#endif

  // Do any cleanup in the hardware factory...
  cHardwareFactory::Update();
  
  // Exit conditons...
  if (population->GetNumOrganisms() == 0) done_flag = true;
  
  return done_flag;
}


void cAvidaDriver_Population::NotifyUpdate()
{
  // Nothing here for now.  This method should be overloaded and only
  // run if there is no proper viewer.
}


void cAvidaDriver_Population::GetEvents()
{
  assert(population != NULL);

  if (population->GetSyncEvents() == true) {
    SyncEventList();
    population->SetSyncEvents(false);
  }
  event_list->Process();
}

void cAvidaDriver_Population::ProcessOrganisms()
{
  //  cout << "DEBUG: Running viewer-less update..." << endl;

  // Process the update.
  const int UD_size =
    cConfig::GetAveTimeslice() * population->GetNumOrganisms();
  const double step_size = 1.0 / (double) UD_size;

  for (int i = 0; i < UD_size; i++) {
    if (population->GetNumOrganisms() == 0) {
      done_flag = true;
      break;
    }
    population->ProcessStep(step_size);
  }

  // end of update stats...
  population->CalcUpdateStats();

  // No viewer; print out status for this update....
  cStats & stats = population->GetStats();
  cout << "UD: "  << stats.GetUpdate() << "\t"
       << "Gen: " << stats.SumGeneration().Average() << "\t"
       << "Fit: " << stats.GetAveFitness() << "\t"
       << "Size: " << population->GetNumOrganisms()
       << endl;
}


void cAvidaDriver_Population::
ReadEventListFile(const cString & filename)
{
  cInitFile event_file(filename);

  // Load in the proper event list and set it up.
  event_file.Load();
  event_file.Compress();

  // Loop through the line_list and change the lines to events.
  while ( event_file.GetNumLines()>0 ) {
    event_list->AddEventFileFormat(event_file.RemoveLine());
  }

  ////////// Adding events from the Genesis file //////////
  // Max Updates and/or Max Generations
  if( cConfig::GetEndConditionMode() == 0 ){
    if( cConfig::GetMaxUpdates() >= 0 ){
      event_list->AddEvent( cEventTriggers::UPDATE, cConfig::GetMaxUpdates(),
			    cEventTriggers::TRIGGER_ONCE,
			    cEventTriggers::TRIGGER_END, "exit", "");
    }
    if( cConfig::GetMaxGenerations() >= 0 ){
      event_list->AddEvent( cEventTriggers::GENERATION,
			    cConfig::GetMaxGenerations(),
			    cEventTriggers::TRIGGER_ONCE,
			    cEventTriggers::TRIGGER_END, "exit", "");
    }
  }else{
    if( cConfig::GetMaxUpdates() >= 0 && cConfig::GetMaxGenerations() >= 0 ){
      cString max_gen_str;
      max_gen_str.Set("%d",cConfig::GetMaxGenerations());
      cString max_update_str;
      max_update_str.Set("%d",cConfig::GetMaxUpdates());
      event_list->AddEvent( cEventTriggers::UPDATE, cConfig::GetMaxUpdates(),
			    1, cEventTriggers::TRIGGER_END,
			    "exit_if_generation_greater_than", max_gen_str);
      event_list->AddEvent( cEventTriggers::GENERATION,
			    cConfig::GetMaxGenerations(),
			    .01, cEventTriggers::TRIGGER_END,
			    "exit_if_update_greater_than", max_update_str);
    }
  }

}

void cAvidaDriver_Population::SyncEventList()
{
  if( event_list != NULL ){
    event_list->Sync();
  }
}

