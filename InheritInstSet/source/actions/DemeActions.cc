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
#include "cString.h"

class cDemeActionMutateInstSetID : public cDemeAction
  {
  private:

  public:
    cDemeActionMutateInstSetID(cWorld* world, const cString& args) : cDemeAction(world, args)
    {
      cString largs(args);
    }
    
    static const cString GetDescription() { return "Arguments: <path> <id>"; }
    
    void Process(cAvidaContext& ctx, cDeme* deme)
    {
    }
  
  };


void RegisterDemeActions(cActionLibrary* action_lib)
{
  action_lib->Register<cDemeActionMutateInstSetID>("MutateDemeInstSetID");
  return;  
}
