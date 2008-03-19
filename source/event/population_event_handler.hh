//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////
/* event_list.hh *************************************************************
 Builds a queue for holding and processing events  

 charles@krl.caltech.edu & travc@ugcs.caltech.edu
 Time-stamp: <1999-12-02 14:26:21 travc>

******************************************************************************/
  
#ifndef POPULATION_EVENT_LIST_HH
#define POPULATION_EVENT_LIST_HH

#include "../defs.hh"

#include "event_handler.hh"
#include "event_base.hh"
#include "cPopulationEvents_DEFS_auto.hh" 

class cPopulation;

class cPopulationEvent : public cEvent {
protected:
  cPopulation * population;
public:
  cPopulationEvent(const cString & name, const cString & args):
   cEvent(name, args) {;}
  void SetPopulation(cPopulation * in){ population = in; }
};


class cPopulationEventHandler : public cEventHandler {
private:
  cPopulation * population;
public:

  cPopulationEventHandler(cPopulation * _pop) : population(_pop) { ; }
  ~cPopulationEventHandler(){;}

  int EventNameToEnum(const cString & name) const;
  cEvent * ConstructEvent(int event_enum, const cString & args );
};


#endif // #ifndef POPULATION_EVENT_LIST_HH

