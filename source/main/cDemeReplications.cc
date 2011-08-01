/*
 *  cDemeReplication.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/25/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cAvidaConfig.h"
#include "cDeme.h"
#include "cDemeManager.h"
#include "cDemeReplications.h"
#include "cGenome.h"
#include "cGermline.h"
#include "cPopulation.h"
#include "cWorld.h"


bool cDemeReplications::PrepareReplication(cDemeManager& mgr, int source_id, int target_id)
{
  cDeme* source = mgr.GetDeme(source_id);
  cDeme* target = mgr.GetDeme(target_id);
  assert(source != NULL && target != NULL);
  
  bool using_germline = (mgr.m_world->GetConfig().DEMES_USE_GERMLINES.Get() > 0);
  
  target->Reset();
  
  if (using_germline){
    if (source_id != target_id)
      mgr.CopyDemeGermline(source_id, target_id);
  }
  return true;
}

bool cDemeReplications::PrepareReplication(cDemeManager& mgr, int source_id, int target_id, const cGenome*& injected)
{
  cDeme* source = mgr.GetDeme(source_id);
  cDeme* target = mgr.GetDeme(target_id);
  assert(source != NULL && target != NULL);
  
  bool using_germline = (mgr.m_world->GetConfig().DEMES_USE_GERMLINES.Get() > 0);
  
  target->Reset();
  
  if (using_germline){
    if (source_id != target_id)
      mgr.CopyDemeGermline(source_id, target_id);
    injected = &target->GetGermline().GetLatest();
    return true;
  } else{
    injected = mgr.SampleRandomDemeGenome(source_id);
    return (injected==NULL) ? false : true;
  }
}



void cDemeReplications::SterileInjectionAtCenter(cDemeManager& mgr, int source_id, int target_id)
{
  const cGenome* inject;
  if (cDemeReplications::PrepareReplication(mgr, source_id, target_id, inject))
    mgr.SterileInjectCenter(cGenome(*inject), target_id);
}



void cDemeReplications::SterileInjectionAtRandom(cDemeManager& mgr, int source_id, int target_id)
{
  const cGenome* inject;
  if (cDemeReplications::PrepareReplication(mgr, source_id, target_id, inject))
     mgr.SterileInjectRandom(cGenome(*inject), target_id);
}



void cDemeReplications::SterileFullInjection(cDemeManager& mgr, int source_id, int target_id)
{
  const cGenome* inject;
  if (cDemeReplications::PrepareReplication(mgr, source_id, target_id, inject)){
    cGenome copy(*inject);
    mgr.SterileInjectFull(copy, target_id);
  }
}


void cDemeReplications::InjectCopy(cDemeManager& mgr, int source_id, int target_id)
{
  cDemeReplications::PrepareReplication(mgr, source_id, target_id);
  mgr.CopyDeme(source_id, target_id);
}



