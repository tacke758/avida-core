//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef MUTATIONS_HH
#define MUTATIONS_HH

#include "../tools/tList.hh"
#include "../tools/tArray.hh"
#include "../tools/string.hh"

#define MUTATION_TRIGGER_NONE     0
#define MUTATION_TRIGGER_UPDATE   1
#define MUTATION_TRIGGER_DIVIDE   2
#define MUTATION_TRIGGER_PARENT   3
#define MUTATION_TRIGGER_WRITE    4
#define MUTATION_TRIGGER_READ     5
#define MUTATION_TRIGGER_EXEC     6
#define NUM_MUTATION_TRIGGERS     7

#define MUTATION_SCOPE_GENOME     0
#define MUTATION_SCOPE_LOCAL      1
#define MUTATION_SCOPE_GLOBAL     2
#define MUTATION_SCOPE_PROP       3
#define MUTATION_SCOPE_SPREAD     4

#define MUTATION_TYPE_POINT       0
#define MUTATION_TYPE_INSERT      1
#define MUTATION_TYPE_DELETE      2
#define MUTATION_TYPE_HEAD_INC    3
#define MUTATION_TYPE_HEAD_DEC    4
#define MUTATION_TYPE_TEMP        5
#define MUTATION_TYPE_KILL        6

class cMutation {
private:
  cString name;
  int id;
  int trigger;
  int scope;
  int type;
  double rate;
public:
  cMutation(const cString & _name, int _id, int _trigger, int _scope,
	    int _type, double _rate);
  ~cMutation();

  const cString & GetName() const { return name; }
  int GetID() const { return id; }
  int GetTrigger() const { return trigger; }
  int GetScope() const { return scope; }
  int GetType() const { return type; }
  double GetRate() const { return rate; }
};


class cMutationLib {
private:
  tArray<cMutation *> mutation_array;
  tArray< tList<cMutation> > trigger_list_array;
public:
  cMutationLib();
  ~cMutationLib();

  int GetSize() const { return mutation_array.GetSize(); }

  cMutation * AddMutation(const cString & name, int trigger, int scope,
			  int type, double rate);

  const tArray<cMutation *> & GetMutationArray() const
    { return mutation_array; }
  const tList<cMutation> & GetMutationList(int trigger) const
    { return trigger_list_array[trigger]; }
};


class cLocalMutations {
private:
  const cMutationLib & mut_lib;
  tArray<double> rates;
  tArray<int> counts;
public:
  cLocalMutations(const cMutationLib & _lib, int genome_length);
  ~cLocalMutations();

  const cMutationLib & GetMutationLib() const { return mut_lib; }
  double GetRate(int id) const { return rates[id]; }
  int GetCount(int id) const { return counts[id]; }

  void IncCount(int id) { counts[id]++; }
  void IncCount(int id, int num_mut) { counts[id] += num_mut; }
};


class cMutationRates {
private:
  // Mutations are divided up by when they occur...

  // ...anytime during execution...
  struct sExecMuts {
    double point_mut_prob;
    double exe_err_prob;
  };
  sExecMuts exec;

  // ...during an instruction copy...
  struct sCopyMuts {
    double copy_mut_prob;
  };
  sCopyMuts copy;

  // ...at the divide...
  struct sDivideMuts {
    double ins_mut_prob;        // Per site
    double del_mut_prob;        // Per site
    double div_mut_prob;        // Per site
    double divide_mut_prob;     // Max one per divide
    double divide_ins_prob;     // Max one per divide
    double divide_del_prob;     // Max one per divide
    double crossover_prob;
    double aligned_cross_prob;
    double parent_mut_prob;
  };
  sDivideMuts divide;
  
  void operator=(const cMutationRates & in_muts)
    { (void) in_muts; } // Disable operator=
public:
  cMutationRates();
  cMutationRates(const cMutationRates & in_muts);
  ~cMutationRates();

  void Clear();
  void Copy(const cMutationRates & in_muts);

  bool TestPointMut() const;
  bool TestExeErr() const;
  bool TestCopyMut() const;
  bool TestDivideMut() const;
  bool TestDivideIns() const;
  bool TestDivideDel() const;
  bool TestParentMut() const;
  bool TestCrossover() const;
  bool TestAlignedCrossover() const;

  double GetPointMutProb() const     { return exec.point_mut_prob; }
  double GetExeErrProb() const       { return exec.exe_err_prob; }
  double GetCopyMutProb() const      { return copy.copy_mut_prob; }
  double GetInsMutProb() const       { return divide.ins_mut_prob; }
  double GetDelMutProb() const       { return divide.del_mut_prob; }
  double GetDivMutProb() const       { return divide.div_mut_prob; }
  double GetDivideMutProb() const    { return divide.divide_mut_prob; }
  double GetDivideInsProb() const    { return divide.divide_ins_prob; }
  double GetDivideDelProb() const    { return divide.divide_del_prob; }
  double GetParentMutProb() const    { return divide.parent_mut_prob; }
  double GetCrossoverProb() const    { return divide.crossover_prob; }
  double GetAlignedCrossProb() const { return divide.aligned_cross_prob; }

  void SetPointMutProb(double in_prob)  { exec.point_mut_prob  = in_prob; }
  void SetExeErrProb(double in_prob)    { exec.exe_err_prob    = in_prob; }
  void SetCopyMutProb(double in_prob)   { copy.copy_mut_prob   = in_prob; }
  void SetInsMutProb(double in_prob)    { divide.ins_mut_prob    = in_prob; }
  void SetDelMutProb(double in_prob)    { divide.del_mut_prob    = in_prob; }
  void SetDivMutProb(double in_prob)    { divide.div_mut_prob    = in_prob; }
  void SetDivideMutProb(double in_prob) { divide.divide_mut_prob = in_prob; }
  void SetDivideInsProb(double in_prob) { divide.divide_ins_prob = in_prob; }
  void SetDivideDelProb(double in_prob) { divide.divide_del_prob = in_prob; }
  void SetParentMutProb(double in_prob) { divide.parent_mut_prob = in_prob; }
  void SetCrossoverProb(double in_prob) { divide.crossover_prob  = in_prob; }
  void SetAlignedCrossProb(double in)   { divide.aligned_cross_prob = in; }
};

#endif
