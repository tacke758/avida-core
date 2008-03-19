//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <iostream.h>        
#include "../defs.hh"
#include "../tools/file.hh"
#include "../tools/string.hh"
#include "../main/avida.hh"
#include "../main/stats.hh"

#include "event_base.hh"
#include "event_manager.hh"
#include "event_handler.hh"


/////////////////
//  cEventManager
/////////////////

cEventManager::cEventManager() : handler_list(10) {; }

cEventManager::~cEventManager(){ 
  for( int i=0; i<handler_list.Size(); ++i ){
    if( handler_list[i] != NULL ){
      delete handler_list[i];
      handler_list[i] = NULL;
   }
  }
}


cEvent * cEventManager::ConstructEvent(const cString name, 
				       const cString & args, 
				       tHandlerID handler_id){
  cEvent * event = NULL;
  // handler_id <0 => send to all handlers
  if( handler_id < 0 ){
    for( int i=0; i<handler_list.Size() && event==NULL; ++i ){
      if( handler_list[i] != NULL ){
	event = handler_list[i]->ConstructEvent(name,args);
      }
    }
  }else{
    // send to particular handler
    if( handler_list[handler_id] != NULL ){
      event = handler_list[handler_id]->ConstructEvent(name,args);
    }
  }
  return event;
}


bool cEventManager::ProcessEvent(cEvent & event){
  bool rv = false;
  // handler_id <0 => send to all handlers
  if( event.GetHandlerID() < 0 ){
    for( int i=0; i<handler_list.Size(); ++i ){
      if( handler_list[i] != NULL ){
	if( handler_list[i]->ProcessEvent(event) ){
	  rv = true;
	}
      }
    }
  }else{
    // send to particular handler
    if( handler_list[event.GetHandlerID()] != NULL ){
      rv = handler_list[event.GetHandlerID()]->ProcessEvent(event);
    }
  }
  return rv;
}


bool cEventManager::ProcessEvent(int event_enum, const cString & args, 
				 tHandlerID handler_id){
  bool rv = false;
  // handler_id <0 => send to all handlers
  if( handler_id < 0 ){
    for( int i=0; i<handler_list.Size(); ++i ){
      if( handler_list[i] != NULL ){
	if( handler_list[i]->ProcessEvent(event_enum, args) ){
	  rv = true;
	}
      }
    }
  }else{
    // send to particular handler
    if( handler_list[handler_id] != NULL ){
      rv = handler_list[handler_id]->ProcessEvent(event_enum, args);
    }
  }
  return rv;
}


bool cEventManager::ProcessEvent(const cString name, const cString & args, 
				 tHandlerID handler_id){
  bool rv = false;
  // handler_id <0 => send to all handlers
  if( handler_id < 0 ){
    for( int i=0; i<handler_list.Size(); ++i ){
      if( handler_list[i] != NULL ){
	if( handler_list[i]->ProcessEvent(name, args) ){
	  rv = true;
	}
      }
    }
  }else{
    // send to particular handler
    if( handler_list[handler_id] != NULL ){
      rv = handler_list[handler_id]->ProcessEvent(name, args);
    }
  }
  return rv;
}


tHandlerID cEventManager::AddHandler(cEventHandler * handler_ptr){ 
  assert( handler_ptr != NULL );
  handler_list.Add(handler_ptr); 
  handler_ptr->SetHandlerID(handler_list.Size()-1, this);
  return handler_list.Size()-1; 
}



