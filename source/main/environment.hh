//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 2003 California Institute of Technology             //
//                                                                          //
// Read the COPYING and README files, or contact 'avida@alife.org',         //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef ENVIRONMENT_HH
#define ENVIRONMENT_HH

#include "../tools/tArray.hh"
#include "../tools/tBuffer.hh"
#include "../tools/tList.hh"

#include "resource.hh"
#include "reaction.hh"
#include "reaction_result.hh"
#include "tasks.hh"
#include "inst_set.hh"
#include "mutations.hh"

class cEnvironment {
private:
  // Keep libraries of resources, reactions, and tasks.
  cResourceLib resource_lib;
  cReactionLib reaction_lib;
  cMutationLib mutation_lib;
  cTaskLib task_lib;
  cInstSet inst_set;
  cMutationRates mut_rates;

  static bool ParseSetting(cString entry, cString & var_name,
			   cString & var_value, const cString & var_type);
  static bool AssertInputInt(const cString & input, const cString & name,
			     const cString & type);
  static bool AssertInputDouble(const cString & input, const cString & name,
				const cString & type);
  static bool AssertInputValid(void * input, const cString & name,
			       const cString & type, const cString & value);

  bool LoadReactionProcess(cReaction * reaction, cString desc);
  bool LoadReactionRequisite(cReaction * reaction, cString desc);
  bool LoadResource(cString desc);
  bool LoadReaction(cString desc);
  bool LoadMutation(cString desc);

  bool TestRequisites(const tList<cReactionRequisite> & req_list,
		      int task_count,
		      const tArray<int> & reaction_count) const;
  void DoProcesses(const tList<cReactionProcess> & process_list,
		   const tArray<double> & resource_count,
		   const double task_quality,
		   cReactionResult & result) const;
public:
  cEnvironment();
  cEnvironment(const cString & filename);
  ~cEnvironment() { ; }

  /**
   * Reads the environment from disk.
   **/
  bool Load(const cString & filename);

  // Interaction with the organisms
  void SetupInputs( tArray<int> & input_array ) const;

  bool TestInput(  cReactionResult & result,
		   const tBuffer<int> & inputs,
		   const tBuffer<int> & outputs,
		   const tArray<double> & resource_count ) const;

  bool TestOutput(  cReactionResult & result,
		    const tBuffer<int> & inputs,
		    const tBuffer<int> & outputs,
		    const tArray<int> & task_count,
		    const tArray<int> & reaction_count,
		    const tArray<double> & resource_count ) const;

  // Accessors
  const cResourceLib & GetResourceLib() const { return resource_lib; }
  const cReactionLib & GetReactionLib() const { return reaction_lib; }
  const cMutationLib & GetMutationLib() const { return mutation_lib; }
  const cTaskLib & GetTaskLib() const { return task_lib; }
  const cInstSet & GetInstSet() const { return inst_set; }
  const cMutationRates & GetMutRates() const { return mut_rates; }

  cResourceLib & GetResourceLib() { return resource_lib; }
  cInstSet & GetInstSet() { return inst_set; }
  cMutationRates & GetMutRates() { return mut_rates; }
};

#endif
