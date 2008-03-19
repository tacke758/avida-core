//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* event_list.hh *************************************************************
 Builds a queue for holding and processing events  

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-11-29 21:44:24 travc>

******************************************************************************/
  
#ifndef EVENT_LIST_HH
#define EVENT_LIST_HH

#include "../defs.hh"
#include "event_base.hh"
#include "event_manager.hh"

class cEventListEvent {
private:
  cEvent * event;
  
  eTriggerVariable trigger;
  double start;
  double interval;
  double stop;
  double original_start;
  
  cEventListEvent * prev;
  cEventListEvent * next;

public:
  cEventListEvent( cEvent * _event = NULL,
		   eTriggerVariable _trigger =UPDATE, 
		   double _start =TRIGGER_BEGIN, 
		   double _interval =TRIGGER_ONCE, 
		   double _stop =TRIGGER_END,
		   cEventListEvent * _prev = NULL, 
		   cEventListEvent * _next = NULL ) :
   event(_event),
   trigger(_trigger),
   start(_start),
   interval(_interval),
   stop(_stop),
   original_start(_start),
   prev(_prev),
   next(_next)
    { ; }

  virtual ~cEventListEvent(){ delete event; }

  cEvent &          GetEvent()          { assert(event!=NULL); return *event; }

  const cString & GetName() const { assert(event!=NULL); return event->GetName(); }
  const cString & GetArgs() const { assert(event!=NULL); return event->GetArgs(); }

  eTriggerVariable  GetTrigger()  const { return trigger; }
  double            GetStart()    const { return start; }
  double            GetInterval() const { return interval; }
  double            GetStop()     const { return stop; }

  cEventListEvent * GetPrev()     const { return prev; }
  cEventListEvent * GetNext()     const { return next; }

  void Setup( cEvent * _event,
	      eTriggerVariable _trigger, 
	      double _start =TRIGGER_BEGIN, 
	      double _interval =TRIGGER_ONCE, 
	      double _stop =TRIGGER_END ){
    event = _event;
    trigger = _trigger;
    start = _start;
    original_start = _start;
    interval = _interval;
    stop = _stop;
  }

  void SetPrev(cEventListEvent * in_prev) { prev = in_prev; }
  void SetNext(cEventListEvent * in_next) { next = in_next; }

  void NextInterval() { start += interval; }
  void Reset() { start = original_start; }

};


class cEventList {
protected:
  cEventManager * event_manager;

  cEventListEvent * head;
  cEventListEvent * tail;
  cEventListEvent * current;
  int num_events;

  void InsertEvent(cEvent * event, eTriggerVariable trigger,
		   double start, double interval, double stop);
  void Delete(cEventListEvent * event);
  void Delete(){ Delete(current); }

  double GetTriggerValue(eTriggerVariable trigger);

public:
  cEventList(cEventManager * _event_manager) : event_manager(_event_manager), 
   head(NULL), tail(NULL), current(NULL), num_events(0) { ; }

  ~cEventList();

  bool AddEvent( const cString & _name, const cString & _arg ){
    return AddEvent(IMMEDIATE, TRIGGER_BEGIN, TRIGGER_ONCE, TRIGGER_END, 
		    _name, _arg); }
    
  bool AddEvent( eTriggerVariable _trigger, 
		 double _start, double _interval, double _stop,
		 const cString & _name, const cString & _arg_list );

  bool AddEventFileFormat(const cString & in_line);

  void Process();              // Go through list exectuting appropriate events

  void Sync();                   // Get all events caught up
  void SyncEvent(cEventListEvent * event);// Get this event caught up

  void PrintEventList(ostream & os = cout);
  void PrintEvent(cEventListEvent * event, ostream & os = cout);
};


#endif // #ifndef EVENT_LIST_HH

