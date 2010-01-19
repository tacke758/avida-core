/*
 *  cDemeManager.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeManager.h"

#include "cAvidaConfig.h"
#include "cDemeCompetitions.h"
#include "cEnvironment.h"
#include "cInstSet.h"
#include "cHardwareManager.h"
#include "cOrganism.h"
#include "cPopulationCell.h"
#include "cRandom.h"
#include "cStats.h"
#include "cWorld.h"

#include <cassert>



cDemeManager::cDemeManager(cPopulation& p) : 
m_world(p.m_world), 
m_population(p)
{
  int world_x = m_world->GetConfig().WORLD_X.Get();
  int world_y = m_world->GetConfig().WORLD_Y.Get();
  int num_demes = m_world->GetConfig().NUM_DEMES.Get();
  
  // What are the sizes of the demes that we're creating?
  const int deme_size_x = world_x;
  const int deme_size_y = world_y / num_demes;
  const int deme_size = deme_size_x * deme_size_y;
  
  m_demes.Resize(num_demes);
  
  // Setup the deme structures.
  tArray<int> deme_cells(deme_size);
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int offset = 0; offset < deme_size; offset++) {
      int cell_id = deme_id * deme_size + offset;
      deme_cells[offset] = cell_id;
      m_population.cell_array[cell_id].SetDemeID(deme_id);
    }
    m_demes[deme_id] = new cDeme();
    m_demes[deme_id]->Setup(deme_cells, deme_size_x, m_world);
  }
}


cDemeManager::~cDemeManager()
{
  for (int k=0; k < GetNumDemes(); k++)
    delete m_demes[k];
}


void cDemeManager::CompeteDemes(void (*FitFunc)(cDemeManager&), tArray<int> (*SelFunc)(cDemeManager&))
{
  FitFunc(*this);
  tArray<int> deme_count = SelFunc(*this);
  
  int num_demes = GetNumDemes();
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
       
    if(m_world->GetConfig().DEMES_USE_GERMLINE.Get())
      SterileGermlineInjection(from_deme_id, to_deme_id);
    else
      SterileRandomInjection(from_deme_id, to_deme_id);
    
    is_init[to_deme_id] = true;
  }
  
  // Now re-inject all remaining demes into themselves to reset them.
  for (int i = 0; i < GetNumDemes(); i++)
    if (!is_init[i]) SterileRandomInjection(i,i);
    
  // Reset all deme stats to zero.
  ResetStats(); 
}





void cDemeManager::ReplicateDemes(bool (*trigger)(cDemeManager&, int) )
{
  assert(GetNumDemes()>1); // Sanity check.
  
  // Determine which demes should be replicated.
  const int num_demes = GetNumDemes();
  
  // Loop through all candidate demes...
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    
    // Doesn't make sense to try and replicate a deme that *has no organisms*.
    if (GetDeme(deme_id)->IsEmpty() || trigger(*this,deme_id) == false) continue;
    
    // -- If we made it this far, we should replicate this deme --
    int target_id = deme_id;
    while(target_id == deme_id) 
      target_id = m_world->GetRandom().GetUInt(num_demes);
    
    if(m_world->GetConfig().DEMES_USE_GERMLINE.Get()){
      SterileGermlineInjection(deme_id, target_id);
      SterileGermlineInjection(deme_id, deme_id);
    }
    else{
      SterileRandomInjection(deme_id, target_id);
      SterileRandomInjection(deme_id, deme_id);
    }
  }
}



void cDemeManager::SpawnDeme(int src_id, int target_id)
{
  assert(src_id != target_id);
  if (target_id == -1)
    do{
      target_id = m_world->GetRandom().GetUInt(GetNumDemes());
    } while (target_id != src_id);
  SterileRandomInjection(src_id, target_id);
}



// Reset Demes goes through each deme and resets the individual organisms as
// if they were just injected into the population.
void cDemeManager::ResetDemes()
{
  for (int deme_id = 0; deme_id < GetNumDemes(); deme_id++)
    CopyDeme(deme_id, deme_id);
}




void cDemeManager::SterilizeDeme(int id)
{
  for (int i=0; i< GetDeme(id)->GetSize(); i++)
    m_population.KillOrganism(m_population.cell_array[GetDeme(id)->GetCellID(i)]);
  return;
}






void cDemeManager::CopyDeme(int source_id, int target_id){
  for (int i = 0; i < GetDeme(source_id)->GetSize(); i++) 
    m_population.CopyClone(GetDeme(source_id)->GetCellID(i), GetDeme(target_id)->GetCellID(i) );    
}






void cDemeManager::CopyDemeGermline(int source_id, int target_id)
{
  int target_deme_inject_cell;
  cDeme* source_deme = GetDeme(source_id);
  cDeme* target_deme = GetDeme(target_id);
  if(m_world->GetConfig().GERMLINE_RANDOM_PLACEMENT.Get() == 2) {
    // organism is randomly placed in deme
    target_deme_inject_cell = target_deme->GetCellID(m_world->GetRandom().GetInt(0, target_deme->GetSize()-1));
  } else {
    // organisms is placed in center of deme
    target_deme_inject_cell = target_deme->GetCellID(target_deme->GetSize()/2);
  }
  // Lineage label is wrong here; fix.
  SterilizeDeme(target_id);
  m_population.InjectGenome(target_deme_inject_cell, source_deme->GetGermline().GetLatest(), 0); // source deme
  
  if(m_world->GetConfig().GERMLINE_RANDOM_PLACEMENT.Get() == 1) {
    int offset = target_deme->GetCellID(0);
    m_population.cell_array[target_deme_inject_cell].Rotate(m_population.cell_array[GridNeighbor(target_deme_inject_cell-offset,
                                                                       target_deme->GetWidth(), 
                                                                       target_deme->GetHeight(), -1, -1)+offset]);
  }
}






cGenome cDemeManager::DoGermlineMutation(const cGenome& source_germ)
{
  cGenome next_germ(source_germ);
  if(m_world->GetConfig().GERMLINE_COPY_MUT.Get() > 0)
    for(int i=0; i<next_germ.GetSize(); i++) 
      if(m_world->GetRandom().P(m_world->GetConfig().GERMLINE_COPY_MUT.Get())) 
        next_germ[i] = m_world->GetHardwareManager().GetInstSet().GetRandomInst(m_world->GetDefaultContext());
  return next_germ;
}






void cDemeManager::SterileGermlineInjection(int source_id, int target_id)
{
  // Get the latest germ from the source deme.
  cGermline& source_germline =  GetDeme(source_id)->GetGermline();
  cGenome& source_genome = source_germline.GetLatest();
  cGenome next_germ = DoGermlineMutation(source_genome);
  
  // Here we're adding the next_germ to the germline(s).  Note the
  // config option to determine if we should update the source_germline
  // as well.
  GetDeme(target_id)->ReplaceGermline(source_germline);
  GetDeme(target_id)->GetGermline().Add(next_germ);
  
  if(m_world->GetConfig().GERMLINE_REPLACES_SOURCE.Get())
    source_germline.Add(next_germ);
    
  // Reset Deme
  GetDeme(target_id)->Reset();
  
  CopyDemeGermline(source_id, target_id);
}






void cDemeManager::SterileRandomInjection(int src_id, int target_id)
{
  int source_cell = GetDeme(src_id)->GetRandomOrganism();
  GetDeme(target_id)->Reset();
  
  if(m_world->GetConfig().ENERGY_ENABLED.Get() == 1) {
    cOrganism* seed_org = m_population.cell_array[source_cell].GetOrganism();
    cGenome seed_genome = seed_org->GetGenome();
    int seed_lineage = seed_org->GetLineageLabel();
    
    SterilizeDeme(target_id);
    
    int target_deme_inject_cell = GetDeme(target_id)->GetCellID(GetDeme(target_id)->GetSize()/2);
    m_population.InjectGenome(target_deme_inject_cell, seed_genome, seed_lineage); // target deme
    
    // Rotate to face northwest
    int offset = target_deme.GetCellID(0);
    m_population.cell_array[target_deme_inject_cell].Rotate(m_population.cell_array[GridNeighbor(target_deme_inject_cell-offset,
                                                                       target_deme.GetWidth(), 
                                                                       target_deme.GetHeight(), -1, -1)+offset]);
  } 
  else {
    
    SterilizeDeme(target_id);
    
    // And do the replication into the central cell of the target deme...
    const int target_cell = GetDeme(target_id)->GetCellID(GetDeme(target_id)->GetWidth()/2, target_deme.GetHeight()/2);
    m_population.CopyClone(source_cell, target_cell);
    
    // Rotate both injected cells to face northwest.
    int offset=target_deme.GetCellID(0);
    m_population.cell_array[target_cell].Rotate(m_population.cell_array[GridNeighbor(target_cell-offset,
                                                        target_deme.GetWidth(), 
                                                        target_deme.GetHeight(), -1, -1)+offset]);
    }
}




tArray<int> cDemeManager::CompeteFitnessProportional(cDemeManager& mgr)
{
  int num_demes = mgr.GetNumDemes();
  // Pick which demes should be in the next generation.
  tArray<int> new_demes(num_demes, -1);
  for (int i = 0; i < num_demes; i++) {
    double birth_choice = (double) mgr.m_world->GetRandom().GetDouble(mgr.m_total_fitness);
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


tArray<int> cDemeManager::CompeteTournament(cDemeManager& mgr)
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
    for (int k = 0; k < m_world->GetConfig().NUM_DEME_TOURNAMENTS().Get(); k++)
      player_id = deme_ids[mgr.m_world->GetRandom().GetUInt(valid)];
      win_id = (win_id == -1 || mgr.m_deme_fitness[player_id] > max_fitness) ? player_id : win_id;
    deme_count[win_id]++;
  }
  return deme_count;
}



void cDemeManager::UpdateDemeCellResources(const tArray<double> & res_change, 
                                          const int cell_id)
{
  GetDeme(m_population.GetCell(cell_id).GetDemeID())->ModifyDemeResCount(res_change, cell_id);
}




// Print out all statistics about individual demes
void cDemeManager::PrintDemeAllStats() {
  PrintDemeFitness();
  PrintDemeLifeFitness();
  PrintDemeMerit();
  PrintDemeGestationTime();
  PrintDemeTasks();
  PrintDemeDonor();
  PrintDemeReceiver();
  PrintDemeMutationRate();
  PrintDemeResource();
  PrintDemeInstructions();
  
  if(m_world->GetConfig().ENERGY_ENABLED.Get() == 1) {
    PrintDemeSpatialEnergyData();
    PrintDemeSpatialSleepData();
  }
}




void cDemeManager::PrintDemeDonor() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_donor = m_world->GetDataFile("deme_donor.dat");
  df_donor.WriteComment("Num orgs doing doing a donate for each deme in population");
  df_donor.WriteTimeStamp();
  df_donor.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_donor;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_donor.Add(phenotype.IsDonorLast()); 	
    }
    comment.Set("Deme %d", deme_id);
    df_donor.Write(single_deme_donor.Sum(), comment);
  }
  df_donor.Endl();
}




void cDemeManager::PrintDemeFitness() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_fit = m_world->GetDataFile("deme_fitness.dat");
  df_fit.WriteComment("Average fitnesses for each deme in the population");
  df_fit.WriteTimeStamp();
  df_fit.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_fitness;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_fitness.Add(phenotype.GetFitness()); 	
    }
    comment.Set("Deme %d", deme_id);
    df_fit.Write(single_deme_fitness.Ave(), comment);
  }
  df_fit.Endl();
}




void cDemeManager::PrintDemeGestationTime() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_gest = m_world->GetDataFile("deme_gest_time.dat");
  df_gest.WriteComment("Average gestation time for each deme in population");
  df_gest.WriteTimeStamp();
  df_gest.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_gest_time;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_gest_time.Add(phenotype.GetGestationTime()); 	
    }
    comment.Set("Deme %d", deme_id);
    df_gest.Write(single_deme_gest_time.Ave(), comment);
  }
  df_gest.Endl();
}




void cDemeManager::PrintDemeInstructions() {  
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  const int num_inst = m_world->GetNumInstructions();
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString filename;
    filename.Set("deme_instruction-%d.dat", deme_id);
    cDataFile & df_inst = m_world->GetDataFile(filename); 
    cString comment;
    comment.Set("Number of times each instruction is exectued in deme %d",
                deme_id);
    df_inst.WriteComment(comment);
    df_inst.WriteTimeStamp();
    df_inst.Write(stats.GetUpdate(), "update");
    
    tArray<cIntSum> single_deme_inst(num_inst);
    
    const cDeme & cur_deme = *GetDeme(deme_id);
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      
      for (int j = 0; j < num_inst; j++) {
        single_deme_inst[j].Add(phenotype.GetLastInstCount()[j]);
      } 
    }
    
    for (int j = 0; j < num_inst; j++) {
      comment.Set("Inst %d", j);
      df_inst.Write((int) single_deme_inst[j].Sum(), comment);
    }
    df_inst.Endl();    
  }
}



void cDemeManager::PrintDemeLifeFitness() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_life_fit = m_world->GetDataFile("deme_lifetime_fitness.dat");
  df_life_fit.WriteComment("Average life fitnesses for each deme in the population");
  df_life_fit.WriteTimeStamp();
  df_life_fit.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_life_fitness;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_life_fitness.Add(phenotype.GetLifeFitness()); 	
    }
    comment.Set("Deme %d", deme_id);
    df_life_fit.Write(single_deme_life_fitness.Ave(), comment);
  }
  df_life_fit.Endl();
}




void cDemeManager::PrintDemeMerit() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_merit = m_world->GetDataFile("deme_merit.dat");
  df_merit.WriteComment("Average merits for each deme in population");
  df_merit.WriteTimeStamp();
  df_merit.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_merit;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_merit.Add(phenotype.GetMerit().GetDouble()); 	
    }
    comment.Set("Deme %d", deme_id);
    df_merit.Write(single_deme_merit.Ave(), comment);
  }
  df_merit.Endl();
}




void cDemeManager::PrintDemeMutationRate() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_mut_rates = m_world->GetDataFile("deme_mut_rates.dat");
  df_mut_rates.WriteComment("Average mutation rates for organisms in each deme");
  df_mut_rates.WriteTimeStamp();
  df_mut_rates.Write(stats.GetUpdate(), "update");
  cDoubleSum total_mut_rate;
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_mut_rate;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      single_deme_mut_rate.Add(m_population.GetCell(cur_cell).GetOrganism()->MutationRates().GetCopyMutProb());
    }
    comment.Set("Deme %d", deme_id);
    df_mut_rates.Write(single_deme_mut_rate.Ave(), comment);
    total_mut_rate.Add(single_deme_mut_rate.Ave());
  }
  df_mut_rates.Write(total_mut_rate.Ave(), "Average deme mutation rate averaged across Demes.");
  df_mut_rates.Endl();
}




void cDemeManager::PrintDemeReceiver() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_receiver = m_world->GetDataFile("deme_receiver.dat");
  df_receiver.WriteComment("Num orgs doing receiving a donate for each deme in population");
  df_receiver.WriteTimeStamp();
  df_receiver.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    cDoubleSum single_deme_receiver;
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_receiver.Add(phenotype.IsReceiver()); 	
    }
    comment.Set("Deme %d", deme_id);
    df_receiver.Write(single_deme_receiver.Sum(), comment);
  }
  df_receiver.Endl();
}




void cDemeManager::PrintDemeResource() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  cDataFile & df_resources = m_world->GetDataFile("deme_resources.dat");
  df_resources.WriteComment("Avida deme resource data");
  df_resources.WriteTimeStamp();
  df_resources.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDeme & cur_deme = *GetDeme(deme_id);
    
    cur_deme.UpdateDemeRes();
    cResourceCount res = GetDeme(deme_id)->GetDemeResourceCount();
    for(int j = 0; j < res.GetSize(); j++) {
      const char * tmp = res.GetResName(j);
      df_resources.Write(res.Get(j), cStringUtil::Stringf("Deme %d Resource %s", deme_id, tmp)); //comment);
      if((res.GetResourcesGeometry())[j] != nGeometry::GLOBAL) {
        PrintDemeSpatialResData(res, j, deme_id);
      }
    }
  }
  df_resources.Endl();
}




// Write spatial energy data to a file that can easily be read into Matlab
void cDemeManager::PrintDemeSpatialEnergyData() {
  int cellID = 0;
  int update = m_world->GetStats().GetUpdate();
  
  for(int i = 0; i < GetNumDemes(); i++) {
    cString tmpfilename = cStringUtil::Stringf( "deme_%07i_spacial_energy.m", i);  // moved here for easy movie making
    cDataFile& df = m_world->GetDataFile(tmpfilename);
    cString UpdateStr = cStringUtil::Stringf( "deme_%07i_energy_%07i = [ ...", i, update );
    df.WriteRaw(UpdateStr);
    
    int gridsize = GetDeme(i)->GetSize();
    int xsize = m_world->GetConfig().WORLD_X.Get();
    
    // write grid to file
    for (int j = 0; j < gridsize; j++) {
      cPopulationCell& cell = m_population.GetCell(cellID);
      if(cell.IsOccupied()) {
        df.WriteBlockElement(cell.GetOrganism()->GetPhenotype().GetStoredEnergy(), j, xsize);
      } else {
        df.WriteBlockElement(0.0, j, xsize);
      }
      cellID++;
    }
    df.WriteRaw("];");
    df.Endl();
  }
}



// Write spatial data to a file that can easily be read into Matlab
void cDemeManager::PrintDemeSpatialResData( cResourceCount res, const int i, const int deme_id) 
{
  const char* tmpResName = res.GetResName(i);
  cString tmpfilename = cStringUtil::Stringf( "deme_spacial_resource_%s.m", tmpResName );
  cDataFile& df = m_world->GetDataFile(tmpfilename);
  cString UpdateStr = cStringUtil::Stringf( "deme_%07i_%s_%07i = [ ...", deme_id, static_cast<const char*>(res.GetResName(i)), m_world->GetStats().GetUpdate() );
  
  df.WriteRaw(UpdateStr);
  
  cSpatialResCount sp_res = res.GetSpatialResource(i);
  int gridsize = sp_res.GetSize();
  int xsize = m_world->GetConfig().WORLD_X.Get();
  
  for (int j = 0; j < gridsize; j++) {
    df.WriteBlockElement(sp_res.GetAmount(j), j, xsize);
  }
  df.WriteRaw("];");
  df.Endl();
}



// Write spatial energy data to a file that can easily be read into Matlab
void cDemeManager::PrintDemeSpatialSleepData() 
{
  int cellID = 0;
  cString tmpfilename = "deme_spacial_sleep.m";
  cDataFile& df = m_world->GetDataFile(tmpfilename);
  int update = m_world->GetStats().GetUpdate();
  
  for(int i = 0; i < GetNumDemes(); i++) {
    cString UpdateStr = cStringUtil::Stringf( "deme_%07i_sleep_%07i = [ ...", i, update);
    df.WriteRaw(UpdateStr);
    
    int gridsize = GetDeme(i)->GetSize();
    int xsize = m_world->GetConfig().WORLD_X.Get();
    
    // write grid to file
    for (int j = 0; j < gridsize; j++) {
      cPopulationCell cell = m_population.GetCell(cellID);
      if(cell.IsOccupied()) {
        df.WriteBlockElement(cell.GetOrganism()->IsSleeping(), j, xsize);
      } else {
        df.WriteBlockElement(0.0, j, xsize);
      }
      cellID++;
    }
    df.WriteRaw("];");
    df.Endl();
  }
}



void cDemeManager::PrintDemeTasks() {
  cStats& stats = m_world->GetStats();
  const int num_demes = GetNumDemes();
  const int num_task = environment.GetNumTasks();
  cDataFile & df_task = m_world->GetDataFile("deme_task.dat");
  df_task.WriteComment("Num orgs doing each task for each deme in population");
  df_task.WriteTimeStamp();
  df_task.Write(stats.GetUpdate(), "update");
  
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cString comment;
    const cDeme & cur_deme = *GetDeme(deme_id);
    tArray<cIntSum> single_deme_task(num_task);
    
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (m_population.cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = m_population.GetCell(cur_cell).GetOrganism()->GetPhenotype();
      for (int j = 0; j < num_task; j++) {
        // only interested if task is done once! 
        if (phenotype.GetLastTaskCount()[j] > 0) {
          single_deme_task[j].Add(1);
        }
      }
    }
    for (int j = 0; j < num_task; j++) {
      comment.Set("Deme %d, Task %d", deme_id, j);
      df_task.Write((int) single_deme_task[j].Sum(), comment);
    }
  }
  df_task.Endl();
}
