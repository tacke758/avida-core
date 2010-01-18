/*
 *  cDemeManager.h
 *  Avida
 *
 *  Created by Matthew Rupp on 1/18/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 */

#include "cPopulation.h"
#include "cDeme.h"


class cDemeManager{
  friend class cDemeCompeitions;
  tArray<int> CompetitionResults();
  
  private:
    cWorld*        m_world;
    cPopulation&   m_population;
    tArray<cDeme*> m_demes;
    tArray<double> m_deme_fitness;
    double         m_total_deme_fitness;
  
  
    static cGenome DoGermlineMutation(const cGenome& source_germ);
    
    cDemeManager();
    cDemeManager(const cDemeManager&);
    cDemeManager operator=(const cDemeManager&);
  
  public:
    cDemeManager(cPopulation& p) : m_world(p->m_world), m_population(p), {;}
    int GetNumDemes() { return deme_array.GetSize(); }
    cDeme& GetDeme(int i) { return *deme_array[i]; }
    
    // Deme-related methods
    void CompeteDemes(int competition_type);
    void ReplicateDemes(int rep_trigger);
    void DivideDemes();
    void ResetDemes();
    void CopyDeme(int source_id, int target_id);
    void CopyDemeGermline(int source_id, int target_id);
    void SterileRandomInjection(int source_id, int target_id);
    void SterileGermlineInjection(int source_id, int target_id);
    void SpawnDeme(int deme1_id, int deme2_id=-1);
    
    // Deme-related stats methods
    void PrintDemeAllStats();
    void PrintDemeDonor();
    void PrintDemeFitness();
    void PrintDemeGestationTime();
    void PrintDemeInstructions();
    void PrintDemeLifeFitness();
    void PrintDemeMerit();
    void PrintDemeMutationRate();
    void PrintDemeReceiver();
    void PrintDemeResource();
    void PrintDemeSpatialResData(cResourceCount res, const int i, const int deme_id) const;
    void PrintDemeSpatialEnergyData() const;
    void PrintDemeSpatialSleepData() const;
    void PrintDemeTasks();
    const tArray<double>& GetDemeResources(int deme_id) { return GetDeme(deme_id).GetDemeResourceCount().GetResources(); }
    void UpdateDemeCellResources(const tArray<double>& res_change, const int cell_id);
};