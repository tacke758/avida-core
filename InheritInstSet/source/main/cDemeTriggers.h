/*
 *  cDemeTriggers.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/19/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cDemeTriggers_h
#define cDemeTriggers_h

class cDemeManager;

typedef bool (*tDemeTrigger)(cDemeManager& mgr, int deme_id);

class cDemeTriggers{
  private:
    cDemeTriggers();
    cDemeTriggers(const cDemeTriggers&);
    cDemeTriggers& operator=(const cDemeTriggers&);
  
  public:
    static bool ReplicateNonEmpty(cDemeManager& mgr, int deme_id);
    static bool ReplicateFull(cDemeManager& mgr, int deme_id);
    static bool ReplicateCornersFilled(cDemeManager& mgr, int deme_id);
    static bool ReplicateOldDemes(cDemeManager& mgr, int deme_id);
};

#endif
