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
#include "event_handler.hh"


//////// cEventHandler ////////////

bool cEventHandler::ProcessEvent(cEvent & event){
  event.Process();
  return true;
}


bool cEventHandler::ProcessEvent(int event_enum, const cString & args){
  cerr<<"cEventHandler::ProcessEvent by enum";
  bool rv = false;
  cEvent * event = ConstructEvent(event_enum, args);
  if( event != NULL ){
    rv = true;
    event->Process();
    delete event;
    cerr<<" worked!";
  }
  cerr<<endl;
  return rv;
}


bool cEventHandler::ProcessEvent(const cString & name, const cString & args){
  cerr<<"cEventHandler::ProcessEvent by name";
  bool rv = false;
  cEvent * event = ConstructEvent(name, args);
  if( event != NULL ){
    rv = true;
    event->Process();
    delete event;
    cerr<<" worked!";
  }
  cerr<<endl;
  return rv;
}



