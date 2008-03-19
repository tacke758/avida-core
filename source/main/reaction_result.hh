//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2001 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef REACTION_RESULT_HH
#define REACTION_RESULT_HH

#include "../tools/tArray.hh"

class cReactionResult {
private:
  tArray<double> resources_consumed;
  tArray<double> resources_produced;
  tArray<bool> tasks_done;
  tArray<bool> reactions_triggered;
  double bonus_add;
  double bonus_mult;

  bool active_reaction;

  inline void ActivateReaction();
public:
  cReactionResult(const int num_resources, const int num_tasks,
		  const int num_reactions);
  ~cReactionResult();

  bool GetActive() { return active_reaction; }

  void Consume(int id, double num);
  void Produce(int id, double num);
  void MarkTask(int id);
  void MarkReaction(int id);
  void AddBonus(double value);
  void MultBonus(double value);

  double GetConsumed(int id);
  double GetProduced(int id);
  bool ReactionTriggered(int id);
  bool TaskDone(int id);
  double GetAddBonus();
  double GetMultBonus();
};

#endif
