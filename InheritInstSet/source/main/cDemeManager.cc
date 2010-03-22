/*
 *  cDemeManager.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeManager.h"

#include "defs.h"
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

#include <iostream>
#include <cassert>



cDemeManager::cDemeManager(cPopulation& p) : 
m_world(p.m_world), 
m_population(p)
{
  int world_x = m_world->GetConfig().WORLD_X.Get();
  int world_y = m_world->GetConfig().WORLD_Y.Get();
  int num_demes = m_world->GetConfig().NUM_DEMES.Get();
  
  
  // What are the sizes of the demes that we're creating?
  m_deme_size_x = world_x;
  m_deme_size_y = world_y / num_demes;
  m_deme_size = m_deme_size_x * m_deme_size_y;
  
  cerr << "(" << m_deme_size << ")" << endl;
  
  m_demes.Resize(num_demes);
  m_deme_fitness.Resize(num_demes, 0.0);
  m_total_deme_fitness = 0.0;
  
  // Setup the deme structures.
  tArray<int> deme_cells(m_deme_size);
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int offset = 0; offset < m_deme_size; offset++) {
      int cell_id = deme_id * m_deme_size + offset;
      deme_cells[offset] = cell_id;
      m_population.GetCell(cell_id).SetDemeID(deme_id);
    }
    m_demes[deme_id] = new cDeme(p);
    m_demes[deme_id]->Setup(deme_cells, m_deme_size_x, m_world);
  }
}




cDemeManager::~cDemeManager()
{
  for (int k=0; k < GetNumDemes(); k++)
    delete m_demes[k];
}





void cDemeManager::CompeteDemes(const cString& fit_fun, const cString& sel_fun, const cString& repl_fun)
{
  tDemeCompetition competition = m_methods.GetCompetition(fit_fun);
  tDemeSelection selection = m_methods.GetSelection(sel_fun);
  tDemeReplication replication = m_methods.GetReplication(repl_fun);
  assert(replication != NULL && selection != NULL && competition != NULL);
  
  competition(*this);
  cEventContext state(m_world->GetDefaultContext(), TRIGGER_DEME_COMPETITION_POST);
  m_world->TriggerEvent(state);
  tArray<int> deme_count = selection(*this);
  
  int num_demes = GetNumDemes();
  tArray<bool> is_init(num_demes, false); 
  
  for (int k = 0; k < num_demes; k++)
    cerr << k << "[" << GetDeme(k)->GetInstSetID() << "] =" << deme_count[k] << endl;
  
  // Copy demes until all deme counts are 1.
  while (true) {
    // Find the next deme to copy...
    int source_id, target_id;
    for (source_id = 0; source_id < num_demes; source_id++) {
      if (deme_count[source_id] > 1) break;
    }
    
    // Stop If we didn't find another deme to copy
    if (source_id == num_demes) break;
    
    for (target_id = 0; target_id < num_demes; target_id++) { 
      if (deme_count[target_id] == 0) break;
    }
    
    // We now have both a from and a to deme....
    deme_count[source_id]--;
    deme_count[target_id]++;
    
    cEventContext state(m_world->GetDefaultContext(), TRIGGER_DEME_REPLACEMENT_PRE);
    state << (cCntxEntry("target_id", target_id)) << (cCntxEntry("source_id", source_id));
    m_world->TriggerEvent(state);
    
    replication(*this, source_id, target_id);
    
    state.SetEventTrigger(TRIGGER_DEME_REPLACEMENT_POST);
    m_world->TriggerEvent(state);
    
    is_init[target_id] = true;
  }
  
  for (int i = 0; i < GetNumDemes(); i++)
    if (is_init[i] == false){
      cEventContext state(m_world->GetDefaultContext(), TRIGGER_DEME_REPLACEMENT_PRE);
      state << cCntxEntry("target_id", i) << cCntxEntry("source_id", i);
      m_world->TriggerEvent(state);      
      replication(*this, i, i);
      state.SetEventTrigger(TRIGGER_DEME_REPLACEMENT_POST);
      m_world->TriggerEvent(state);
    }
  ResetDemes();
  cEventContext exitstate(m_world->GetDefaultContext(), TRIGGER_DEME_COMPETITION_FINAL);
  m_world->TriggerEvent(exitstate);
}





void cDemeManager::ReplicateDemes(const cString& trigger_fun, const cString& repl_fun)
{
  tDemeTrigger trigger = m_methods.GetTrigger(trigger_fun);
  tDemeReplication replication = m_methods.GetReplication(repl_fun);
  assert(trigger != NULL && replication != NULL);
  
  assert(GetNumDemes()>1); // Sanity check.
  
  // Determine which demes should be replicated.
  const int num_demes = GetNumDemes();
  
  // Loop through all candidate demes...
  for (int source_id = 0; source_id < num_demes; source_id++) {
    
    // Doesn't make sense to try and replicate a deme that *has no organisms*.
    if (GetDeme(source_id)->IsEmpty() || trigger(*this,source_id) == false) continue;
    
    // -- If we made it this far, we should replicate this deme --
    int target_id = source_id;
    while(target_id == source_id) 
      target_id = m_world->GetRandom().GetUInt(num_demes);
    
    cEventContext state(m_world->GetDefaultContext(), TRIGGER_DEME_REPLACEMENT_PRE);
    state << cCntxEntry("target_id", target_id) << cCntxEntry("source_id", source_id);
    m_world->TriggerEvent(state);
    replication(*this, source_id, target_id);
    GetDeme(target_id)->Reset();
    state.SetEventTrigger(TRIGGER_DEME_REPLACEMENT_POST);
    m_world->TriggerEvent(state);
  }
}


void cDemeManager::CopyDeme(int source_id, int target_id)
{
  cDeme* source = GetDeme(source_id);
  cDeme* target = GetDeme(target_id);
  assert(source != NULL && target != NULL);
  for (int i = 0; i < source->GetSize(); i++) 
    m_population.CopyClone(source->GetCellID(i), target->GetCellID(i));
  source->Reset();
}




void cDemeManager::SpawnDeme(int src_id, int target_id)
{
  assert(src_id != target_id);
  if (target_id == -1)
    do{
      target_id = m_world->GetRandom().GetUInt(GetNumDemes());
    } while (target_id != src_id);
  m_methods.GetReplication("BySterilizingAndInjectingAtRandom")(*this, src_id, target_id);
}





void cDemeManager::ResetDemeOrganisms()
{
  for (int deme_id = 0; deme_id < GetNumDemes(); deme_id++)
    CopyDeme(deme_id, deme_id);
}





void cDemeManager::ResetDemes()
{
  for (int deme_id = 0; deme_id < GetNumDemes(); deme_id++)
    GetDeme(deme_id)->Reset();
}





void cDemeManager::SterilizeDeme(int id)
{
  for (int i=0; i< GetDeme(id)->GetSize(); i++)
    m_population.KillOrganism(m_population.GetCell(GetDeme(id)->GetCellID(i)));
  return;
}





void cDemeManager::SterileInjectCenter(const cGenome& genome, int target_id)
{
  cDeme* target = GetDeme(target_id);
  assert(target != NULL);
  
  SterilizeDeme(target_id);
  const int target_cell = target->GetCellID(target->GetWidth()/2, target->GetHeight()/2);
  m_population.InjectGenome(target_cell, genome, -1);
}




void cDemeManager::SterileInjectRandom(const cGenome& genome, int target_id)
{
  cDeme* target = GetDeme(target_id);
  assert(target != NULL);
  
  SterilizeDeme(target_id);
  const int target_cell = target->GetCellID(m_world->GetRandom().GetUInt(target->GetSize()));
  m_population.InjectGenome(target_cell, genome, -1);
}


void cDemeManager::SterileInjectFull(const cGenome& genome, int target_id)
{
  cDeme* target = GetDeme(target_id);
  assert(target != NULL);
  
  SterilizeDeme(target_id);
  for (int i = 0; i < target->GetSize(); i++) 
    m_population.InjectGenome(target->GetCellID(i), genome, -1 );
}




void cDemeManager::CopyDemeGermline(int source_id, int target_id)
{
  cDeme* target = GetDeme(target_id);
  cDeme* source = GetDeme(source_id);
  assert(target!=NULL && source!=NULL);
  if (m_world->GetConfig().DEMES_USE_GERMLINES.Get() > 0){
    cGenome source_germ  = source->GetGermline().GetLatest();
    cGenome mutated_germ = DoGermlineMutation(source_germ);
    target->ReplaceGermline(source->GetGermline());
    target->GetGermline().Add(mutated_germ);
    if (m_world->GetConfig().GERMLINE_REPLACES_SOURCE.Get() > 0)
      source->GetGermline().Add(mutated_germ);
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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
void cDemeManager::PrintDemeSpatialResData( cResourceCount res, const int i, const int deme_id) const
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
  const int num_task = m_population.environment.GetNumTasks();
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
      if (m_population.GetCell(cur_cell).IsOccupied() == false) continue;
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


cOrganism* cDemeManager::SampleRandomDemeOrganism(int deme_id){

  cDeme& deme = *GetDeme(deme_id);
  if (deme.GetOrgCount() < 1)
    return NULL;
  int num_cells = deme.GetSize();
  tArray<int> occupied(num_cells);
  int count = 0;
  for (int i = 0; i < num_cells; i++){
    int cell = deme.GetCellID(i);
    if (m_population.GetCell(cell).IsOccupied())
      occupied[count++] = cell;
  }
  int selected = m_world->GetRandom().GetUInt(count);
  return m_population.GetCell(occupied[selected]).GetOrganism();
}


const cGenome* cDemeManager::SampleRandomDemeGenome(int deme_id){
  const cGenome* retval = NULL;
  cDeme& deme = *GetDeme(deme_id);
  if (deme.GetOrgCount() < 1)
    return retval;
  int num_cells = deme.GetSize();
  tArray<int> occupied(num_cells);
  int count = 0;
  for (int i = 0; i < num_cells; i++){
    int cell = deme.GetCellID(i);
    if (m_population.GetCell(cell).IsOccupied())
      occupied[count++] = cell;
  }
  if (count == 0)
    return retval;
  int selected = occupied[m_world->GetRandom().GetUInt(count)];
  
  retval = &m_population.GetCell(occupied[selected]).GetOrganism()->GetGenome();
  return retval;
}
