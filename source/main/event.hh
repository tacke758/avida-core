/* event.hh ******************************************************************
 Handles the event-list file, and eventually a generalized (scripted) interface

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1998-09-30 01:40:20 travc>

******************************************************************************/
  
#ifndef EVENT2_HH
#define EVENT2_HH


class cPopulation;

enum eTriggerVariable { UPDATE, GENERATION, UNDEFINED };

#define TRIGGER_BEGIN  DBL_MIN
#define TRIGGER_END    DBL_MAX
#define TRIGGER_ALL    0
#define TRIGGER_ONCE   DBL_MAX



class cEvent {

private:
  eTriggerVariable trigger;
  double start;
  double interval;
  double stop;

  cString name;
  cString arg_list;
  cEvent * prev;
  cEvent * next;

public:
  cEvent( eTriggerVariable _trigger, 
	  double _start, double _interval, double _stop,
	  cString & _name, const cString & _arg_list, 
	  cEvent * _prev = NULL, cEvent * _next = NULL ) :
   trigger(_trigger),
   start(_start),
   interval(_interval),
   stop(_stop),
   name(_name),
   arg_list(_arg_list),
   prev(_prev),
   next(_next)
    { ; }

  cEvent( const cEvent & in ){
    trigger = in.trigger;
    start = in.start;
    interval = in.interval;
    stop = in.stop;
    name = in.name;
    arg_list = in.arg_list;
    prev = NULL;
    next = NULL;
  }

  cEvent & operator=(const cEvent & in) {
    trigger = in.trigger;
    start = in.start;
    interval = in.interval;
    stop = in.stop;
    name = in.name;
    arg_list = in.arg_list;
    return *this;
  }

  eTriggerVariable GetTrigger()  const { return trigger; }
  double           GetStart()    const { return start; }
  double           GetInterval() const { return interval; }
  double           GetStop()     const { return stop; }
  const cString &  GetName()     const { return name; }
  const cString &  GetArgs()     const { return arg_list; }

  cEvent *         GetPrev()     const { return prev; }
  cEvent *         GetNext()     const { return next; }

  void SetPrev(cEvent * in_prev) { prev = in_prev; }
  void SetNext(cEvent * in_next) { next = in_next; }

  void NextInterval() { start += interval; }
};


class cEventList {
private:
  cEvent * head;
  cEvent * tail;
  cEvent * current;
  int num_events;

  cPopulation * population;

  void AddEvent( eTriggerVariable _trigger, 
		 double _start, double _interval, double _stop,
		 cString & _name, const cString & _arg_list );
  
  void Delete();  // Removes Currnet event from event list

  double GetTriggerValue(eTriggerVariable trigger);

public:
  cEventList(const cString & filename, cPopulation * _pop) : 
   head(NULL), tail(NULL), current(NULL), num_events(0), population(_pop) 
    { BuildEventList(filename); }

  ~cEventList();



  void BuildEventList(const cString & filename);


  void Process();              // Go through list
  void ProcessCurrentEvent();  // Actually take the action for an event

  /*
  const cEvent & Next(); 

  cEvent * GetEvent() { return event_list; }
  int GetNumEvents()  { return num_events; }
  */


};

#endif
