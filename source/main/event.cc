#include <float.h>            // for DBL_MIN
#include <iostream.h>
#include "../defs.hh"
#include "../tools/file.hh"
#include "../tools/string.hh"
#include "stats.hh"
#include "population.hh"

#ifdef GENEOLOGY
#include "geneology.hh"
#endif

#include "event.hh"

#include "../cpu/environment.hh"
#include "../cpu/cpu.hh"
#include "../cpu/cpu.ii"



/////////////////
//  cEventList
/////////////////


cEventList::~cEventList() {
  while( head != NULL ){
    current = head;
    head = head->GetNext();
    delete head;
  }
}




void cEventList::BuildEventList(const cString & filename) {

  eTriggerVariable trigger;
  double start;
  double interval;
  double stop;
  cString name;
  cString arg_list;

  cInitFile event_file(filename);
  cString cur_word;
  cString tmp;

  // Load in the proper event list and set it up.
  event_file.Load();
  event_file.Compress();

  // Loop through the line_list and change the lines to events.
  while ( event_file.GetNumLines()>0 ) {
    cString cur_line = event_file.RemoveLine();

    // Get the trigger variable
    cur_word = cur_line.PopWord();
    if(        cur_word == "update"  ||
	       cur_word == "u" ){
      trigger = UPDATE;
    } else if( cur_word == "generation"  ||
	       cur_word == "g" ){
      trigger = GENERATION;
    } else {
      trigger = UNDEFINED;
    }

    // Get the start:interval:stop
    cur_word = cur_line.PopWord();
    tmp = cur_word.Pop(':');

    // First number is start
    if( tmp == "begin" ){
      start = TRIGGER_BEGIN;
    }else{
      start = tmp.AsDouble();
    }

    // If no other words... is "start" syntax
    if( cur_word.GetSize() == 0 ){
      interval = TRIGGER_ONCE;
      stop     = TRIGGER_END;
    }else{

      // Second word is interval
      tmp = cur_word.Pop(':');
      if( tmp == "all" ){
	interval = TRIGGER_ALL;
      }else if( tmp == "once" ){
	interval = TRIGGER_ONCE;
      }else{
	interval = tmp.AsDouble();
      }

      // If no other words... is "start:interval" syntax
      if( cur_word.GetSize() == 0 ){
	stop     = TRIGGER_END;
      }else{

	// We have "start:interval:stop" syntax
	tmp = cur_word;
	if( tmp == "end" ){
	  stop = TRIGGER_END;
	}else{
	  stop = tmp.AsDouble();
	}
      }
    }
    name = cur_line.PopWord();
    arg_list = cur_line;

    cerr<<trigger<<" "<<start<<":"<<interval<<":"<<stop<<" "
	<<name<<" "<<arg_list<<endl;

    AddEvent( trigger, start, interval, stop, name, arg_list );
  }
}



void cEventList::AddEvent( eTriggerVariable trigger,
			   double start, double interval, double stop,
			   cString & name, const cString & arg_list ){
  cEvent * event = new cEvent(trigger, start, interval, stop, name, arg_list);
  num_events ++;

  // If there are no events in the list yet.
  if( tail == NULL ){
    assert( head == NULL );
    head = event;
    tail = event;
  }else{
    // Add to the end of the list
    tail->SetNext(event);
    event->SetPrev(tail);
    tail = event;
  }
}


void cEventList::Delete(){
  assert( current != NULL );

  if( current->GetPrev() != NULL ){
    current->GetPrev()->SetNext(current->GetNext());
  }else{
    assert( current == head );
    head = current->GetNext();
  }
  if( current->GetNext() != NULL ){
    current->GetNext()->SetPrev(current->GetPrev());
  }else{
    assert( current == tail );
    tail = current->GetPrev();
  }
  current = NULL;
}



double cEventList::GetTriggerValue(eTriggerVariable trigger){
  // Returns DBL_MIN if invalid
  double t_val = DBL_MIN;
  switch( trigger ){
    case UPDATE:
      t_val = cStats::GetUpdate();
      break;
    case GENERATION:
      t_val = cStats::GetAveGeneration();
      break;
    default:
      cString mesg;
      mesg.Set("Unknown Trigger: [%s]", current->GetTrigger());
      population->NotifyWarning(mesg);
  }
  return t_val;
}


void cEventList::Process(){
  double t_val = 0; // trigger value

  // Iterate through all events in event list
  current = head;
  while( current != NULL ){

    cEvent * next_event = current->GetNext();

    // Get the value of the appropriate tigger varioable
    t_val = GetTriggerValue(current->GetTrigger());

    // Check trigger condition
    if( t_val != DBL_MIN &&
	( t_val >= current->GetStart() ||
	  current->GetStart() == TRIGGER_BEGIN ) &&
	( t_val <= current->GetStop() ||
	  current->GetStop() == TRIGGER_END ) ){

      ProcessCurrentEvent();

      // Handle the interval thing
      if( current->GetInterval() == TRIGGER_ALL ){
	// Do Nothing
      }else if( current->GetInterval() == TRIGGER_ONCE ){
	// If it is a onetime thing, remove it...
	Delete();
      }else{
	// There is an interal.. so add it
	current->NextInterval();
      }

      // If the event can never happen now... excize it
      if( current != NULL  &&  current->GetStop() != TRIGGER_END ){
	if( current->GetStart() > current->GetStop() &&
	    current->GetInterval() > 0 ){
	  Delete();
	}else if( current->GetStart() < current->GetStop() &&
		  current->GetInterval() < 0 ){
	  Delete();
	}
      }

    }  // end condition to do event

    current = next_event;

  }
}


void cEventList::ProcessCurrentEvent(){
  //******* ACTUAL EVENTS ***********//

      // *** Meta Events... Controlling the viewer or process ***
  if (current->GetName() == "echo") {
    if( current->GetArgs() != "" ){
      cString mesg;
      mesg.Set("%s", current->GetArgs()());
      population->NotifyComment(mesg);
    }else{
      cString mesg;
      mesg.Set("Echo (%f)", GetTriggerValue(current->GetTrigger()) );
      population->NotifyComment(mesg);
    }

  } else if (current->GetName() == "pause") {
    population->Pause();

  } else if (current->GetName() == "exit") {
    cStats::Shutdown();
    exit(0);  // @CAO should make this cleaner...
	

    // ** Printing Data Files ***
  } else if (current->GetName() == "print_average_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "average.dat";
    cStats::PrintAverageData(filename);

  } else if (current->GetName() == "print_error_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "error.dat";
    cStats::PrintErrorData(filename);

  } else if (current->GetName() == "print_variance_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "variance.dat";
    cStats::PrintVarianceData(filename);

  } else if (current->GetName() == "print_dominant_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "dominant.dat";
    cStats::PrintDominantData(filename);

  } else if (current->GetName() == "print_stats_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "stats.dat";
    cStats::PrintStatsData(filename);

  } else if (current->GetName() == "print_count_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "count.dat";
    cStats::PrintCountData(filename);

  } else if (current->GetName() == "print_totals_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "totals.dat";
    cStats::PrintTotalsData(filename);

  } else if (current->GetName() == "print_tasks_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "tasks.dat";
    cStats::PrintTasksData(filename);

  } else if (current->GetName() == "print_time_data") {
    cString filename = current->GetArgs();
    if( filename == "" )  filename = "time.dat";
    cStats::PrintTimeData(filename);


    // ** Load and Save ***
  } else if (current->GetName() == "save_clone") {
    cString filename;
    filename.Set("clone.%d", cStats::GetUpdate());
    ofstream fp(filename());
    population->SaveClone(fp);
	
  } else if (current->GetName() == "load_clone") {
    cString filename = current->GetArgs().GetWord(0);
    ifstream fp(filename());
    population->LoadClone(fp);


    // *** Inject ***
  } else if (current->GetName() == "inject") {
    cString filename = current->GetArgs().GetWord(0);
    int cpu_num = current->GetArgs().GetWord(1).AsInt();
    int merit = current->GetArgs().GetWord(2).AsInt();
    int label = current->GetArgs().GetWord(3).AsInt();
    population->Inject(filename(), cpu_num, merit, label);
	
  } else if (current->GetName() == "inject_all") {
    cString filename = current->GetArgs().GetWord(0);
    population->Inject(filename(), 0);
    cGenotype * new_genotype = population->GetCPU(0).GetActiveGenotype();
    for(int i = 1; i < population->GetSize(); i++) {
      population->GetCPU(i).ChangeGenotype(new_genotype);
      population->GetCPU(i).SetParentTrue(TRUE);
      population->GetCPU(i).SetCopiedSize(new_genotype->GetLength());
      population->GetCPU(i).GetPhenotype()->
	Clear(new_genotype->GetLength());
      population->GetCPU(i).SetInjected();
      population->AdjustTimeSlice(i);
    }
	
  } else if (current->GetName() == "inject_random") {
    int mem_size = current->GetArgs().GetWord(0).AsInt();
    population->InjectRandom(mem_size);


    // *** Modify Mutation Rates ***
  } else if (current->GetName() == "mod_copy_mut") {
    double new_cmut = cConfig::GetCopyMutProb() +
      current->GetArgs().GetWord(0).AsDouble();
    for (int i = 0; i < population->GetSize(); i++) {
      population->GetCPU(i).GetEnvironment()->SetCopyMutProb(new_cmut);
    }
    cConfig::SetCopyMutProb(new_cmut);
	
  } else if (current->GetName() == "set_copy_mut") {
    double new_cmut = current->GetArgs().GetWord(0).AsDouble();
    for (int i = 0; i < population->GetSize(); i++) {
      population->GetCPU(i).GetEnvironment()->SetCopyMutProb(new_cmut);
    }
    cConfig::SetCopyMutProb(new_cmut);
	
  } else if (current->GetName() == "set_point_mut") {
    double new_pmut = current->GetArgs().GetWord(0).AsDouble();
    for (int i = 0; i < population->GetSize(); i++) {
      population->GetCPU(i).GetEnvironment()->SetPointMutProb(new_pmut);
    }
    cConfig::SetPointMutProb(new_pmut);


    // *** HillClimbing ***
  } else if (current->GetName() == "hillclimb") {
    int search_depth = 1;
    int num_words = current->GetArgs().CountNumWords();
    if( num_words>=1 ) search_depth = current->GetArgs().GetWord(0).AsInt();
    cCodeArray & code = population->GetGenebank().
      GetBestGenotype()->GetCode();
    population->HillClimb( code );


    // *** Landscaping ***

    // Exhaustively calculate the fitness results of each mutant to a depth of
    // <landscape_dist>.
    // First argument: landscape_dist
    // Second argument: filename for recording fitness of each mutant.  If it
    //      is omitted, this file is not written.
  } else if (current->GetName() == "calc_landscape") {
    int landscape_dist = 1;
    cString filename("");
    int num_words = current->GetArgs().CountNumWords();
    if ( num_words >= 1 )
      landscape_dist = current->GetArgs().GetWord(0).AsInt();
    if ( num_words >= 2 )
      filename = current->GetArgs().GetWord(1);
    cGenotype * genotype = population->GetGenebank().
      GetBestGenotype();
    population->CalcLandscape(landscape_dist, *genotype, &filename);

  } else if (current->GetName() == "sample_landscape") {
    int sample_size = 0;
    int num_words = current->GetArgs().CountNumWords();
    if( num_words>=1 ) sample_size = current->GetArgs().GetWord(0).AsInt();
    cCodeArray & code = population->GetGenebank().
      GetBestGenotype()->GetCode();
    population->SampleLandscape(sample_size, code);

  } else if (current->GetName() == "random_landscape") {
    int landscape_dist = 1;
    int sample_size = 0;
    int min_found = 0;
    int max_sample_size = 0;
    int print_if_found = FALSE;
	
    int num_args = current->GetArgs().CountNumWords();
    if (num_args >= 1)
      landscape_dist  = current->GetArgs().GetWord(0).AsInt();
    if (num_args >= 2)
      sample_size     = current->GetArgs().GetWord(1).AsInt();
    if (num_args >= 3)
      min_found       = current->GetArgs().GetWord(2).AsInt();
    if (num_args >= 4)
      max_sample_size = current->GetArgs().GetWord(3).AsInt();
    if (num_args >= 5)
      print_if_found  = current->GetArgs().GetWord(4).AsInt();
    cCodeArray & code = population->GetGenebank().
      GetBestGenotype()->GetCode();
    population->RandomLandscape(landscape_dist, sample_size, min_found,
				max_sample_size, code, print_if_found);

  } else if (current->GetName() == "analyze_landscape") {
    int sample_size = 1000;
    int num_words = current->GetArgs().CountNumWords();
    if (num_words >= 1) sample_size=current->GetArgs().GetWord(0).AsInt();
    cCodeArray & code = population->GetGenebank().
      GetBestGenotype()->GetCode();
    population->AnalyzeLandscape(code, sample_size);

  } else if (current->GetName() == "pairtest_landscape") {
    int sample_size = 1000;
    int num_words = current->GetArgs().CountNumWords();
    if (num_words >= 1) sample_size=current->GetArgs().GetWord(0).AsInt();
    cCodeArray & code = population->GetGenebank().
      GetBestGenotype()->GetCode();
    population->PairTestLandscape(code, sample_size);

  } else if (current->GetName() == "pairtest_full_landscape") {
    cCodeArray & code = population->GetGenebank().
      GetBestGenotype()->GetCode();
    population->PairTestFullLandscape(code);

    // *** apocalypse ***
    //event added to list by mckell carter
  } else if (current->GetName() == "apocalypse") {
      double p_apoc = current->GetArgs().GetWord(0).AsDouble();
      population->Apocalypse(p_apoc);



    // *** Consensus ***
  } else if (current->GetName() == "calc_consensus") {
    int lines_saved = 0;
    if (!current->GetArgs().IsEmpty())
      lines_saved = current->GetArgs().GetWord(0).AsInt();
    population->CalcConsensus(lines_saved);


    // *** Threads ***
  } else if (current->GetName() == "test_threads") {
    cStats::TestThreads( population->GetGenebank().
			 GetBestGenotype()->GetCode() );


    // *** Grid Ouptut ***
  } else if (current->GetName() == "dump_basic_grid") {
    cString filename;
    filename.Set("grid.%05d.out", cStats::GetUpdate());
    ofstream fp(filename());
    for (int i = 0; i < population->GetWorldX(); i++) {
      for (int j = 0; j < population->GetWorldY(); j++) {
	char out_char = population->GetCPU(j * population->GetWorldX()
					   + i).GetBasicSymbol();
	switch (out_char) {
	  case ' ':
	    fp << -3;
	    break;
	  case '.':
	    fp << -2;
	    break;
	  case '+':
	    fp << -1;
	    break;
	  default:
	    fp << out_char - 'A';
	}
	fp << " ";
      }
      fp << endl;
    }
	
  } else if (current->GetName() == "dump_fitness_grid") {
    cString filename;
    filename.Set("fgrid.%05d.out", cStats::GetUpdate());
    ofstream fp(filename());
    for (int i = 0; i < population->GetWorldX(); i++) {
      for (int j = 0; j < population->GetWorldY(); j++) {
	cGenotype * genotype = population->
	  GetCPU(j * population->GetWorldX() + i).GetActiveGenotype();
	double fitness = (genotype != NULL) ? genotype->GetFitness() : 0.0;
	
	fp << fitness << " ";
      }
	
      fp << endl;
    }
	
	
    // *** Misc Output ***
  } else if (current->GetName() == "print_dom") {
    cString filename;
    cGenotype * dom = population->GetGenebank().GetBestGenotype();
    if (!current->GetArgs().IsEmpty()) {
      filename = current->GetArgs().GetWord(0);
    } else {
      filename.Set("genebank/%s", dom->GetName()());
    }
    cTestCPU::PrintCode(dom->GetCode(), filename);
	
#ifdef GENEOLOGY
  } else if (current->GetName() == "print_geneology_tree") {
    cString filename = current->GetArgs().GetWord(0);
    if( filename.IsEmpty() )
      filename.Set("geneology_tree_%d.out",cStats::GetUpdate());
    ofstream out_geneology_tree(filename());
    population->GetGeneology()->PrintTree(1,out_geneology_tree);

  } else if (current->GetName() == "print_geneology_depth") {
    population->GetGeneology()->PrintDepthHistogram();
#endif
  } else if (current->GetName() == "analyze_population" ) {
    cString filename;
    // default arguments
    filename.Set("population_info_%d.dat",cStats::GetUpdate());
    double sample_prob = 1;
    bool landscape = false;
    bool save_genotype = false;

    // overide default arguments
    int num_args = current->GetArgs().CountNumWords();
    if (num_args >= 1)
      sample_prob     = current->GetArgs().GetWord(0).AsDouble();
    if (num_args >= 2)
      landscape       = current->GetArgs().GetWord(1).AsInt();
    if (num_args >= 3)
      save_genotype   = current->GetArgs().GetWord(2).AsInt();
    if (num_args >= 4)
      filename        = current->GetArgs().GetWord(3);
    ofstream population_info(filename());
    population->AnalyzePopulation( population_info, sample_prob, landscape, save_genotype );


    // *** Error: Unknown Event ***
  } else {
    cString mesg;
    mesg.Set("Unknown Event: [%s]", current->GetName()());
    population->NotifyWarning(mesg);
  }

}

