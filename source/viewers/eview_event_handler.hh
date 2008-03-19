//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* event_list.hh *************************************************************
 Builds a queue for holding and processing events  

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-12-02 14:26:41 travc>

******************************************************************************/
  
#ifndef EVIEW_EVENT_LIST_HH
#define EVIEW_EVENT_LIST_HH

#include "../defs.hh"
#include "../tools/string.hh"

#include "../event/event_base.hh"
#include "../event/event_handler.hh"
#include "cEViewEvents_DEFS_auto.hh" 

class cExternalView;
class cEventManager;


class cEViewEvent : public cEvent {
protected:
  cExternalView * viewer;
  cEventManager * event_manager;
public:
  cEViewEvent(const cString & name, const cString & args) :
   cEvent(name, args) {;}
  void SetViewer(cExternalView * _in){ viewer = _in; }
  void SetEventManager(cEventManager * _in){ event_manager = _in; }
};


class cEViewEventHandler : public cEventHandler {
protected:
  cExternalView * viewer;
  cEventManager * event_manager;
public:

  cEViewEventHandler(cExternalView * _viewer, 
		     cEventManager * _event_manager) : 
   viewer(_viewer), event_manager(_event_manager) {;}
  ~cEViewEventHandler() {;}

  int EventNameToEnum(const cString & name) const;
  cEvent * ConstructEvent(int event_enum, const cString & args );
};




#endif // #ifndef EVIEW_EVENT_LIST_HH

