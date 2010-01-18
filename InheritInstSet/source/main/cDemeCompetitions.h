/*
 *  cDemeCompetitions.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cPopulation.h"
#include "cDemeManager.h"

class cDemeCompetitions{
 
  private:
    cDemeCompetitions();
  
  public:
    static void Control(cDemeManager& mgr);
    static void NewBirths(cDemeManager& mgr);
    static void AverageFitness(cDemeManager& mgr);
    static void AverageMutationRate(cDemeManager& mgr);
    static void StrongRankSelection(cDemeManager& mgr);
    static void AverageOrganismLife(cDemeManager& mgr);
    static void StrongRankSelectionLife(cDemeManager& mgr);
};