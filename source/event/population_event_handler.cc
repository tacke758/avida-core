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
#include "../main/genebank.hh"

#include "event_base.hh"
#include "event_list.hh"


/////////////////
//  cPopulationEventList
/////////////////
#include "population_event_handler.hh"

// The Process() functions
#include "cPopulationEvents_auto.ci" 

// EventNameToEnum()
#include "cPopulationEvents_N2E_auto.ci" 

//******* ACTUAL EVENTS ***********//
cEvent * cPopulationEventHandler::ConstructEvent( int event_enum, 
						  const cString & arg_list ){
  cPopulationEvent * event = NULL;
  
  switch (event_enum){ 

#include "cPopulationEvents_MakeEvent_auto.ci"

    default:
      event = NULL;
      break;
  }

  if( event != NULL ){
    // Have to setup the base class variables
    assert(population!=NULL);
    event->SetHandlerID(GetHandlerID());
    event->SetPopulation(population);
  }
  return event;
}


