/*
 *  cDemeCompetitions.cc
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeCompetitions.h"

void cDemeCompetitions::Control(cDemeManager& mgr)
{
  mgr.total_deme_fitness = (double) mgr.num_demes;
  mgr.m_deme_fitness.SetAll(1);
  return;
}



void cDemeCompetitions::NewBirths(cDemeManager& mgr)
{
  mgr.m_total_deme_fitness = 0.0;
  for (int id = 0; id < mgr.GetNumDemes(); id++){
    double fitness = mgr.m_demes[id].GetBirthCount();
    mgr.m_deme_fitness[id] = fitness;
    mgr.m_total_deme_fitness += fitness;
  }
  return;
}



void cDemeCompetitions::AverageFitness(cDemeManager& mgr)
{
  mgr.m_total_fitness = 0.0;
  for (int id = 0; id < num_demes; id++) {
    cDoubleSum single_deme_fitness;
    const cDeme & cur_deme = mgr.m_demes[id];
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_fitness.Add(phenotype.GetFitness());
    } 
    mgr.m_deme_fitness[id] = single_deme_fitness.Ave();
    mgr.total_fitness += deme_fitness[deme_id];
  }
}



void cDemeCompetitions::AverageMutationRate(cDemeManager& mgr)
{
  mgr.m_total_fitness = 0.0;
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDoubleSum single_deme_div_type;
    const cDeme & cur_deme = deme_array[deme_id];
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype =
      GetCell(cur_cell).GetOrganism()->GetPhenotype();
      assert(phenotype.GetDivType()>0);
      single_deme_div_type.Add(1/phenotype.GetDivType());
    }
    deme_fitness[deme_id] = single_deme_div_type.Ave();
    total_fitness += deme_fitness[deme_id];
  }
}



void cDemeCompetitions::StrongRankSelection(cDemeManager& mgr)
{
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDoubleSum single_deme_fitness;
    const cDeme & cur_deme = deme_array[deme_id];
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_fitness.Add(phenotype.GetFitness());
    }  
    deme_fitness[deme_id] = single_deme_fitness.Ave();
  }
  // ... then determine the rank of each deme based on its fitness
  tArray<double> deme_rank(num_demes);
  deme_rank.SetAll(1);
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int test_deme = 0; test_deme < num_demes; test_deme++) {
      if (deme_fitness[deme_id] < deme_fitness[test_deme]) {
        deme_rank[deme_id]++;
      } 
    } 
  } 
  // ... finally, make deme fitness 2^(-deme rank)
  deme_fitness.SetAll(1);	
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    for (int i = 0; i < deme_rank[deme_id]; i++) { 
      deme_fitness[deme_id] = deme_fitness[deme_id]/2;
    } 
    total_fitness += deme_fitness[deme_id]; 
  } 
}



void cDemeCompetitions::AverageOrganismLife(cDemeManager& mgr)
{
  for (int deme_id = 0; deme_id < num_demes; deme_id++) {
    cDoubleSum single_deme_life_fitness;
    const cDeme & cur_deme = deme_array[deme_id];
    for (int i = 0; i < cur_deme.GetSize(); i++) {
      int cur_cell = cur_deme.GetCellID(i);
      if (cell_array[cur_cell].IsOccupied() == false) continue;
      cPhenotype & phenotype = GetCell(cur_cell).GetOrganism()->GetPhenotype();
      single_deme_life_fitness.Add(phenotype.GetLifeFitness());
    }
    deme_fitness[deme_id] = single_deme_life_fitness.Ave();
    total_fitness += deme_fitness[deme_id];
  }
}


void cDemeCompetitions::StrongRankSelectionLife(cDemeManager& mgr)
{
  {
    for (int deme_id = 0; deme_id < num_demes; deme_id++) {
      cDoubleSum single_deme_life_fitness;
      const cDeme & cur_deme = deme_array[deme_id];
      for (int i = 0; i < cur_deme.GetSize(); i++) {
        int cur_cell = cur_deme.GetCellID(i);
        if (cell_array[cur_cell].IsOccupied() == false) continue;
        cPhenotype & phenotype = GetCell(cur_cell).GetOrganism()->GetPhenotype();
        single_deme_life_fitness.Add(phenotype.GetLifeFitness());
      }
      deme_fitness[deme_id] = single_deme_life_fitness.Ave();
    }
    // ... then determine the rank of each deme based on its fitness
    tArray<double> deme_rank(num_demes);
    deme_rank.SetAll(1);
    for (int deme_id = 0; deme_id < num_demes; deme_id++) {
      for (int test_deme = 0; test_deme < num_demes; test_deme++) {
        if (deme_fitness[deme_id] < deme_fitness[test_deme]) {
          deme_rank[deme_id]++;
        }
      }
    }
    // ... finally, make deme fitness 2^(-deme rank)
    deme_fitness.SetAll(1);
    for (int deme_id = 0; deme_id < num_demes; deme_id++) {
      for (int i = 0; i < deme_rank[deme_id]; i++) {
        deme_fitness[deme_id] = deme_fitness[deme_id]/2;
      }
      total_fitness += deme_fitness[deme_id];
    }
  }
}