//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2000 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef CREATURE_HH
#define CREATURE_HH

#include "../defs.hh"

class cCreature;

/**
 * The sCreatureInfo structure keeps track of all the generic information about
 * the status of a cpu.  Effectively a miscilaneous structure.
 **/

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
  int lineage_label; // an integer that can be used to tag lineages.
  // 			It is inherited unchanged from mother to daughter, so
  //			that it is clear which daughters stem from which 
  //                    mother.

  cCreature * next;
  cCreature * prev;

  void SaveState(ostream & fp){
    assert(fp.good());
    fp<< copied_size <<endl;
    fp<< executed_size <<endl;
    fp<< total_time_used <<endl;
    fp<< max_executed <<endl;
    fp<< gestation_start <<endl;
    fp<< gestation_time <<endl;
    fp<< num_divides <<endl;
    fp<< num_errors <<endl;
    fp<< fitness <<endl;
    fp<< age <<endl;
    fp<< neutral_metric <<endl;
    fp<< lineage_label <<endl;
  }

  void LoadState(istream & fp){
    assert(fp.good());
    fp>> copied_size;
    fp>> executed_size;
    fp>> total_time_used;
    fp>> max_executed;
    fp>> gestation_start;
    fp>> gestation_time;
    fp>> num_divides;
    fp>> num_errors;
    fp>> fitness;
    fp>> age;
    fp>> neutral_metric;
    fp>> lineage_label;
  }
};

/**
 * This is the base class for all creatures.  Currently it assumes that
 * creatures must have genotypes which are a linear series of instructions.
 **/

class cCreature {
protected:
  sCreatureInfo info;
public:
  cCreature() { ; }
  virtual ~cCreature() { ; }

  void   SetNeutralMetric(double _in){ info.neutral_metric=_in; };
  double GetNeutralMetric() const { return info.neutral_metric; };
  
  void SetLineageLabel( int label ) { info.lineage_label=label; };
  int GetLineageLabel() const { return info.lineage_label; };
};

#endif





