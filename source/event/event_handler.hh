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
 Time-stamp: <1999-11-30 23:19:23 travc>

******************************************************************************/
  
#ifndef EVENT_HANDLER_HH
#define EVENT_HANDLER_HH

#include "../defs.hh"
#include "../tools/tVector.hh"
#include "../tools/string.hh"
#include "event_base.hh"

class cEventManager;


typedef int tHandlerID;

class cEventHandler {
private:
  tHandlerID handler_id;
  cEventManager * event_manager;

protected: 
  const cEventManager * GetEventManager(){ 
    assert(event_manager != NULL );
    return event_manager; }

public:
  cEventHandler() : handler_id(-1), event_manager(NULL) {;}
  virtual ~cEventHandler() {;}

  // @TCC -- SetHandlerID should be private and friend to cEventManager
  void SetHandlerID(tHandlerID id, cEventManager * manager){ 
    handler_id = id;  
    event_manager = manager; }

  tHandlerID GetHandlerID(){ return handler_id; }

  virtual int EventNameToEnum(const cString & name) const = 0;

  virtual bool ProcessEvent( cEvent & event );
  virtual bool ProcessEvent( int event_enum, const cString & args );
  virtual bool ProcessEvent( const cString & name, const cString & args );

  virtual cEvent * ConstructEvent( int event_enum, const cString & args )=0;
  cEvent * ConstructEvent( const cString & name, const cString & args ){
    return ConstructEvent(EventNameToEnum(name), args); }

};

#endif // #ifndef EVENT_HANDLER_HH

