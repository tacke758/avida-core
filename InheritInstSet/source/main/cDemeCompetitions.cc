/*
 *  cDemeCompetitions.cc
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeCompetitions.h"
#include "cDemeManager.h"
#include "cDoubleSum.h"
#include "cOrganism.h"
#include "cPhenotype.h"
#include "cPopulation.h"
#include "cPopulationCell.h"

void cDemeCompetitions::Control(cDemeManager& mgr)
{
  mgr.m_total_deme_fitness = (double) mgr.GetNumDemes();
  mgr.m_deme_fitness.SetAll(1.0);
  return;
}



void cDemeCompetitions::NewBirths(cDemeManager& mgr)
{
  mgr.m_total_deme_fitness = 0.0;
  for (int id = 0; id < mgr.GetNumDemes(); id++){
    double fitness = mgr.GetDeme(id)->GetBirthCount();
    mgr.m_deme_fitness[id] = fitness;
    mgr.m_total_deme_fitness += fitness;
  }
  return;
}



void cDemeCompetitions::AverageFitness(cDemeManager& mgr)
{
  mgr.m_total_deme_fitness = 0.0;
  for (int id = 0; id < mgr.GetNumDemes(); id++) {
    cDoubleSum single_deme_fitness;
    const cDeme & cur_deme = *mgr.GetDeme(id);
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (mgr.GetPopulation().GetCell(cur_cell).IsOccupied() == false) continue;
      cPhenotype & phenotype = mgr.GetPopulation().GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_fitness.Add(phenotype.GetFitness());
    } 
    mgr.m_deme_fitness[id] = single_deme_fitness.Ave();
    mgr.m_total_deme_fitness += mgr.m_deme_fitness[id];
  }
}



void cDemeCompetitions::AverageMutationRate(cDemeManager& mgr)
{
  mgr.m_total_deme_fitness = 0.0;
  for (int deme_id = 0; deme_id < mgr.GetNumDemes(); deme_id++) {
    cDoubleSum single_deme_div_type;
    const cDeme & cur_deme = *mgr.GetDeme(deme_id);
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (mgr.GetPopulation().GetCell(cur_cell).IsOccupied() == false) continue;
      cPhenotype & phenotype =
      mgr.GetPopulation().GetCell(cur_cell).GetOrganism()->GetPhenotype();
      assert(phenotype.GetDivType()>0);
      single_deme_div_type.Add(1/phenotype.GetDivType());
    }
    mgr.m_deme_fitness[deme_id] = single_deme_div_type.Ave();
    mgr.m_total_deme_fitness += mgr.m_deme_fitness[deme_id];
  }
}



void cDemeCompetitions::StrongRankSelection(cDemeManager& mgr)
{
  int num_demes = mgr.GetNumDemes();
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDoubleSum single_deme_fitness;
    const cDeme & cur_deme = *mgr.GetDeme(deme_id);
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (mgr.GetPopulation().GetCell(cur_cell).IsOccupied() == false) continue;
      cPhenotype & phenotype = mgr.GetPopulation().GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_fitness.Add(phenotype.GetFitness());
    }  
    mgr.m_deme_fitness[deme_id] = single_deme_fitness.Ave();
  }
  // ... then determine the rank of each deme based on its fitness
  tArray<double> deme_rank(num_demes);
  deme_rank.SetAll(1);
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int test_deme = 0; test_deme < num_demes; test_deme++) {
      if (mgr.m_deme_fitness[deme_id] < mgr.m_deme_fitness[test_deme]) {
        deme_rank[deme_id]++;
      } 
    } 
  } 
  // ... finally, make deme fitness 2^(-deme rank)
  mgr.m_deme_fitness.SetAll(1.0);	
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int i = 0; i < deme_rank[deme_id]; i++) { 
      mgr.m_deme_fitness[deme_id] = mgr.m_deme_fitness[deme_id]/2;
    } 
    mgr.m_total_deme_fitness += mgr.m_deme_fitness[deme_id]; 
  } 
}



void cDemeCompetitions::AverageOrganismLife(cDemeManager& mgr)
{
  int num_demes = mgr.GetNumDemes();
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDoubleSum single_deme_life_fitness;
    const cDeme & cur_deme = *mgr.GetDeme(deme_id);
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (mgr.GetPopulation().GetCell(cur_cell).IsOccupied() == false) continue;
      cPhenotype & phenotype = mgr.GetPopulation().GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_life_fitness.Add(phenotype.GetLifeFitness());
    }
    mgr.m_deme_fitness[deme_id] = single_deme_life_fitness.Ave();
    mgr.m_total_deme_fitness += mgr.m_deme_fitness[deme_id];
  }
}


void cDemeCompetitions::StrongRankSelectionLife(cDemeManager& mgr)
{
  int num_demes = mgr.GetNumDemes();
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDoubleSum single_deme_life_fitness;
    const cDeme & cur_deme = *mgr.GetDeme(deme_id);
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (mgr.GetPopulation().GetCell(cur_cell).IsOccupied() == false) continue;
      cPhenotype & phenotype = mgr.GetPopulation().GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_life_fitness.Add(phenotype.GetLifeFitness());
    }
    mgr.m_deme_fitness[deme_id] = single_deme_life_fitness.Ave();
  }
  // ... then determine the rank of each deme based on its fitness
  tArray<double> deme_rank(num_demes);
  deme_rank.SetAll(1);
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int test_deme = 0; test_deme < num_demes; test_deme++) {
      if (mgr.m_deme_fitness[deme_id] < mgr.m_deme_fitness[test_deme]) {
        deme_rank[deme_id]++;
      }
    }
  }
  // ... finally, make deme fitness 2^(-deme rank)
  mgr.m_deme_fitness.SetAll(1);
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int i = 0; i < deme_rank[deme_id]; i++) {
      mgr.m_deme_fitness[deme_id] = mgr.m_deme_fitness[deme_id]/2;
    }
    mgr.m_total_deme_fitness += mgr.m_deme_fitness[deme_id];
  }
}
