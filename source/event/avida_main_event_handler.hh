//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* event_list.hh *************************************************************
 Builds a queue for holding and processing events  

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-12-02 14:26:01 travc>

******************************************************************************/
  
#ifndef AVIDA_MAIN_EVENT_LIST_HH
#define AVIDA_MAIN_EVENT_LIST_HH

#include "../defs.hh"
#include "../tools/string.hh"

#include "event_base.hh"
#include "event_handler.hh"
#include "cAvidaMainEvents_DEFS_auto.hh" 

class cEventList;

class cAvidaMainEvent : public cEvent {
protected:
  cEventList * event_list;
public:
  cAvidaMainEvent(const cString & name, const cString & args) :
   cEvent(name, args) {;}
  void SetEventList(cEventList * in){ event_list = in; }
};


class cAvidaMainEventHandler : public cEventHandler {
protected:
  cEventList * event_list;
public:

  cAvidaMainEventHandler(cEventList * _in) : event_list(_in) {;}
  ~cAvidaMainEventHandler() {;}

  int EventNameToEnum(const cString & name) const;
  cEvent * ConstructEvent(int event_enum, const cString & args );
};




#endif // #ifndef AVIDA_MAIN_EVENT_LIST_HH

