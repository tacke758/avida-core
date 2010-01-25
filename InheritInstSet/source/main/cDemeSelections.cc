/*
 *  cDemeSelections.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/25/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeManager.h"
#include "cDemeSelections.h"
#include "cPopulation.h"
#include "cRandom.h"
#include "cWorld.h"


tArray<int> cDemeSelections::FitnessProportional(cDemeManager& mgr)
{
  int num_demes = mgr.GetNumDemes();
  // Pick which demes should be in the next generation.
  tArray<int> new_demes(num_demes, -1);
  for (int i = 0; i < num_demes; i++) {
    double birth_choice = (double) mgr.m_world->GetRandom().GetDouble(mgr.m_total_deme_fitness);
    double test_total = 0.0;
    for (int test_deme = 0; test_deme < num_demes; test_deme++) {
      test_total += mgr.m_deme_fitness[test_deme];
      if (birth_choice < test_total) {
        new_demes[i] = test_deme;
        break;
      }
    }
  }
  
  // Track how many of each deme we should have.
  tArray<int> deme_count(num_demes, 0);
  for (int i = 0; i < num_demes; i++) 
    deme_count[new_demes[i]]++;
  return deme_count;
}


tArray<int> cDemeSelections::Tournament(cDemeManager& mgr)
{
  tArray<int> deme_count(0);
  int num_demes = mgr.GetNumDemes();
  tArray<int> deme_ids(num_demes, -1);
  int valid = 0;
  for (int id = 0; id < num_demes; id++)
    if (mgr.GetDeme(id)->GetBirthCount() > 0)
      deme_ids[valid++] = id;
  
  for (int id = 0; id < num_demes; id++){
    double max_fitness = 0.0;
    int win_id = -1;
    int player_id;
    for (int k = 0; k < mgr.m_world->GetConfig().NUM_DEME_TOURNAMENTS.Get(); k++)
      player_id = deme_ids[mgr.m_world->GetRandom().GetUInt(valid)];
    win_id = (win_id == -1 || mgr.m_deme_fitness[player_id] > max_fitness) ? player_id : win_id;
    deme_count[win_id]++;
  }
  return deme_count;
}
