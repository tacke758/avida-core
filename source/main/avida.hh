//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef AVIDA_HH
#define AVIDA_HH

#include <assert.h>

#ifndef DEFS_HH
#include "../defs.hh"
#endif

#ifndef STRING_HH
#include "../tools/string.hh"
#endif


#ifndef EVENT_LIST_HH
#include "../event/event_list.hh"
#endif

#ifndef STATS_HH
#include "./stats.hh"
#endif

// cAvidaMain is a static class to handle the main() level of an avida run
// Globals and the like should actually be part of this class.

class cEventFactoryManager;
class cEventList;
class cBaseView;
class cAvidaDriver_Base;

class cEnvironment;
class cGenebank;
class cGenotype;
class cOrganism;
class cPopulation;
class cPopulationInterface;


/**
 * This function properly shuts down the Avida program.
 **/
void ExitAvida(int exit_code);



/**
 * This function does all the command line parsing etc., sets everything
 * up, and returns a reference to the environment that should be used.
 *
 * Besides the command line parameters, this function needs two more
 * variables. One is the environment the Avida run should use (the
 * environment is read in from disk and initialized in SetupAvida), the
 * other is the test population interface (also initialized in SetupAvida).
 **/
void SetupAvida(int argc, char * argv[],
		cEnvironment &environment,
		cPopulationInterface &test_interface );


/**
 * A small class that determines the correct triggers for the event list.
 * Needed in order to separate the event business from the rest.
 */
class cAvidaTriggers : public cEventTriggers {
private:
  cStats & stats;
public:
  cAvidaTriggers(cStats & _stats) : stats(_stats) { ; }
  double GetUpdate() const { return (double) stats.GetUpdate(); }
  double GetGeneration() const { return stats.SumGeneration().Average(); }
};



/**
 * The Avida Driver Objects.
 **/

/**
 *  cAvidaDriver_Base
 *
 *  This class is the base class for driving an avida run (be it in analyze
 *  mode or as a population).  It includes all of the functions that should
 *  be overloaded to make any viewer work.
 **/

class cAvidaDriver_Base {
protected:
  bool done_flag;   // This is set to true when run should finish.

public:
  static cAvidaDriver_Base * main_driver;

  cAvidaDriver_Base();
  virtual ~cAvidaDriver_Base();

  virtual void Run() = 0;  // Called when Driver should take over execution.
  virtual void ExitProgram(int exit_code);  // Called on an interupt,

  // There functions are ideally called by avida whenever a message needs to
  // be sent to the user.
  virtual void NotifyComment(const cString & in_string);
  virtual void NotifyWarning(const cString & in_string);
  virtual void NotifyError(const cString & in_string);

  // Called when the driver should stop what its doing for the moment, to
  // let the user see what just happened.
  virtual void SignalBreakpoint() { ; }

  void SetDone() { done_flag = true; }
};

class cAvidaDriver_Analyze : public cAvidaDriver_Base {
protected:
public:
  cAvidaDriver_Analyze();
  virtual ~cAvidaDriver_Analyze();

  virtual void Run();
};

class cAvidaDriver_Population : public cAvidaDriver_Base {
protected:
  cPopulation * population;
  cEventFactoryManager * event_manager;
  cEventList * event_list;

  void GetEvents();
  /**
   * Processes one complete update.
   *
   * Returns true when finished.
   **/
  virtual bool ProcessUpdate();
  virtual void ProcessOrganisms();
  void ReadEventListFile(const cString & filename="event_list");
  void SyncEventList();
			
public:
  cAvidaDriver_Population(cEnvironment & environment);
  virtual ~cAvidaDriver_Population();

  virtual void Run();
  virtual void NotifyUpdate();
};

#endif



