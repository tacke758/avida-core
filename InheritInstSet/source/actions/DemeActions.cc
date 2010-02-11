/*
 *  DemeActions.cc
 *  Avida
 *
 *  Created by Matthew Rupp on 2/1/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "DemeActions.h"

#include "cAction.h"
#include "cActionLibrary.h"
#include "cDataFileManager.h"
#include "cDeme.h"
#include "cDemeManager.h"
#include "cHardwareManager.h"
#include "cPopulation.h"
#include "cRandom.h"
#include "cStats.h"
#include "cString.h"

#include <iostream>

class cDemeActionMutateInstSetID : public cDemeAction
  {
  private:
    double m_mutation_rate;
    int DoInstSetMutation(int old_id)
    {
      int new_id = old_id;
      int num_ids = m_world->GetHardwareManager().GetNumInstSets();
      double p = m_world->GetRandom().GetDouble(0,1);
      if (p < m_mutation_rate){
        p = m_world->GetRandom().GetDouble(0,1);
        new_id = (p < 0.5) ? (num_ids+old_id-1) % num_ids : (num_ids+old_id+1) % num_ids;
      }
      return new_id;
    }
    
  public:
    cDemeActionMutateInstSetID(cWorld* world, const cString& args) : cDemeAction(world, args)
    {
      cString largs(args);
      if (largs.GetSize()){
        m_mutation_rate = largs.PopWord().AsDouble();
      } else {
        world->GetDriver().RaiseFatalException(2, "Unable to set instset mutation rate.");
      }
    }
    
    static const cString GetDescription() { return "Arguments: <mutation_rate>"; }
    
    void Process(cAvidaContext& ctx)
    {
      return;
    }
    
    void Process(cEventContext& ctx)
    {
      int source_id = (*ctx["source_id"]).AsInt();
      int target_id = (*ctx["target_id"]).AsInt();
   
      int old_id = m_world->GetPopulation().GetDemeManager().GetDeme(source_id)->GetInstSetID();
      int new_id = (source_id != target_id) ? DoInstSetMutation(old_id) : old_id;
      m_world->GetPopulation().GetDemeManager().GetDeme(target_id)->SetInstSetID(new_id);
    
    }
  
  };



class cDemeActionPrintInstSetData : public cDemeAction
{
  private:
    cString m_filename;
    bool first_run;
    void PrintFormattingHeader(ofstream& fot)
    {
      int num_demes = m_world->GetPopulation().GetDemeManager().GetNumDemes();
      fot << "#format: update%d ";
      for (int k = 0; k < num_demes; k++)
        fot << "fitness_" << k << "%f instset_" << k << "%d ";
      fot << endl;
    }
  
  public:
    cDemeActionPrintInstSetData(cWorld* world, const cString& args) : cDemeAction(world, args)
    {
      cString largs(args);
      m_filename = (largs.GetSize()) ? largs.PopWord() : "deme_instset_data.dat";
      first_run = true;
    }
    
    static const cString GetDescription() { return "Arguments: <path>"; }
    
    void Process(cAvidaContext& ctx)
    {
      cEventContext state(ctx, TRIGGER_UNKNOWN);
      Process(state);
    }
  
    void Process(cEventContext& ctx)
    {
      ofstream& fot = m_world->GetDataFileManager().GetOFStream(m_filename);
      if (first_run){
        first_run = false;
        PrintFormattingHeader(fot);
      }
      int num_demes = m_world->GetPopulation().GetDemeManager().GetNumDemes();
      int update = m_world->GetStats().GetUpdate();
      
      fot << update;
      for (int id = 0; id < num_demes; id++){
        double deme_fitness = m_world->GetPopulation().GetDemeManager().GetDemeFitness(id);
        int    deme_instset = m_world->GetPopulation().GetDemeManager().GetDeme(id)->GetInstSetID();
        fot <<  " " << deme_fitness << " " << deme_instset;
      }
      fot << endl;
    }
};




class cDemeActionSetAllDemesInstSetID : public cDemeAction
  {
  private:
    int m_id;
    
  public:
    cDemeActionSetAllDemesInstSetID(cWorld* world, const cString& args) : cDemeAction(world, args)
    {
      cString largs(args);
      m_id = largs.PopWord().AsInt();
    }
    
    static const cString GetDescription() { return "Arguments: <path>"; }
    
    void Process(cAvidaContext& ctx)
    {
      cEventContext state(ctx, TRIGGER_UNKNOWN);
      Process(state);
    }
    
    void Process(cEventContext& ctx)
    {
      int num_demes = m_world->GetPopulation().GetDemeManager().GetNumDemes();
      for (int k = 0; k < num_demes; k++)
        m_world->GetPopulation().GetDemeManager().GetDeme(k)->SetInstSetID(m_id);
    }
  };



void RegisterDemeActions(cActionLibrary* action_lib)
{
  action_lib->Register<cDemeActionMutateInstSetID>("MutateDemeInstSetID");
  action_lib->Register<cDemeActionPrintInstSetData>("PrintDemeInstSetData");
  action_lib->Register<cDemeActionSetAllDemesInstSetID>("SetAllDemesInstSetID");
  return;  
}
