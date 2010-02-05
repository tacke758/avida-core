/*
 *  DemeActions.cc
 *  Avida
 *
 *  Created by Matthew Rupp on 2/1/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "DemeActions.h"

#include "cAction.h"
#include "cActionLibrary.h"
#include "cDeme.h"
#include "cDemeManager.h"
#include "cString.h"

#include <iostream>

class cDemeActionMutateInstSetID : public cDemeAction
  {
  private:

  public:
    cDemeActionMutateInstSetID(cWorld* world, const cString& args) : cDemeAction(world, args)
    {
      cString largs(args);
    }
    
    static const cString GetDescription() { return "Arguments: <path> <id>"; }
    
    void Process(cAvidaContext& ctx)
    {
      return;
    }
    
    void Process(cEventContext& ctx)
    {
      cerr << "Made it into process." << endl;
      int source_id = (*ctx["source_id"]).AsInt();
      int target_id = (*ctx["target_id"]).AsInt();
      cerr << "Event Triggered " << source_id << " " << target_id << " " << endl;
    }
  
  };


void RegisterDemeActions(cActionLibrary* action_lib)
{
  action_lib->Register<cDemeActionMutateInstSetID>("MutateDemeInstSetID");
  return;  
}
