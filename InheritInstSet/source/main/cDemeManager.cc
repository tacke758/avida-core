/*
 *  cDemeManager.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeManager.h"

// CompeteDemes  probabilistically copies demes into the next generation
// based on their fitness. How deme fitness is estimated is specified by 
// competition_type input argument as:
/*
 0: deme fitness = 1 (control, random deme selection)
 1: deme fitness = number of births since last competition (default) 
 2: deme fitness = average organism fitness at the current update (uses parent's fitness, so
 does not work with donations)
 3: deme fitness = average mutation rate at the current update
 4: deme fitness = strong rank selection on (parents) fitness (2^-deme fitness rank)
 5: deme fitness = average organism life (current, not parents) fitness (works with donations)
 6: deme fitness = strong rank selection on life (current, not parents) fitness
 */
//  For ease of use, each organism 
// is setup as if it we just injected into the population.

void cDemeManager:CompeteDemes(void (*FitFunc)(cDemeManager&))
{
  &FitFunc(*this);
  tArray<int> deme_count = CompetitionResults();
  
  tArray<bool> is_init(num_demes, false); 
  
  // Copy demes until all deme counts are 1.
  while (true) {
    // Find the next deme to copy...
    int from_deme_id, to_deme_id;
    for (from_deme_id = 0; from_deme_id < num_demes; from_deme_id++) {
      if (deme_count[from_deme_id] > 1) break;
    }
    
    // Stop If we didn't find another deme to copy
    if (from_deme_id == num_demes) break;
    
    for (to_deme_id = 0; to_deme_id < num_demes; to_deme_id++) { 
      if (deme_count[to_deme_id] == 0) break;
    }
    
    // We now have both a from and a to deme....
    deme_count[from_deme_id]--;
    deme_count[to_deme_id]++;
       
    
    if (m_world->GetConfig().DEMES_USE_GERMLINE.Get()){
      
    }
    else{
      
    }
  }   
  
  // Now re-inject all remaining demes into themselves to reset them.
      
    
  // Reset all deme stats to zero.
  ResetStats(); 
}








void cDemeManager::ReplicateDemes(void (*trigger)(cDemeManager&) )
{
  /* Check if any demes have met the critera to be replicated and do so.
   There are several bases this can be checked on:
   
   0: 'all'       - ...all non-empty demes in the population.
   1: 'full_deme' - ...demes that have been filled up.
   2: 'corners'   - ...demes with upper left and lower right corners filled.
   3: 'deme-age'  - ...demes who have reached their maximum age
   */

  assert(GetNumDemes()>1); // Sanity check.
  
  // Determine which demes should be replicated.
  const int num_demes = GetNumDemes();
  
  // Loop through all candidate demes...
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDeme & source_deme = deme_array[deme_id];
    
    // Doesn't make sense to try and replicate a deme that *has no organisms*.
    if(source_deme.IsEmpty()) continue;
    
    // Test this deme to determine if it should be replicated.  If not,
    // continue on to the next deme.
    switch (rep_trigger) {
      case 0: {
        // Replicate all non-empty demes.
        if (source_deme.IsEmpty()) continue;
        break;
      }
      case 1: {
        // Replicate all full demes.
        if (source_deme.IsFull() == false) continue;
        break;
      }
      case 2: {
        // Replicate all demes with the corners filled in.
        // The first and last IDs represent the two corners.
        const int id1 = source_deme.GetCellID(0);
        const int id2 = source_deme.GetCellID(source_deme.GetSize() - 1);
        if(cell_array[id1].IsOccupied() == false ||
           cell_array[id2].IsOccupied() == false) continue;
        break;
      }
      case 3: {
        // Replicate old demes.
        if(source_deme.GetAge() < m_world->GetConfig().MAX_DEME_AGE.Get()) continue;
        break;
      }
      default: {
        cerr << "ERROR: Invalid replication trigger " << rep_trigger
        << " in cPopulation::ReplicateDemes()" << endl;
        assert(false);
      }
    }

    // -- If we made it this far, we should replicate this deme --
    int target_id = deme_id;
    while(target_id == deme_id) 
      target_id = m_world.GetRandom().GetUInt(num_demes);
    
    if(m_world->GetConfig().DEMES_USE_GERMLINE.Get()){
      SterileGermlineInjection(src_id, target_id);
      SterileGermlineInjection(src_id, src_id);
    }
    else
    {
      SterileRandomInjection(src_id, target_id);
      SterileRandomInjection(src_id, src_id);
    }
  }
}




tArray<int> cDemeManager::CompetitionResults()
{
  // Pick which demes should be in the next generation.
  tArray<int> new_demes(m_demes.GetSize());
  for (int i = 0; i < num_demes; i++) {
    double birth_choice = (double) m_world->GetRandom().GetDouble(total_fitness);
    double test_total = 0;
    for (int test_deme = 0; test_deme < num_demes; test_deme++) {
      test_total += deme_fitness[test_deme];
      if (birth_choice < test_total) {
        new_demes[i] = test_deme;
        break;
      }
    }
  }
  
  // Track how many of each deme we should have.
  tArray<int> deme_count(m_demes.GetSize());
  deme_count.SetAll(0);
  for (int i = 0; i < num_demes; i++) {
    deme_count[new_demes[i]]++;
  }
  return deme_count;
}



void cDemeManager::SterilizeDeme(int id)
{
  for (int i=0; i< m_demes[id].GetSize(); i++)
    cPopulation::KillOrganism(cell_array[m_demes[id].GetCellID(i)]);
  return;
}




void cDemeManager::CopyDeme(int source_id, int target_id){
  SterilizeDeme(target_id);
  for (int i = 0; i < m_demes[source_id].GetSize(); i++) 
    cPopulation::CopyClone(m_demes[source_id].GetCellID(i), m_demes[target_id].GetCellID(i) );    
}




void cDemeManager:CopyDemeGermline(int source_id, int target_id){
  SterilizeDeme(target_id);
  if(m_world->GetConfig().GERMLINE_RANDOM_PLACEMENT.Get() == 2) {
    // organism is randomly placed in deme
    source_deme_inject_cell = source_deme.GetCellID(m_world->GetRandom().GetInt(0, source_deme.GetSize()-1));
    target_deme_inject_cell = target_deme.GetCellID(m_world->GetRandom().GetInt(0, target_deme.GetSize()-1));
  } else {
    // organisms is placed in center of deme
    source_deme_inject_cell = source_deme.GetCellID(source_deme.GetSize()/2);
    target_deme_inject_cell = target_deme.GetCellID(target_deme.GetSize()/2);
  }
  // Lineage label is wrong here; fix.
  InjectGenome(source_deme_inject_cell, source_germline.GetLatest(), 0); // source deme
  InjectGenome(target_deme_inject_cell, target_germline.GetLatest(), 0); // target deme
  
  if(m_world->GetConfig().GERMLINE_RANDOM_PLACEMENT.Get() == 1) {
    // Rotate both injected cells to face northwest.
    int offset = source_deme.GetCellID(0);
    cell_array[source_deme_inject_cell].Rotate(cell_array[GridNeighbor(source_deme_inject_cell-offset,
                                                                       source_deme.GetWidth(),
                                                                       source_deme.GetHeight(), -1, -1)+offset]);
    offset = target_deme.GetCellID(0);
    cell_array[target_deme_inject_cell].Rotate(cell_array[GridNeighbor(target_deme_inject_cell-offset,
                                                                       target_deme.GetWidth(), 
                                                                       target_deme.GetHeight(), -1, -1)+offset]);
  }
}




cGenome cDemeManager:DoGermlineMutation(const cGenome& source_germ)
{
  cGenome next_germ(source_germ);
  if(m_world->GetConfig().GERMLINE_COPY_MUT.Get() > 0)
    for(int i=0; i<next_germ.GetSize(); ++i) 
      if(m_world->GetRandom().P(m_world->GetConfig().GERMLINE_COPY_MUT.Get())) 
        next_germ[i] = m_world->GetHardwareManager().GetInstSet().GetRandomInst(ctx);
  return next_germ;
}




void cDemeManager::SterileGermlineInjection(int source_id, int target_id)
{
  // Get the latest germ from the source deme.
  cGenome& source_germline =  m_demes[source_id].GetGermline().GetLatest()      
  cGenome next_germ = DoGermlineMutation(source_germline);
  
  // Here we're adding the next_germ to the germline(s).  Note the
  // config option to determine if we should update the source_germline
  // as well.
  m_demes[target_id].ReplaceGermline(source_germline);
  m_demes[target_id].GetGermline().Add(next_germ);
  
  if(m_world->GetConfig().GERMLINE_REPLACES_SOURCE.Get())
    source_germline.Add(next_germ);
    
  // And reset both demes, in case they have any cleanup work to do.
  target_deme.Reset();
  
  CopyDemeGermline(src_id, target_id);
}



void cDemeManager::SterileRandomInjection(int source_id, int target_id)
{
  int source_cell = m_demes[src_id].GetRandomOrganism();
  target_deme.Reset();
  
  if(m_world->GetConfig().ENERGY_ENABLED.Get() == 1) {
    cOrganism* seed_org = cell_array[source_cell].GetOrganism();
    cGenome seed_genome = seed_org->GetGenome();
    int seed_lineage = seed_org->GetLineageLabel();
    
    SterilizeDeme(target_id);
    
    int target_deme_inject_cell = target_deme.GetCellID(target_deme.GetSize()/2);
    InjectGenome(target_deme_inject_cell, seed_genome, seed_lineage); // target deme
    
    // Rotate to face northwest
    offset = target_deme.GetCellID(0);
    cell_array[target_deme_inject_cell].Rotate(cell_array[GridNeighbor(target_deme_inject_cell-offset,
                                                                       target_deme.GetWidth(), 
                                                                       target_deme.GetHeight(), -1, -1)+offset]);
  } 
  else {
    
    SterilizeDeme(target_id);
    
    // And do the replication into the central cell of the target deme...
    const int target_cell = target_deme.GetCellID(target_deme.GetWidth()/2, target_deme.GetHeight()/2);
    CopyClone(source_cell, target_cell);
    
    // Rotate both injected cells to face northwest.
    int offset=target_deme.GetCellID(0);
    cell_array[target_cell].Rotate(cell_array[GridNeighbor(target_cell-offset,
                                                        target_deme.GetWidth(), 
                                                        target_deme.GetHeight(), -1, -1)+offset]);
    }
}

