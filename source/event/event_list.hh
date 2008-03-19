/* event_list.hh *************************************************************
 Builds a queue for holding and processing events

 charles@krl.caltech.edu & travc@ugcs.caltech.edu

******************************************************************************/

#ifndef EVENT_LIST_HH
#define EVENT_LIST_HH

#ifndef DEFS_HH
#include "../defs.hh"
#endif

#ifndef EVENT_HH
#include "./event.hh"
#endif

class cEventFactoryManager;


/**
 * This class is used to physically separate the whole event list business
 * from the world an so on. You have to derive from this object and fill
 * in the purely virtual functions with the appropriate contents.
 **/

class cEventTriggers {
public:
  enum eTriggerVariable { UPDATE, GENERATION, IMMEDIATE, UNDEFINED };

  static const double TRIGGER_BEGIN;
  static const double TRIGGER_END;
  static const double TRIGGER_ALL;
  static const double TRIGGER_ONCE;

private:
  // not implemented. prevents inadvertend wrong instantiation.
  cEventTriggers( const cEventTriggers& );
  cEventTriggers& operator=( const cEventTriggers& );
public:
  cEventTriggers() {}
  
  virtual double GetUpdate() const = 0;
  virtual double GetGeneration() const = 0;

  double GetTriggerValue( eTriggerVariable trigger ) const;
};


/**
 * This class holds a single event in the event list. It is used by the class
 * @ref cEventList. There is no need to use it in any other context.
 **/

class cEventListEntry {
private:
  cEvent *m_event;

  cEventTriggers::eTriggerVariable m_trigger;
  double m_start;
  double m_interval;
  double m_stop;
  double m_original_start;

  cEventListEntry *m_prev;
  cEventListEntry *m_next;


   // not implemented, prevents inadvertent wrong instantiation
  cEventListEntry();
  cEventListEntry( const cEventListEntry & );
  cEventListEntry& operator=( const cEventListEntry& );
public:
  // creators
  cEventListEntry( cEvent *event = NULL,
		   cEventTriggers::eTriggerVariable trigger
		                   = cEventTriggers::UPDATE,
		   double start = cEventTriggers::TRIGGER_BEGIN,
		   double interval = cEventTriggers::TRIGGER_ONCE,
		   double stop = cEventTriggers::TRIGGER_END,
		   cEventListEntry *prev = NULL,
		   cEventListEntry *next = NULL );

  virtual ~cEventListEntry();


  // manipulators
  void Setup( cEvent *event,
	      cEventTriggers::eTriggerVariable trigger,
	      double start = cEventTriggers::TRIGGER_BEGIN,
	      double interval = cEventTriggers::TRIGGER_ONCE,
	      double stop = cEventTriggers::TRIGGER_END ){
    m_event = event;
    m_trigger = trigger;
    m_start = start;
    m_original_start = start;
    m_interval = interval;
    m_stop = stop;
  }

  void SetPrev(cEventListEntry * prev) { m_prev = prev; }
  void SetNext(cEventListEntry * next) { m_next = next; }

  void NextInterval() { m_start += m_interval; }
  void Reset() { m_start = m_original_start; }


  // accessors
  cEvent* GetEvent() const {
    assert(m_event!=NULL); return m_event; }

  const cString& GetName() const {
    assert(m_event!=NULL); return m_event->GetName(); }
  const cString& GetArgs() const {
    assert(m_event!=NULL); return m_event->GetArgs(); }

  cEventTriggers::eTriggerVariable  GetTrigger()  const { return m_trigger; }
  double            GetStart()    const { return m_start; }
  double            GetInterval() const { return m_interval; }
  double            GetStop()     const { return m_stop; }

  cEventListEntry*  GetPrev()     const { return m_prev; }
  cEventListEntry*  GetNext()     const { return m_next; }

};



/**
 * sort of an iterator for cEventList;
 * doesn't allow access to its cEventListEntry, but will allow its entry
 * to be printed to an ostream.
 * added by Kaben.
 **/

class cEventListIterator {
private:
  cEventListEntry *m_node;
public:
  cEventListIterator() : m_node(0) {}
  cEventListIterator(cEventListEntry *node) : m_node(node) {}
  cEventListIterator(const cEventListIterator &it) : m_node(it.m_node) {}

  void PrintEvent(std::ostream & os = std::cout);

  bool operator==(const cEventListIterator &it) const {
    return m_node == it.m_node;
  }
  bool operator!=(const cEventListIterator &it) const {
    return m_node != it.m_node;
  }
  cEventListIterator& operator++() {
    m_node = m_node->GetNext();
    return *this;
  }
};


/**
 * This is the fundamental class for event management. It holds a list of all
 * events, and provides methods to add new events and to process existing
 * events.
 **/

class cEventList {

/*
additions by kaben to support cEventListIterator.
*/
friend class cEventListIterator;
public:
  cEventListIterator begin() { return cEventListIterator(m_head); }
  const cEventListIterator end() const { return cEventListIterator(0); }
/**/

protected:
  cEventFactoryManager *m_factory_manager;
  cEventTriggers *m_triggers;

  cEventListEntry *m_head;
  cEventListEntry *m_tail;
  cEventListEntry *m_current;
  int m_num_events;

  void InsertEvent(cEvent *event, cEventTriggers::eTriggerVariable trigger,
		   double start, double interval, double stop);
  void Delete( cEventListEntry *entry );
  void Delete(){ Delete( m_current); }

  double GetTriggerValue( cEventTriggers::eTriggerVariable trigger);

private:
  // not implemented, prevents inadvertent wrong instantiation
  cEventList();
  cEventList( const cEventList& );
  cEventList& operator=( const cEventList& );

public:
  // creators
  /**
   * The cEventList assumes ownership of both objects it is handed, and
   * destroys them when it is done.
   *
   * @param factory_manager A pointer to an event factory manager,
   * @ref cEventFactoryManager. The event factory manager is responsible
   * for finding the appropriate event factory to construct a given event.
   *
   * @param triggers A trigger object. The event list needs a trigger object
   * to determine what events to call when.
   **/
  cEventList( cEventFactoryManager *factory_manager, cEventTriggers *triggers );
  ~cEventList();

  // manipulators
  /**
   * Adds an event with given name and argument list. The event will be of
   * type immediate, i.e. it is processed only once, and then deleted.
   *
   * @param name The name of the event.
   * @param args The argument list.
   **/
  bool AddEvent( const cString & name, const cString & args ){
    return AddEvent( cEventTriggers::IMMEDIATE, cEventTriggers::TRIGGER_BEGIN, cEventTriggers::TRIGGER_ONCE, cEventTriggers::TRIGGER_END, name, args); }


  /**
   * Adds an event with specified trigger type.
   *
   * @param trigger The type of the trigger.
   * @param start The start value of the trigger variable.
   * @param interval The length of the interval between one processing
   * and the next.
   * @param stop The value of the trigger variable at which the event should
   * be deleted.
   * @param name The name of the even.
   * @param args The argument list.
   **/
  bool AddEvent( cEventTriggers::eTriggerVariable trigger,
		 double start, double interval, double stop,
		 const cString &name, const cString &args );

  /**
   * This function adds an event that is given in the event list file format.
   * In other words, it can be used to parse one line from an event list file,
   * and construct the appropriate event.
   **/
  bool AddEventFileFormat(const cString & line);

  /**
   * Delete all events in list; leave list ready for insertion of new
   * events.
   *
   * added by kaben.
   **/
  void DeleteAll(void);

  /**
   * Go through list executing appropriate events.
   **/
  void Process();

  /**
   * Get all events caught up.
   **/
  void Sync();

  /**
   * Get this event caught up.
   **/
  void SyncEvent(cEventListEntry *event);

  void PrintEventList(std::ostream & os = std::cout);

  // kaben changed this member function to static for easy access by
  // cEventListIterator.
  static void PrintEvent(cEventListEntry * event, std::ostream & os = std::cout);
};


#endif // #ifndef EVENT_LIST_HH



