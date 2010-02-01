/*
 *  DemeActions.h
 *  Avida
 *
 *  Created by Matthew Rupp on 2/1/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef DemeActions_h
#define DemeActions_h

#include "cAction.h"

class cActionLibrary;
class cAvidaContext;
class cDeme;


class cDemeAction : public cAction
{
  private:
    void Process(cAvidaContext& ctx) {;}
  public:
    cDemeAction(cWorld* world, const cString& args) : cAction(world,args) {;};
    virtual void Process(cAvidaContext& ctx, cDeme* deme) = 0;
};

void RegisterDemeActions(cActionLibrary* action_lib);

#endif
