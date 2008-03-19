//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* event_base.hh *************************************************************
 The base class from which all events are derived.
 Definitions of event and event_list constants and enum types as well

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-11-30 15:09:39 travc>

******************************************************************************/
  
#ifndef EVENT_BASE_HH
#define EVENT_BASE_HH

#include "../defs.hh"
#include <values.h>
//#include "event_defs.hh"

enum eTriggerVariable { UPDATE, GENERATION, IMMEDIATE, UNDEFINED };

#define TRIGGER_BEGIN  DBL_MIN
#define TRIGGER_END    DBL_MAX
#define TRIGGER_ALL    0
#define TRIGGER_ONCE   DBL_MAX



class cEvent {
protected:
  cString name;
  cString args;
private:
  int handler_id;
public:
  cEvent( const cString & _name = "Undefined", 
	  const cString & _args = "",
	  int _handler_id = -1 ) :
   name(_name),
   args(_args),
   handler_id(_handler_id) { ; }

  virtual ~cEvent(){;}

  const cString &  GetName()       const { return name; }
  const cString &  GetArgs()       const { return args; }
  int              GetHandlerID()  const { return handler_id; } 

  void SetHandlerID(int _handler_id){ handler_id = _handler_id; } 

  virtual void Process() = 0;
};


#endif // #ifndef EVENT_BASE_HH

