//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* event_manager.hh **********************************************************
 Passes events to appropriate event handlers (ex. event lists)
 Maintains primary event handlers

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-11-30 22:56:36 travc>

******************************************************************************/
  
#ifndef EVENT_MANAGER_HH
#define EVENT_MANAGER_HH

#include "../defs.hh"
#include "../tools/tVector.hh"
#include "../tools/string.hh"
#include "event_base.hh"


typedef int tHandlerID;

class cEventHandler;


class cEventManager {
private:
  tVector <cEventHandler *> handler_list;

public:
  cEventManager();
  ~cEventManager();

  tHandlerID AddHandler(cEventHandler * handler_ptr); 

  cEvent * ConstructEvent(const cString name, const cString & args, 
		      tHandlerID handler_id = -1);

  bool ProcessEvent(cEvent & event);
  bool ProcessEvent(int event_enum, const cString & args, 
		    tHandlerID handler_id = -1);
  bool ProcessEvent(const cString name, const cString & args, 
		    tHandlerID handler_id = -1);

};


#endif // #ifndef EVENT_MANAGER_HH

