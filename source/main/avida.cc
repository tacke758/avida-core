//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "avida.hh"

#include "population.hh"

#include "genotype.hh"
#include "genebank.hh"
#include "species.hh"

#include "../event/event_ipc.hh"
#include "../event/event_list.hh"
#include "../event/avida_main_event_handler.hh"
#include "../event/population_event_handler.hh"

#include "../viewers/baseview.hh"
#include "../viewers/view.hh"
//#include "../viewers/eview_event_handler.hh"

#include "../viewers/primitiveview.hh"
#include "../viewers/externalview.hh"
#ifdef VIEW_VISTA
#include "../vista/vista_view.hh"
#endif

// cAvidaMain static variable definitions
cEventManager * cAvidaMain::event_manager = NULL;
cEventList * cAvidaMain::event_list = NULL;
cEventIPC * cAvidaMain::event_ipc = NULL;
cPopulation * cAvidaMain::population = NULL;
int cAvidaMain::main_eh_id = -1;
int cAvidaMain::population_eh_id = -1;
cView * cAvidaMain::viewer = NULL;  // @TCC old Viewer
cBaseView * cAvidaMain::base_viewer = NULL;
bool cAvidaMain::step_flag = false;
bool cAvidaMain::paused_flag = false;
bool cAvidaMain::done_flag = false;


int main(int argc, char * argv[]) {
  // Catch Interrupt making sure to close appropriately
  signal(SIGINT, cAvidaMain::ExitProgram);

  // output copyright message
  printf( "Avida version %s\nCopyright (C) 1993-2000 California Institute of Technology.\n\n", AVIDA_VERSION );
  printf( "Avida comes with ABSOLUTELY NO WARRANTY.\n" );
  printf( "This is free software, and you are welcome to redistribute it\nunder certain conditions. See file COPYING for details.\n\n" );

  // Initialize Static Classes... Setting everything up
  cConfig::Setup(argc, argv);
  cConfig::SetupMS();
  cStats::Setup();
  cAvidaMain::Setup();

  // And finally, process the population...
  cAvidaMain::Run();


  // Exit Nicely
  cAvidaMain::ExitProgram(0);
  return 0;
}


void cAvidaMain::Setup(){

  base_viewer = new cPrimitiveView; // Temporary viewer for while setting up

  // Setup Population
  cout<<"Initializing Population..."<<flush;
  population = new cPopulation();
  cTestCPU::Init();
  population->InitSoup();
  cout<<" ...done"<<endl;

  // Setup Event List
  cout<<"Initializing Event Manager..."<<flush;
  event_manager = new cEventManager;
  event_list = new cEventList(event_manager);
  cout<<"...EventIPC..."<<flush;
  event_ipc = new cEventIPC("avida_ipc.in", "avida_ipc.out");
  cout<<"...Handlers..."<<flush;
  main_eh_id =
    event_manager->AddHandler(new cAvidaMainEventHandler(event_list));
  population_eh_id =
    event_manager->AddHandler(new cPopulationEventHandler(population));
  cout<<" ...done"<<endl;

  cout<<"Reading Event List File..."<<flush;
  ReadEventListFile(cConfig::GetEventFilename());
  cout<<" ...done"<<endl;


  // check for directory genebank
  printf( "Checking for directory 'genebank'\n" );
  FILE *fp = fopen ( "genebank", "r" );
  if ( fp == 0 ){
    if ( errno == ENOENT ){
      printf( " 'genebank' doesn't exist. Has to be created...\n" );
      if ( mkdir( "genebank", ACCESSPERMS ) )
	printf( " Error creating 'genebank'. Creatures cannot be stored successfully!\n" );
      else printf( " ...done\n" );
    }
    else printf( " Error opening 'genebank'.  Creatures cannot be stored successfully!\n" );
  }
  else printf( " ...done\n" );

  // Setup Viewer
  cout<<"Initializing Viewer... "<<flush;
#ifdef VIEW_NEWVIEWER
  cAvidaMain::SetupViewer(cConfig::GetViewerType());
#endif
  // OLD VIEWER STUFF
#ifdef VIEW_PRIMITIVE
  viewer = new cView;
#else
  viewer = new cView(*population);
  viewer->SetViewMode(cConfig::GetViewMode());
#endif
  cout<<" ...done"<<endl;
}


void cAvidaMain::SetupViewer(const int viewer_type){
  switch( viewer_type ){
    case 0:
      StartViewer("primitive");
      break;	
    case 1:
      StartViewer("vista");
      break;
    default:
      cerr<<endl<<"Error: Nonvalid Viewer Type "<<viewer_type<<endl;
  }
}



void cAvidaMain::Run(){
  assert( population != NULL );
  // All normal exit is handled through GetDoneFlag()
  // MaxGenerations & MaxUpdates are added to event_list if in genesis
  while( !GetDoneFlag() ){
    GetEvents();
    ProcessPause();
    if( !GetDoneFlag() ){
      ProcessPopulationUpdate();
    }
  }
}


void cAvidaMain::ProcessPause(){
  // Handle Pause State
  if( IsPaused() && !GetDoneFlag() ){

    base_viewer->NotifyPause();
    while( IsPaused() && !GetDoneFlag() ){
      GetEvents();
      // Some wait function...
      //usleep(500);
      base_viewer->DoUpdate();
    }

    if( step_flag == true ){ // If stepping, set for immediate pause
      paused_flag = true;
    }

  }else{ // Can't be stepping if pause is off!
    step_flag = false;
  }
}

void cAvidaMain::GetEvents(){
  event_ipc->GetEvents();
  event_list->Process();
  base_viewer->GetEvents();
}

void cAvidaMain::ProcessPopulationUpdate(){
  if( !GetDoneFlag() ){
    cStats::IncCurrentUpdate();

    // Handle all data collection for previous update.
    if (cStats::GetUpdate() > 0) CollectData(population->GetGenebank());

    // Process the update.
    population->DoUpdate();

    // Setup the viewer for the new update.
    NotifyUpdate();

    // Exit conditons...
    if (population->GetNumCreatures() == 0) SetDoneFlag();
  }
}


bool cAvidaMain::ProcessEvent(const cString & name, const cString & args,
			      int event_handler_id){
  assert(event_manager!=NULL);
  return event_manager->ProcessEvent(name,args, event_handler_id);
}

bool cAvidaMain::ProcessEvent(int event_enum, const cString & args,
			      int event_handler_id){
  assert(event_manager!=NULL);
  return event_manager->ProcessEvent(event_enum, args, event_handler_id);
}


bool cAvidaMain::AddEventFileFormat(const cString & event_line){
  if( event_list->AddEventFileFormat(event_line) ){
    cerr<<"Event List Added : "<<event_line<<endl;
  }else{
    cString mesg;
    mesg.Set("Unkown Event [%s]",event_line());
    NotifyWarning(mesg);
    return false;
  }
  return true;
}


void cAvidaMain::SyncEventList(){
  if( event_list != NULL ){
    event_list->Sync();
  }
}


void cAvidaMain::ReadEventListFile(const cString & filename){
  cInitFile event_file(filename);

  // Load in the proper event list and set it up.
  event_file.Load();
  event_file.Compress();

  // Loop through the line_list and change the lines to events.
  while ( event_file.GetNumLines()>0 ) {
    AddEventFileFormat(event_file.RemoveLine());
  }

  ////////// Adding events from the Genesis file //////////
  // Max Updates and/or Max Generations
  if( cConfig::GetEndConditionMode() == 0 ){
    if( cConfig::GetMaxUpdates() >= 0 ){
      event_list->AddEvent( UPDATE, cConfig::GetMaxUpdates(),
			    TRIGGER_ONCE, TRIGGER_END, "exit", "");
    }
    if( cConfig::GetMaxGenerations() >= 0 ){
      event_list->AddEvent( GENERATION, cConfig::GetMaxGenerations(),
			    TRIGGER_ONCE, TRIGGER_END, "exit", "");
    }
  }else{
    if( cConfig::GetMaxUpdates() >= 0 && cConfig::GetMaxGenerations() >= 0 ){
      cString max_gen_str;
      max_gen_str.Set("%d",cConfig::GetMaxGenerations());
      cString max_update_str;
      max_update_str.Set("%d",cConfig::GetMaxUpdates());
      event_list->AddEvent( UPDATE, cConfig::GetMaxUpdates(),
			    1, TRIGGER_END,
			    "exit_if_generation_greater_than", max_gen_str);
      event_list->AddEvent( GENERATION, cConfig::GetMaxGenerations(),
			    .01, TRIGGER_END,
			    "exit_if_update_greater_than", max_update_str);
    }
  }


  // Data File Intervals
  if( cConfig::GetAverageDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetAverageDataInterval(),
			  TRIGGER_END, "print_average_data", "");
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetAverageDataInterval(),
			  TRIGGER_END, "print_error_data", "");
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetAverageDataInterval(),
			  TRIGGER_END, "print_variance_data", "");
  }
  if( cConfig::GetDominantDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetDominantDataInterval(),
			  TRIGGER_END, "print_dominant_data", "");
  }
  if( cConfig::GetCountDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetCountDataInterval(),
			  TRIGGER_END, "print_count_data", "");
  }
  if( cConfig::GetTotalsDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetTotalsDataInterval(),
			  TRIGGER_END, "print_totals_data", "");
  }
  if( cConfig::GetTasksDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetTasksDataInterval(),
			  TRIGGER_END, "print_tasks_data", "");
  }
  if( cConfig::GetStatsDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetStatsDataInterval(),
			  TRIGGER_END, "print_stats_data", "");
  }
  if( cConfig::GetTimeDataInterval() ){
    event_list->AddEvent( UPDATE, TRIGGER_BEGIN,
			  cConfig::GetTimeDataInterval(),
			  TRIGGER_END, "print_time_data", "" );
  }
}


// @TCC -- Move out of cAvidaMain
void cAvidaMain::CollectData(cGenebank & genebank)
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



void cAvidaMain::ExitProgram(int ignore){
  signal(SIGINT, SIG_IGN);          // Ignore all future interupts.
  if( population != NULL ){
    population->Clear();
  }
  cStats::Shutdown();
  if( base_viewer != NULL ){
    base_viewer->NotifyExit();
  }
  if( viewer != NULL ){
    EndProg(ignore); // Remove when all viewers fixed to handle NotifyExit()
  }
  exit(0);
}



void cAvidaMain::NotifyUpdate(){
  base_viewer->NotifyUpdate();
#ifndef VIEW_PRIMITIVE
  viewer->NotifyUpdate();
#endif
  viewer->NewUpdate();
}

void cAvidaMain::NotifyBirth(int p_cell, int d_cell){
  base_viewer->NotifyBirth(p_cell, d_cell);
  ProcessPause();
}


void cAvidaMain::NotifyError(const cString & in_string){
  // Send an error to the user.
  base_viewer->NotifyError(in_string);
#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  viewer->NotifyError(in_string);
#endif
}

void cAvidaMain::NotifyWarning(const cString & in_string) {
  // Send a warning to the user.
  base_viewer->NotifyWarning(in_string);
#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  viewer->NotifyWarning(in_string);
#endif
}

void cAvidaMain::NotifyComment(const cString & in_string) {
  // Send a commment to the user.
  base_viewer->NotifyComment(in_string);
#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  viewer->NotifyComment(in_string);
#endif
}




void cAvidaMain::StartViewer(const cString & viewer_name){
  if( viewer_name == "primitive" ){
    cerr<<"STARTING PRIMITIVE VIEWER"<<endl;
    delete base_viewer;
    base_viewer = new cPrimitiveView;
  }else if( viewer_name == "vista" ){
#ifdef VIEW_VISTA
    cerr<<"STARTING VISTA VIEWER"<<endl;
    delete base_viewer;
    base_viewer = new cVistaView;
#else
    cerr<<"Error: Vista Viewer not enabled (compile option)"<<endl;
#endif
  }else{
    cString mesg;
    mesg.Set("Error: Unknown Viewer \"%s\"",viewer_name());
    cerr<<mesg<<endl;
    //NotifyWarning(mesg);
  }
}


// Stuff for viewer.... @TCCC Shouldn't have to be here
cGenotype * cAvidaMain::GetBestGenotype(){
  assert( population != NULL );
  return population->GetGenebank().GetBestGenotype();
}

cGenebank * cAvidaMain::GetGenebank(){
  assert( population != NULL );
  return &(population->GetGenebank());
}

cGenotype * cAvidaMain::GetCellGenotype(int cpu_id){
  assert( population != NULL );
  assert( cpu_id >= 0  &&  cpu_id < population->GetSize() );
  return population->GetCPU(cpu_id).GetActiveGenotype();
}

const cBaseCPU * cAvidaMain::GetCellCPU(int cpu_id){
  assert( population != NULL );
  assert( cpu_id >= 0  &&  cpu_id < population->GetSize() );
  return &(population->GetCPU(cpu_id));
}



























// @TCC -- Why isn't this in population.cc  (actually, moved to avida main)
void cPopulation::NotifyUpdate()
{
  // If we are in a view mode that can handle it,
  // update the information on the view-screen.

#ifndef VIEW_PRIMITIVE
  cAvidaMain::viewer->NotifyUpdate();
#endif
}

void cPopulation::NotifyError(const cString & in_string)
{
  // Send an error to the user.

#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  cAvidaMain::viewer->NotifyError(in_string);
#endif
}

void cPopulation::NotifyWarning(const cString & in_string)
{
  // Send a warning to the user.

#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  cAvidaMain::viewer->NotifyWarning(in_string);
#endif
}

void cPopulation::NotifyComment(const cString & in_string)
{
  // Send a commment to the user.

#ifdef VIEW_PRIMITIVE
  fprintf(stderr, "%s\n", in_string());
#else
  cAvidaMain::viewer->NotifyComment(in_string);
#endif
}

void cPopulation::Pause() {
  cAvidaMain::TogglePause();
#ifndef VIEW_PRIMITIVE
  cAvidaMain::viewer->Pause();
#endif
}

void cPopulation::DoBreakpoint()
{
#ifndef VIEW_PRIMITIVE
  cAvidaMain::viewer->DoBreakpoint();
#endif
}


