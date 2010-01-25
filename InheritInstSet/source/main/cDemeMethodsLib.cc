/*
 *  cDemeMethodsLib.cpp
 *  Avida
 *
 *  Created by Matthew Rupp on 1/25/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cDemeMethodsLib.h"
#include "cString.h"

void cDemeMethodsLib::Setup()
{
  m_competitions.Add("UsingControl", cDemeCompetitions::Control);
  m_competitions.Add("UsingNewBirths", cDemeCompetitions::NewBirths);
  m_competitions.Add("UsingAverageFitness", cDemeCompetitions::AverageFitness);
  m_competitions.Add("UsingAverageMutationRate", cDemeCompetitions::AverageMutationRate);
  m_competitions.Add("UsingStrongRankSelection", cDemeCompetitions::StrongRankSelection);
  m_competitions.Add("UsingAverageOrganismLife", cDemeCompetitions::AverageOrganismLife);
  m_competitions.Add("UsingStrongRankSelectionOnLife", cDemeCompetitions::StrongRankSelectionLife);
  
  m_selections.Add("WithFitnessProportionalSelection", cDemeSelections::FitnessProportional);
  m_selections.Add("WithTournamentSelection", cDemeSelections::Tournament);
  
  m_triggers.Add("WhenNotEmpty", cDemeTriggers::ReplicateNonEmpty);
  m_triggers.Add("WhenFull", cDemeTriggers::ReplicateFull);
  m_triggers.Add("WhenCornersFilled", cDemeTriggers::ReplicateCornersFilled);
  m_triggers.Add("WhenOld", cDemeTriggers::ReplicateOldDemes);
  
  m_replications.Add("BySterilizingAndInjectingAtCenter", cDemeReplications::SterileInjectionAtCenter);
  m_replications.Add("BySterilizingAndInjectingAtRandom", cDemeReplications::SterileInjectionAtRandom);
  m_replications.Add("BySterilizingAndFullyInjecting", cDemeReplications::SterileFullInjection);
  m_replications.Add("ByFullyCopying", cDemeReplications::InjectCopy);
}




tDemeCompetition cDemeMethodsLib::GetCompetition(const cString& name)
{
  tDemeCompetition func = NULL;
  bool found = m_competitions.Find(name, func);
  return (found) ? func : NULL;
}



tDemeSelection cDemeMethodsLib::GetSelection(const cString& name)
{
  tDemeSelection func = NULL;
  bool found = m_selections.Find(name, func);
  return (found) ? func : NULL;
}




tDemeTrigger cDemeMethodsLib::GetTrigger(const cString& name)
{
  tDemeTrigger func = NULL;
  bool found = m_triggers.Find(name, func);
  return (found) ? func : NULL;
}



tDemeReplication cDemeMethodsLib::GetReplication(const cString& name)
{
  tDemeReplication func = NULL;
  bool found = m_replications.Find(name, func);
  return (found) ? func : NULL;
}
