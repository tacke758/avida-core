//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef REACTION_HH
#define REACTION_HH

#include <limits.h>

#include "../tools/string.hh"
#include "../tools/tArray.hh"
#include "../tools/tList.hh"

class cResource;
class cReaction;
class cTaskEntry;

#define REACTION_PROCTYPE_ADD  0
#define REACTION_PROCTYPE_MULT 1
#define REACTION_PROCTYPE_POW  2

class cReactionProcess {
private:
  cResource * resource;  // Resource consumed.
  double value;          // Efficiency.
  int type;              // Method of applying bonuses.
  double max_number;     // Max quantity of resource usable.
  double min_number;     // Minimum quantity of resource needed (otherwise 0)
  double max_fraction;   // Max fraction of avaiable resources useable.
  cResource * product;   // Output resource.
  double conversion;     // Conversion factor.
  bool lethal;			 // Lethality of reaction
  cResource * detect; // Resource Measured
  double detection_threshold;  //Minimum quantity of resource to register as present
  double detection_error;  //Var of Detection Event (as % of resource present)
public:
  cReactionProcess();
  ~cReactionProcess();

  cResource * GetResource() const { return resource; }
  double GetValue() const { return value; }
  int GetType() const { return type; }
  double GetMaxNumber() const { return max_number; }
  double GetMinNumber() const { return min_number; }
  double GetMaxFraction() const { return max_fraction; }
  cResource * GetProduct() const { return product; }
  double GetConversion() const { return conversion; }
  bool GetLethal() const { return lethal; }
  cResource * GetDetect() const { return detect; }
  double GetDetectionThreshold() const { return detection_threshold; }
  double GetDetectionError() const { return detection_error; }

  void SetResource(cResource * _in) { resource = _in; }
  void SetValue(double _in) { value = _in; }
  void SetType(int _in) { type = _in; }
  void SetMaxNumber(double _in) { max_number = _in; }
  void SetMinNumber(double _in) { min_number = _in; }
  void SetMaxFraction(double _in) { max_fraction = _in; }
  void SetProduct(cResource * _in) { product = _in; }
  void SetConversion(double _in) { conversion = _in; }
  void SetLethal(int _in) { lethal = _in; }
  void SetDetect(cResource * _in) { detect = _in; }
  void SetDetectionThreshold(double _in) { detection_threshold = _in; }
  void SetDetectionError(double _in) { detection_error = _in; }
};

class cReactionRequisite {
private:
  tList<cReaction> prior_reaction_list;
  tList<cReaction> prior_noreaction_list;
  int min_task_count;
  int max_task_count;
public:
  cReactionRequisite();
  ~cReactionRequisite();

  const tList<cReaction> & GetReactions() const { return prior_reaction_list; }
  const tList<cReaction> & GetNoReactions() const
    { return prior_noreaction_list; }
  int GetMinTaskCount() const { return min_task_count; }
  int GetMaxTaskCount() const { return max_task_count; }

  void AddReaction(cReaction * in_reaction) {
    prior_reaction_list.PushRear(in_reaction);
  }
  void AddNoReaction(cReaction * in_reaction) {
    prior_noreaction_list.PushRear(in_reaction);
  }
  void SetMinTaskCount(int min) { min_task_count = min; }
  void SetMaxTaskCount(int max) { max_task_count = max; }
};

class cReaction {
private:
  cString name;
  int id;
  cTaskEntry * task;
  tList<cReactionProcess> process_list;
  tList<cReactionRequisite> requisite_list;
  bool active;
public:
  cReaction(const cString & _name, int _id);
  ~cReaction();

  const cString & GetName() const { return name; }
  int GetID() const { return id; }
  cTaskEntry * GetTask() { return task; }
  const tList<cReactionProcess> & GetProcesses() { return process_list; }
  const tList<cReactionRequisite> & GetRequisites()
    { return requisite_list; }
  bool GetActive() const { return active; }

  void SetTask(cTaskEntry * _task) { task = _task; }
  cReactionProcess * AddProcess();
  cReactionRequisite * AddRequisite();
  void SetActive(bool in_active=true) { active = in_active; }
};

class cReactionLib {
private:
  tArray<cReaction *> reaction_array;
public:
  cReactionLib() { ; }
  ~cReactionLib();

  int GetSize() const { return reaction_array.GetSize(); }
  
  cReaction * AddReaction(const cString & name);
  cReaction * GetReaction(const cString & name) const;
  cReaction * GetReaction(int id) const;
};

#endif
