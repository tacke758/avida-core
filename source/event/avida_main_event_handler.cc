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

#include "event_base.hh"
#include "event_list.hh"


/////////////////
//  cAvidaMainEventList
/////////////////
#include "avida_main_event_handler.hh"

// The Process() functions
#include "cAvidaMainEvents_auto.ci" 

// EventNameToEnum() 
#include "cAvidaMainEvents_N2E_auto.ci" 

//******* ACTUAL EVENTS ***********//
cEvent * cAvidaMainEventHandler::ConstructEvent( int event_enum, 
						 const cString & arg_list ){
  cAvidaMainEvent * event = NULL;
  
  switch (event_enum){ 

#include "cAvidaMainEvents_MakeEvent_auto.ci"

    default:
      event = NULL;
      break;
  }

  if( event != NULL ){
    // Have to setup the base class variables
    assert(event_list!=NULL);
    event->SetHandlerID(GetHandlerID());
    event->SetEventList(event_list);
  }
  return event;
}


