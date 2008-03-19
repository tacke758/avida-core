//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
#include <iostream.h>        
#include "../defs.hh"
#include "../tools/ipc.hh"
#include "../tools/string.hh"
#include "../main/avida.hh"
#include "../main/stats.hh"

#include "externalview.hh"

#include "../event/event_list.hh"
#include "../event/event_list.hh"


/////////////////
//  cEViewEventList
/////////////////
#include "eview_event_handler.hh"

// The Process() functions
#include "cEViewEvents_auto.ci" 

// EventNameToEnum() 
#include "cEViewEvents_N2E_auto.ci" 

//******* ACTUAL EVENTS ***********//
cEvent * cEViewEventHandler::ConstructEvent( int event_enum, 
					     const cString & arg_list ){
  cEViewEvent * event = NULL;
  
  switch (event_enum){ 

#include "cEViewEvents_MakeEvent_auto.ci"

    default:
      event = NULL;
      break;
  }

  if( event != NULL ){
    // Have to setup the base class variables
    event->SetHandlerID(GetHandlerID());
    event->SetViewer(viewer);
    event->SetEventManager(event_manager);
  }
  return event;
}


