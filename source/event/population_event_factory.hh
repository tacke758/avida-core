#ifndef AVIDA_EVENT_FACTORY_HH
#define AVIDA_EVENT_FACTORY_HH

#ifndef DEFS_HH
#include "../defs.hh"
#endif

#ifndef EVENT_FACTORY_HH
#include "./event_factory.hh"
#endif

#ifndef EVENT_HH
#include "./event.hh"
#endif

class cPopulation;

class cPopulationEvent : public cEvent {
private:
  // not implemented. prevents inadvertend wrong instantiation.
  cPopulationEvent();
  cPopulationEvent( const cPopulationEvent & );
  const cPopulationEvent& operator=( cPopulationEvent & );

  
protected:
  cPopulation *population;
  
public:
  cPopulationEvent(const cString & name, const cString & args);

  void SetPopulation( cPopulation *pop ){ population = pop; }
};


class cPopulationEventFactory : public cEventFactory {
private:
  cPopulation *m_population;


public:
  // event enums
#include "cPopulation_enums_auto.ci"

  cPopulationEventFactory( cPopulation *pop );
  ~cPopulationEventFactory();

  int EventNameToEnum(const cString & name) const;
  cEvent * ConstructEvent(int event_enum, const cString & args );
};


#endif



