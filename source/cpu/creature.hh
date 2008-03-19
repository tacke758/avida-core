//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          //
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CREATURE_HH
#define CREATURE_HH

#include "../defs.hh"

class cCreature;

// The sCreatureInfo structure keeps track of all the generic information about
// the status of a cpu.  Effectively a miscilaneous structure.

class sCreatureInfo {
public:
  cGenotype * active_genotype;

  int copied_size;
  int executed_size;

  int total_time_used;
  int max_executed;
  int gestation_start;
  int gestation_time;

  int num_divides;
  int num_errors;
  double fitness;
  int age;

  double neutral_metric;  // A variable that undergoes drift 
                          // (gausian 0,1) per generation
  int lineage_label; // A label to track lineages
  

  cCreature * next;
  cCreature * prev;
};

// This is the base class for all creatures.  Currently it assumes that
// creatures must have genotypes which are a linear series of instructions.

class cCreature {
protected:
  sCreatureInfo info;
public:
  cCreature() { ; }
  virtual ~cCreature() { ; }

  void   SetNeutralMetric(double _in){ info.neutral_metric=_in; };
  double GetNeutralMetric() const { return info.neutral_metric; };
  
  void   SetLineageLabel(int _in){ info.lineage_label=_in; };
  int    GetLineageLabel() const { return info.lineage_label; };
  
  
};

#endif
