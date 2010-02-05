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
#include "cEventContext.h"

class cActionLibrary;
class cAvidaContext;
class cDeme;


class cDemeAction : public cAction
{
  private:
  public:
    cDemeAction(cWorld* world, const cString& args) : cAction(world,args) {;};
    virtual void Process(cEventContext&) = 0;
};

void RegisterDemeActions(cActionLibrary* action_lib);

#endif
