//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include "../defs.hh"
#include "../tools/string.hh"

#include "../main/avida.hh"
#include "../main/stats.hh"
#include "../main/genebank.hh"
#include "../main/population.hh"

#ifdef GENEOLOGY
#include "../main/geneology.hh"
#endif

#include "event_base.hh"
#include "event_list.hh"
#include "events.hh"

//******* ACTUAL EVENTS ***********//
bool cEventList::AddEvent( eTriggerVariable trigger, 
			   double start, double interval, double stop,
			   const cString & name, const cString & arg_list ){

  cEvent * event = NULL;  // if it is NULL at end, it isn't added

  //// ********* ADD EVENTS HERE ********** ////
  if ( name == "no_event" ){ 
    event = NULL;

#include "../utils/events_name_auto.ci"


/*
    // *** Landscapeing ***
    //  } else if (name == "sample_landscape") {
    //    int sample_size = 0;
    //    int num_words = current->GetArgs().CountNumWords();
    //    if( num_words>=1 ) sample_size = current->GetArgs().GetWord(0).AsInt();
    //    cCodeArray & code = population->GetGenebank().
    //      GetBestGenotype()->GetCode();
    //    population->SampleLandscape(sample_size, code);

#ifdef GENEOLOGY
  } else if (name == "print_geneology_tree") {
    cString filename = current->GetArgs().GetWord(0);
    if( filename.IsEmpty() )
      filename.Set("geneology_tree_%d.out",cStats::GetUpdate());
    ofstream out_geneology_tree(filename());
    population->GetGeneology()->PrintTree(1,out_geneology_tree);

  } else if (name == "print_geneology_depth") {
    population->GetGeneology()->PrintDepthHistogram();
#endif
	
*/
    // *** Error: Unknown Event ***
  } else {
    event = NULL; // Making sure we don't add it
    cString mesg;
    mesg.Set("Unknown Event: [%s %s]", name(), arg_list());
    population->NotifyError(mesg);
  }
      
  ///// Adding Event to the list /////
  if( event != NULL ){
    num_events ++;
    // Have to setup the base class variables
    event->Setup(name, arg_list, population, this, 
		 trigger, start, interval, stop);
    
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
    SyncEvent(event);
  }
  return (event != NULL);
}


//////////////////////////
//  cEvent & subclasses
//////////////////////////

#include "../utils/events_process_auto.ci"


