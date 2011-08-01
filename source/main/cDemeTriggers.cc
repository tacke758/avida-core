/*
 *  cDemeTriggers.cc
 *  Avida
 *
 *  Created by Matthew Rupp on 1/19/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeManager.h"
#include "cDemeTriggers.h"
#include "cPopulation.h"
#include "cPopulationCell.h"

bool cDemeTriggers::ReplicateNonEmpty(cDemeManager& mgr, int id)
{
  return (mgr.GetDeme(id)->IsEmpty()) ? false : true;
}

bool cDemeTriggers::ReplicateFull(cDemeManager& mgr, int id)
{
  return (mgr.GetDeme(id)->IsFull()) ? true : false;
}

bool cDemeTriggers::ReplicateCornersFilled(cDemeManager& mgr, int id)
{
  cDeme* deme = mgr.GetDeme(id);
  int id1 = deme->GetCellID(0);
  int id2 = deme->GetCellID(deme->GetSize() -1);
  if (mgr.GetPopulation().GetCell(id1).IsOccupied() == false ||
      mgr.GetPopulation().GetCell(id2).IsOccupied() == false) 
    return false;
  return true;
}

bool cDemeTriggers::ReplicateOldDemes(cDemeManager& mgr, int id)
{
  return (mgr.GetDeme(id)->GetAge() < mgr.m_world->GetConfig().MAX_DEME_AGE.Get()) ? false : true;
}

