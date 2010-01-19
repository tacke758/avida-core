/*
 *  cDemeTriggers.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/19/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeManager.h"

class cDemeTriggers{

  private:
    cDemeTriggers();
  
  public:
    static bool ReplicateNonEmpty(cDemeManager& mgr);
    static bool ReplicateFull(cDemeManager& mgr);
    static bool ReplicateCornersFilled(cDemeManager& mgr);
    static bool ReplicateOldDemes(cDemeManager& mgr);
};
