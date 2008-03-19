//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <float.h>           // for DBL_MIN
#include <iostream.h>        
#include <ctype.h>           // for isdigit
#include "../defs.hh"
#include "../tools/file.hh"
#include "../tools/string.hh"
#include "../main/avida.hh"
#include "../main/stats.hh"
#include "../main/population.hh"

#ifdef GENEOLOGY
#include "../main/geneology.hh"
#endif

#include "event_base.hh"
#include "event_manager.hh"
#include "event_list.hh"


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


bool cEventList::AddEvent( eTriggerVariable trigger, 
			   double start, double interval, double stop,
			   const cString & name, const cString & arg_list ){
  assert(event_manager!=NULL);
  cEvent * event = event_manager->ConstructEvent(name, arg_list);  

  ///// Adding Event to the list /////
  if( event != NULL ){
    InsertEvent(event, trigger, start, interval, stop);
  }
  return (event != NULL);
}


void cEventList::InsertEvent(cEvent * event, eTriggerVariable trigger,
			     double start, double interval, double stop){
  assert( event != NULL );
  cEventListEvent * entry = new cEventListEvent(event, trigger, 
						start, interval, stop);
  // If there are no events in the list yet.
  if( tail == NULL ){
    assert( head == NULL );
    head = entry;
    tail = entry;
  }else{
    // Add to the end of the list 
    tail->SetNext(entry);
    entry->SetPrev(tail);
    tail = entry;
  }
  SyncEvent(entry);
  ++num_events;
}


void cEventList::Delete(cEventListEvent * event){
  assert( event != NULL );

  if( event->GetPrev() != NULL ){
    event->GetPrev()->SetNext(event->GetNext());
  }else{
    //assert( event == head ); // @TCC - This breaks if load clone or load 
    // population is first thing in event list... why?
    head = event->GetNext();
  }
  if( event->GetNext() != NULL ){
    event->GetNext()->SetPrev(event->GetPrev());
  }else{
    assert( event == tail );
    tail = event->GetPrev();
  }
  event = NULL;
}



double cEventList::GetTriggerValue(eTriggerVariable trigger){
  // Returns DBL_MAX if invalid, DBL_MIN for IMMEDIATE
  double t_val = DBL_MAX;
  switch( trigger ){
    case IMMEDIATE: 
      t_val = DBL_MIN;
      break;
    case UPDATE: 
      t_val = cStats::GetUpdate();
      break;
    case GENERATION: 
      t_val = cStats::GetAveGeneration();
      break;
    default: 
      cString mesg;
      mesg.Set("Unknown Trigger: [%s]", current->GetTrigger());
      cAvidaMain::NotifyWarning(mesg);
  }
  return t_val;
}



void cEventList::Process(){
  double t_val = 0; // trigger value
  
  // Iterate through all events in event list
  cEventListEvent * event = head;
  while( event != NULL ){ 

    cEventListEvent * next_event = event->GetNext();
    assert(event != NULL );

    // Check trigger condition

    // IMMEDIATE Events always happen and are always deleted
    if( event->GetTrigger() == IMMEDIATE  ){ 
      //cerr<<"IMMEDIATE EVENT "<<event->GetName()<<endl;
      event_manager->ProcessEvent(event->GetEvent());
      Delete(event);  
    }else{
      
      // Get the value of the appropriate tigger variable
      t_val = GetTriggerValue(event->GetTrigger());
      
      if( //!cAvidaMain::IsPaused() && // Paused skips all Non-IMMEDIATE events
	  t_val != DBL_MAX && 
	  ( t_val >= event->GetStart() || 
	    event->GetStart() == TRIGGER_BEGIN ) &&
	  ( t_val <= event->GetStop() || 
	    event->GetStop() == TRIGGER_END ) ){
	
	//cerr<<"EVENT "<<event->GetName()<<endl;
	event_manager->ProcessEvent(event->GetEvent());
	
	if( event == NULL ){ // It is possible for an event to kill itself
	  //cerr<<"EVENT KILLED ITSELF!!!"<<endl;
	}else{
	  
	  // Handle the interval thing
	  if( event->GetInterval() == TRIGGER_ALL ){
	    // Do Nothing
	  }else if( event->GetInterval() == TRIGGER_ONCE ){
	    // If it is a onetime thing, remove it...
	    Delete(event);
	  }else{
	    // There is an interal.. so add it
	    event->NextInterval();
	  }
      
	  // If the event can never happen now... excize it
	  if( event != NULL  &&  event->GetStop() != TRIGGER_END ){
	    if( event->GetStart() > event->GetStop() && 
		event->GetInterval() > 0 ){
	      Delete(event);
	    }else if( event->GetStart() < event->GetStop() && 
		      event->GetInterval() < 0 ){
	      Delete(event);
	    }
	  }
	  
	}
	
      } // End Non-IMMEDITAE events
      
    }  // end condition to do event
    
    event = next_event;
  }
}


void cEventList::Sync(){
  cEventListEvent * event = head;
  cEventListEvent * next_event;
  while( event != NULL ){ 
    next_event = event->GetNext();
    assert( event != NULL);
    SyncEvent(event);
    event = next_event;
  }
}


void cEventList::SyncEvent(cEventListEvent * event){
  // Ignore events that are immdeiate
  if( event->GetTrigger() == IMMEDIATE ){ return; }

  double t_val = GetTriggerValue(event->GetTrigger());

  // If t_val has past the end, remove (even if it is TRIGGER_ALL)
  if( t_val > event->GetStop() ){
    Delete(event);
    return;
  }
  
  // If it is a trigger once and has passed, remove
  if( t_val > event->GetStart() && event->GetInterval() == TRIGGER_ONCE ){
    Delete(event);
    return;
  }

  // If for some reason t_val has been reset or soemthing, rewind
  if( t_val + event->GetInterval() <= event->GetStart() ){
    event->Reset();
  }

  // Can't fast forward events that are Triger All
  if( event->GetInterval() == TRIGGER_ALL ){ return; }

  // Keep adding interval to start until we are caught up
  while( t_val > event->GetStart() ){
    event->NextInterval();
  }
}


void cEventList::PrintEventList(ostream & os){
  cEventListEvent * event = head;
  cEventListEvent * next_event;
  while( event != NULL ){ 
    next_event = event->GetNext();
    PrintEvent(event,os);
    event = next_event;
  }
} 

void cEventList::PrintEvent(cEventListEvent * event, ostream & os){
    assert( event != NULL);
    if( event->GetTrigger() == UPDATE ){
      os<<"UPDATE ";
    }else if( event->GetTrigger() == GENERATION ){
      os<<"GENERATION ";
    }else if( event->GetTrigger() == IMMEDIATE ){
      os<<"IMMEDIATE ";
    }else{
      os<<"UNDEFINED ";
    }
    os<<"[";
    if( event->GetStart() == TRIGGER_BEGIN ){
      os<<"begin";
    }else{
      os<<event->GetStart();
    }
    os<<":";
    if( event->GetInterval() == TRIGGER_ONCE ){
      os<<"once";
    }else if( event->GetInterval() == TRIGGER_ALL ){
      os<<"all";
    }else{
      os<<event->GetInterval();
    }
    os<<":";
    if( event->GetStop() == TRIGGER_END ){
      os<<"end";
    }else{
      os<<event->GetStop();
    }
    os<<"] "<<event->GetName()<<" "<<event->GetArgs()<<endl;
}


//// Parsing Event List File Format ////
bool cEventList::AddEventFileFormat(const cString & in_line){
  cString cur_line = in_line;

  // Timing
  eTriggerVariable trigger = UPDATE;
  double start = TRIGGER_BEGIN;
  double interval = TRIGGER_ONCE;
  double stop = TRIGGER_END;

  cString name;
  cString arg_list;

  cString tmp;

  cString cur_word = cur_line.PopWord();
  
  // Get the trigger variable if there
  if( cur_word == "i"  ||
      cur_word == "immediate" ){
    trigger = IMMEDIATE;
    name = cur_line.PopWord();
    return AddEvent(name, cur_line); // If event is IMMEDIATE shortcut
  }else if( cur_word == "u"  ||
	    cur_word == "update" ){
    trigger = UPDATE;
    cur_word = cur_line.PopWord();
  }else if( cur_word == "g"  ||
	    cur_word == "generation" ){
    trigger = GENERATION;
    cur_word = cur_line.PopWord();
  }else{
    // If Trigger is skipped so assume IMMEDIATE 
    trigger = UPDATE;
  }
    
  // Do we now have timing specified?
  // Parse the Timing
  cString timing_str = cur_word;

  // Get the start:interval:stop
  tmp = timing_str.Pop(':');
  
  // If first value is valid, we are getting a timing.
  if( tmp.IsNumber() || tmp == "begin" ){

    // First number is start
    if( tmp == "begin" ){ 
      start = TRIGGER_BEGIN;
    }else{
      start = tmp.AsDouble();
    }

    // If no other words... is "start" syntax
    if( timing_str.GetSize() == 0 ){
      interval = TRIGGER_ONCE;
      stop     = TRIGGER_END;
    }else{
      // Second word is interval
      tmp = timing_str.Pop(':');
      if( tmp == "all" ){ 
	  interval = TRIGGER_ALL;
      }else if( tmp == "once" ){ 
	interval = TRIGGER_ONCE;
      }else{
	interval = tmp.AsDouble();
      }
      // If no other words... is "start:interval" syntax
      if( timing_str.GetSize() == 0 ){
	stop     = TRIGGER_END;
      }else{
	// We have "start:interval:stop" syntax
	tmp = timing_str;
	if( tmp == "end" ){ 
	  stop = TRIGGER_END;
	}else{
	  stop = tmp.AsDouble();
	}
      }
    } 
    cur_word = cur_line.PopWord(); // timing provided, so get next word

  }else{ // We don't have timing, so assume IMMEDIATE
    trigger = IMMEDIATE;
    start = TRIGGER_BEGIN;
    interval = TRIGGER_ONCE;
    stop = TRIGGER_END;
  }
    
  // Get the rest of the info
  name = cur_word;
  arg_list = cur_line;
  
  return AddEvent( trigger, start, interval, stop, name, arg_list );
}
  

