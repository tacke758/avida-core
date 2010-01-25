/*
 *  cDemeMethodsLib.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/25/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#ifndef cDemeMethods_h
#define cDemeMethods_h

#include "cDemeCompetitions.h"
#include "cDemeSelections.h"
#include "cDemeTriggers.h"
#include "cDemeReplications.h"

class cString;

class cDemeMethodsLib
{
  private:
    tDictionary<tDemeCompetition> m_competitions;
    tDictionary<tDemeSelection>   m_selections;
    tDictionary<tDemeTrigger>     m_triggers;
    tDictionary<tDemeReplication> m_replications;
  
  public:
  cDemeMethodsLib() {Setup();}
  
  void Setup();
  
  tDemeCompetition GetCompetition(const cString& name);
  tDemeSelection   GetSelection(const cString& name);
  tDemeTrigger     GetTrigger(const cString& name);
  tDemeReplication GetReplication(const cString& name);
};


#endif
